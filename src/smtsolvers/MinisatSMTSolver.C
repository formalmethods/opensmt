/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

OpenSMT -- Copyright (C) 2008, Roberto Bruttomesso

OpenSMT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenSMT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenSMT. If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

/****************************************************************************************[Solver.C]
MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

// #include "Solver.h"
#include "MinisatSMTSolver.h"
#include "Sort.h"
#include <cmath>

//=================================================================================================
// Added code

#ifndef OPTIMIZE
#include <iostream>
#include <fstream>
#include <sys/wait.h>
#endif

// Added code
//=================================================================================================

//=================================================================================================
// Constructor/Destructor:


MinisatSMTSolver::MinisatSMTSolver( Egraph & e, SMTConfig & c ) 
    // Initializes configuration and egraph
  : SMTSolver        ( e, c )
    // Parameters: (formerly in 'SearchParams')
  , var_decay(1 / 0.95), clause_decay(1 / 0.999), random_var_freq(0.02)
  , restart_first(100), restart_inc(1.5), learntsize_factor((double)1/(double)3), learntsize_inc(1.1)

    // More parameters:
    //
  , expensive_ccmin  (true)
  , polarity_mode    (polarity_false)
  , verbosity        (config.satconfig.verbose)

    // Statistics: (formerly in 'SolverStats')
    //
  , starts(0), decisions(0), rnd_decisions(0), propagations(0), conflicts(0)
  , clauses_literals(0), learnts_literals(0), max_literals(0), tot_literals(0)

  , ok               (true)
  , cla_inc          (1)
  , var_inc          (1)
  , qhead            (0)
  , simpDB_assigns   (-1)
  , simpDB_props     (0)
  , order_heap       (VarOrderLt(activity))
  , random_seed      (91648253)
  , progress_estimate(0)
  , remove_satisfied (true)
{
  
//=================================================================================================
// Added code

  theory_handler = new THandler( egraph, config, *this, trail, level, assigns );
  vec< Lit > fc;
  fc.push( lit_Undef );
  fake_clause = Clause_new( fc );
  first_model_found = false;
  
// Added code
//=================================================================================================

}

MinisatSMTSolver::~MinisatSMTSolver()
{
    for (int i = 0; i < learnts.size(); i++) free(learnts[i]);
    for (int i = 0; i < clauses.size(); i++) free(clauses[i]);

//=================================================================================================
// Added code

    if ( first_model_found ) theory_handler->backtrack( -1 );
    delete theory_handler;
    free(fake_clause);

// Added code
//=================================================================================================

}


//=================================================================================================
// Minor methods:


// Creates a new SAT variable in the solver. If 'decision_var' is cleared, variable will not be
// used as a decision variable (NOTE! This has effects on the meaning of a SATISFIABLE result).
//
Var MinisatSMTSolver::newVar(bool sign, bool dvar)
{
    int v = nVars();
    watches   .push();          // (list for positive literal)
    watches   .push();          // (list for negative literal)
    reason    .push(NULL);
    assigns   .push(toInt(l_Undef));
    level     .push(-1);
    activity  .push(0);
    seen      .push(0);

    polarity    .push((char)sign);
    decision_var.push((char)dvar);

    insertVarOrder(v);
    return v;
}


bool MinisatSMTSolver::addClause(vec<Lit>& ps)
{
    assert(decisionLevel() == 0);

    if (!ok)
        return false;
    else{
        // Check if clause is satisfied and remove false/duplicate literals:
        sort(ps);
        Lit p; int i, j;
        for (i = j = 0, p = lit_Undef; i < ps.size(); i++)
            if (value(ps[i]) == l_True || ps[i] == ~p)
                return true;
            else if (value(ps[i]) != l_False && ps[i] != p)
                ps[j++] = p = ps[i];
        ps.shrink(i - j);
    }

    if (ps.size() == 0)
        return ok = false;
    else if (ps.size() == 1){
        assert(value(ps[0]) == l_Undef);
        uncheckedEnqueue(ps[0]);
        return ok = (propagate() == NULL);
    }else{
        Clause* c = Clause_new(ps, false);
        clauses.push(c);
        attachClause(*c);
    }

    return true;
}


void MinisatSMTSolver::attachClause(Clause& c) {
    assert(c.size() > 1);
    watches[toInt(~c[0])].push(&c);
    watches[toInt(~c[1])].push(&c);
    if (c.learnt()) learnts_literals += c.size();
    else            clauses_literals += c.size(); }


void MinisatSMTSolver::detachClause(Clause& c) {
    assert(c.size() > 1);
    assert(find(watches[toInt(~c[0])], &c));
    assert(find(watches[toInt(~c[1])], &c));
    remove(watches[toInt(~c[0])], &c);
    remove(watches[toInt(~c[1])], &c);
    if (c.learnt()) learnts_literals -= c.size();
    else            clauses_literals -= c.size(); }


void MinisatSMTSolver::removeClause(Clause& c) {
    detachClause(c);
    free(&c); }


bool MinisatSMTSolver::satisfied(const Clause& c) const {
    for (int i = 0; i < c.size(); i++)
        if (value(c[i]) == l_True)
            return true;
    return false; }


// Revert to the state at given level (keeping all assignment at 'level' but not beyond).
//
void MinisatSMTSolver::cancelUntil(int level) {
    if (decisionLevel() > level){
        for (int c = trail.size()-1; c >= trail_lim[level]; c--){
            Var     x  = var(trail[c]);
            assigns[x] = toInt(l_Undef);
            insertVarOrder(x); }
        qhead = trail_lim[level];
        trail.shrink(trail.size() - trail_lim[level]);
        trail_lim.shrink(trail_lim.size() - level);
    } 
    
//=================================================================================================
// Added code
    
    if ( first_model_found ) theory_handler->backtrack( level );

// Added code
//=================================================================================================
}


//=================================================================================================
// Major methods:


Lit MinisatSMTSolver::pickBranchLit(int polarity_mode, double random_var_freq)
{
    Var next = var_Undef;

    // Random decision:
    if (drand(random_seed) < random_var_freq && !order_heap.empty()){
        next = order_heap[irand(random_seed,order_heap.size())];
        if (toLbool(assigns[next]) == l_Undef && decision_var[next])
            rnd_decisions++; }

    // Activity based decision:
    while (next == var_Undef || toLbool(assigns[next]) != l_Undef || !decision_var[next])
        if (order_heap.empty()){
            next = var_Undef;
            break;
        }else
            next = order_heap.removeMin();

    bool sign = false;
    switch (polarity_mode){
    case polarity_true:  sign = false; break;
    case polarity_false: sign = true;  break;
    case polarity_user:  sign = polarity[next]; break;
    case polarity_rnd:   sign = irand(random_seed, 2); break;
    default: assert(false); }

    return next == var_Undef ? lit_Undef : Lit(next, sign);
}


/*_________________________________________________________________________________________________
|
|  analyze : (confl : Clause*) (out_learnt : vec<Lit>&) (out_btlevel : int&)  ->  [void]
|  
|  Description:
|    Analyze conflict and produce a reason clause.
|  
|    Pre-conditions:
|      * 'out_learnt' is assumed to be cleared.
|      * Current decision level must be greater than root level.
|  
|    Post-conditions:
|      * 'out_learnt[0]' is the asserting literal at level 'out_btlevel'.
|  
|  Effect:
|    Will undo part of the trail, upto but not beyond the assumption of the current decision level.
|________________________________________________________________________________________________@*/
void MinisatSMTSolver::analyze(Clause* confl, vec<Lit>& out_learnt, int& out_btlevel)
{
    assert( cleanup.size( ) == 0 );

    int pathC = 0;
    Lit p     = lit_Undef;

    // Generate conflict clause:
    //
    out_learnt.push();      // (leave room for the asserting literal)
    int index   = trail.size() - 1;
    out_btlevel = 0;

    do{
        assert(confl != NULL);          // (otherwise should be UIP)
        Clause& c = *confl;


        if (c.learnt())
            claBumpActivity(c);

        for (int j = (p == lit_Undef) ? 0 : 1; j < c.size(); j++){
            Lit q = c[j];

            if (!seen[var(q)] && level[var(q)] > 0){
                varBumpActivity(var(q));
                seen[var(q)] = 1;
		
                if (level[var(q)] >= decisionLevel())
                    pathC++;
                else{
                    out_learnt.push(q);
                    if (level[var(q)] > out_btlevel)
                        out_btlevel = level[var(q)];
                }
            }
        }

        // Select next clause to look at:
	while (!seen[var(trail[index--])]);
        p     = trail[index+1];

//=================================================================================================
// Added code

	if ( reason[var(p)] != NULL && reason[var(p)] == fake_clause )
	{
	  vec< Lit > r;
	  theory_handler->getReason( p, r );

	  // Produce a new reason
	  Clause * c = Clause_new( r );
	  cleanup.push( c );
	  reason[var(p)] = c;
	} 

        confl = reason[var(p)];

// Added code
//=================================================================================================

        seen[var(p)] = 0;
        pathC--;

    }while (pathC > 0);
    out_learnt[0] = ~p;

    // Simplify conflict clause:
    //
    int i, j;
    if (expensive_ccmin){
        uint32_t abstract_level = 0;
        for (i = 1; i < out_learnt.size(); i++)
            abstract_level |= abstractLevel(var(out_learnt[i])); // (maintain an abstraction of levels involved in conflict)

        out_learnt.copyTo(analyze_toclear);
        for (i = j = 1; i < out_learnt.size(); i++)
            if (reason[var(out_learnt[i])] == NULL || !litRedundant(out_learnt[i], abstract_level))
                out_learnt[j++] = out_learnt[i];
    }else{
        out_learnt.copyTo(analyze_toclear);
        for (i = j = 1; i < out_learnt.size(); i++){
            Clause& c = *reason[var(out_learnt[i])];
	    for (int k = 1; k < c.size(); k++)
                if (!seen[var(c[k])] && level[var(c[k])] > 0){
                    out_learnt[j++] = out_learnt[i];
                    break; }
        }
    }
    max_literals += out_learnt.size();
    out_learnt.shrink(i - j);
    tot_literals += out_learnt.size();

    // Find correct backtrack level:
    //
    if (out_learnt.size() == 1)
        out_btlevel = 0;
    else{
        int max_i = 1;
        for (int i = 2; i < out_learnt.size(); i++)
            if (level[var(out_learnt[i])] > level[var(out_learnt[max_i])])
                max_i = i;
        Lit p             = out_learnt[max_i];
        out_learnt[max_i] = out_learnt[1];
        out_learnt[1]     = p;
        out_btlevel       = level[var(p)];
    }

    for (int j = 0; j < analyze_toclear.size(); j++) seen[var(analyze_toclear[j])] = 0;    // ('seen[]' is now cleared)

    for ( int i = 0 ; i < cleanup.size() ; i ++ )
      free(cleanup[ i ]);
    cleanup.clear();
}


// Check if 'p' can be removed. 'abstract_levels' is used to abort early if the algorithm is
// visiting literals at levels that cannot be removed later.
bool MinisatSMTSolver::litRedundant(Lit p, uint32_t abstract_levels)
{
    analyze_stack.clear(); analyze_stack.push(p);
    int top = analyze_toclear.size();
    while (analyze_stack.size() > 0){
        assert(reason[var(analyze_stack.last())] != NULL);
	
//=================================================================================================
// Added code

	// Since we are lazily constructing reasons for theory propagation
	// it might be the case that we didn't construct a reason, because
	// it was just not needed for UIP detection. In this case we just
	// abort early this algorithm. Alternatively we can construct the
	// reason now. However this cannot be done now, since every time
	// we construct a reason the solver is backtracked a little bit, in order
	// to avoid the presence of literals of a wrong decision level inside
	// the explanation. Therefore we might not be able to retrieve the reason 
	// for this literal ... at the moment we just give up and return false
	if( reason[var(analyze_stack.last()) ] == fake_clause )
	  return false;

// Added code
//=================================================================================================

        Clause& c = *reason[var(analyze_stack.last())]; 
	analyze_stack.pop();

        for (int i = 1; i < c.size(); i++){
            Lit p  = c[i];

            if (!seen[var(p)] && level[var(p)] > 0){

                if (reason[var(p)] != NULL && (abstractLevel(var(p)) & abstract_levels) != 0){
                    seen[var(p)] = 1;
                    analyze_stack.push(p);
                    analyze_toclear.push(p);
                }else{
                    for (int j = top; j < analyze_toclear.size(); j++)
                        seen[var(analyze_toclear[j])] = 0;
                    analyze_toclear.shrink(analyze_toclear.size() - top);

                    return false;
                }
            }
        }
    }

    return true;
}

/*_________________________________________________________________________________________________
|
|  analyzeFinal : (p : Lit)  ->  [void]
|  
|  Description:
|    Specialized analysis procedure to express the final conflict in terms of assumptions.
|    Calculates the (possibly empty) set of assumptions that led to the assignment of 'p', and
|    stores the result in 'out_conflict'.
|________________________________________________________________________________________________@*/
void MinisatSMTSolver::analyzeFinal(Lit p, vec<Lit>& out_conflict)
{
    out_conflict.clear();
    out_conflict.push(p);

    if (decisionLevel() == 0)
        return;

    seen[var(p)] = 1;

    for (int i = trail.size()-1; i >= trail_lim[0]; i--){
        Var x = var(trail[i]);
        if (seen[x]){
            if (reason[x] == NULL){
                assert(level[x] > 0);
                out_conflict.push(~trail[i]);
            }else{
                Clause& c = *reason[x];
                for (int j = 1; j < c.size(); j++)
                    if (level[var(c[j])] > 0)
                        seen[var(c[j])] = 1;
            }
            seen[x] = 0;
        }
    }

    seen[var(p)] = 0;
}


void MinisatSMTSolver::uncheckedEnqueue(Lit p, Clause* from)
{
    assert(value(p) == l_Undef);
    assigns [var(p)] = toInt(lbool(!sign(p)));  // <<== abstract but not uttermost effecient
    level   [var(p)] = decisionLevel();
    reason  [var(p)] = from;
    trail.push(p);
}


/*_________________________________________________________________________________________________
|
|  propagate : [void]  ->  [Clause*]
|  
|  Description:
|    Propagates all enqueued facts. If a conflict arises, the conflicting clause is returned,
|    otherwise NULL.
|  
|    Post-conditions:
|      * the propagation queue is empty, even if there was a conflict.
|________________________________________________________________________________________________@*/
Clause* MinisatSMTSolver::propagate()
{
    Clause* confl     = NULL;
    int     num_props = 0;

    while (qhead < trail.size()){
        Lit            p   = trail[qhead++];     // 'p' is enqueued fact to propagate.

        vec<Clause*>&  ws  = watches[toInt(p)];
        Clause         **i, **j, **end;
        num_props++;

        for (i = j = (Clause**)ws, end = i + ws.size();  i != end;){
            Clause& c = **i++;

            // Make sure the false literal is data[1]:
            Lit false_lit = ~p;
            if (c[0] == false_lit)
                c[0] = c[1], c[1] = false_lit;

            assert(c[1] == false_lit);

            // If 0th watch is true, then clause is already satisfied.
            Lit first = c[0];
            if (value(first) == l_True){
                *j++ = &c;
            }else{
                // Look for new watch:
                for (int k = 2; k < c.size(); k++)
                    if (value(c[k]) != l_False){
                        c[1] = c[k]; c[k] = false_lit;
                        watches[toInt(~c[1])].push(&c);
                        goto FoundWatch; }

                // Did not find watch -- clause is unit under assignment:
                *j++ = &c;
                if (value(first) == l_False){
                    confl = &c;
                    qhead = trail.size();
                    // Copy the remaining watches:
                    while (i < end)
                        *j++ = *i++;
                }else
		{
                    uncheckedEnqueue(first, &c);
		}
            }
        FoundWatch:;
        }
        ws.shrink(i - j);
    }
    propagations += num_props;
    simpDB_props -= num_props;

    return confl;
}

/*_________________________________________________________________________________________________
|
|  reduceDB : ()  ->  [void]
|  
|  Description:
|    Remove half of the learnt clauses, minus the clauses locked by the current assignment. Locked
|    clauses are clauses that are reason to some assignment. Binary clauses are never removed.
|________________________________________________________________________________________________@*/
struct reduceDB_lt { bool operator () (Clause* x, Clause* y) { return x->size() > 2 && (y->size() == 2 || x->activity() < y->activity()); } };
void MinisatSMTSolver::reduceDB()
{
    int     i, j;
    double  extra_lim = cla_inc / learnts.size();    // Remove any clause below this activity

    sort(learnts, reduceDB_lt());
    for (i = j = 0; i < learnts.size() / 2; i++){
        if (learnts[i]->size() > 2 && !locked(*learnts[i]))
            removeClause(*learnts[i]);
        else
            learnts[j++] = learnts[i];
    }
    for (; i < learnts.size(); i++){
        if (learnts[i]->size() > 2 && !locked(*learnts[i]) && learnts[i]->activity() < extra_lim)
            removeClause(*learnts[i]);
        else
            learnts[j++] = learnts[i];
    }
    learnts.shrink(i - j);
}


void MinisatSMTSolver::removeSatisfied(vec<Clause*>& cs)
{
    int i,j;
    for (i = j = 0; i < cs.size(); i++){
        if (satisfied(*cs[i]))
            removeClause(*cs[i]);
        else
            cs[j++] = cs[i];
    }
    cs.shrink(i - j);
}


/*_________________________________________________________________________________________________
|
|  simplify : [void]  ->  [bool]
|  
|  Description:
|    Simplify the clause database according to the current top-level assigment. Currently, the only
|    thing done here is the removal of satisfied clauses, but more things can be put here.
|________________________________________________________________________________________________@*/
bool MinisatSMTSolver::simplify()
{
    assert(decisionLevel() == 0);

    if (!ok || propagate() != NULL)
        return ok = false;

    if (nAssigns() == simpDB_assigns || (simpDB_props > 0))
        return true;

    // Remove satisfied clauses:
    removeSatisfied(learnts);
    if (remove_satisfied)        // Can be turned off.
        removeSatisfied(clauses);

    // Remove fixed variables from the variable heap:
    order_heap.filter(VarFilter(*this));

    simpDB_assigns = nAssigns();
    simpDB_props   = clauses_literals + learnts_literals;   // (shouldn't depend on stats really, but it will do for now)

    return true;
}


/*_________________________________________________________________________________________________
|
|  search : (nof_conflicts : int) (nof_learnts : int) (params : const SearchParams&)  ->  [lbool]
|  
|  Description:
|    Search for a model the specified number of conflicts, keeping the number of learnt clauses
|    below the provided limit. NOTE! Use negative value for 'nof_conflicts' or 'nof_learnts' to
|    indicate infinity.
|  
|  Output:
|    'l_True' if a partial assigment that is consistent with respect to the clauseset is found. If
|    all variables are decision variables, this means that the clause set is satisfiable. 'l_False'
|    if the clause set is unsatisfiable. 'l_Undef' if the bound on number of conflicts is reached.
|________________________________________________________________________________________________@*/
lbool MinisatSMTSolver::search(int nof_conflicts, int nof_learnts)
{
    assert(ok);
    int         backtrack_level;
    int         conflictC = 0;
    vec<Lit>    learnt_clause;

    starts++;

    bool first = true;

//=================================================================================================
// Added code
             
    // Check that the facts at level 0 are theory-consistent
    int res = checkTheory( false );
    assert( res == 1 || res == -1 );
    if ( res == -1 ) return l_False;  
    // Perform a first theory propagation
    if ( config.satconfig.theory_propagation > 0 )
    {
      res = deduceTheory( ); 
      assert( res == 0 || res == 1 );
    }

// Added code
//=================================================================================================

    for (;;){

        Clause* confl = propagate();
        if (confl != NULL){
            // CONFLICT
            conflicts++; conflictC++;
            if (decisionLevel() == 0) return l_False;

            first = false;

            learnt_clause.clear();
            analyze(confl, learnt_clause, backtrack_level);
            cancelUntil(backtrack_level);
            assert(value(learnt_clause[0]) == l_Undef);

            if (learnt_clause.size() == 1){
                uncheckedEnqueue(learnt_clause[0]);
            }else{
                Clause* c = Clause_new(learnt_clause, true);
                learnts.push(c);
                attachClause(*c);
                claBumpActivity(*c);
                uncheckedEnqueue(learnt_clause[0], c);
            }

            varDecayActivity();
            claDecayActivity();

        }else{
            // NO CONFLICT

            if (nof_conflicts >= 0 && conflictC >= nof_conflicts){
                // Reached bound on number of conflicts:
                progress_estimate = progressEstimate();
                cancelUntil(0);
                return l_Undef; }

            // Simplify the set of problem clauses:
            if (decisionLevel() == 0 && !simplify())
                return l_False;

            if (nof_learnts >= 0 && learnts.size()-nAssigns() >= nof_learnts)
                // Reduce the set of learnt clauses:
                reduceDB();
	    
//=================================================================================================
// Added code
             
	    if ( first_model_found )
	    {
	    // Early Pruning Call
	    // Step 1: check if the current assignment is theory-consistent
	    int res = checkTheory( false );
	    switch( res )
	    {
	      case -1: return l_False;        // Top-Level conflict: unsat
	      case  0: conflictC++; continue; // Theory conflict: time for bcp
	      case  1: break;                 // Sat: go ahead
	      default: assert( false );
	    }

	    if ( config.satconfig.theory_propagation > 0 )
	    {
	      // Step 2: it's consistent, let's see if we can deduce something
	      res = deduceTheory( ); 
	      switch( res )
	      {
		case  0: break;                 // Nothing to deduce, go ahead
		case  1: continue;              // Deduction performed, time for bcp
		default: assert( false );
	      }
	    }
	    }

// Added code
//=================================================================================================

            Lit next = lit_Undef;
            while (decisionLevel() < assumptions.size()){
                // Perform user provided assumption:
                Lit p = assumptions[decisionLevel()];
                if (value(p) == l_True){
                    // Dummy decision level:
                    newDecisionLevel();
                }else if (value(p) == l_False){
                    analyzeFinal(~p, conflict);
                    return l_False;
                }else{
                    next = p;
                    break;
                }
            }

            if (next == lit_Undef){
                // New variable decision:
                decisions++;
                next = pickBranchLit(polarity_mode, random_var_freq);
		
//=================================================================================================
// Added code

		// Complete Call
		if ( next == lit_Undef )
		{
		  first_model_found = true;
		  int res = checkTheory( true );
		  switch( res )
		  {
		    case -1: return l_False;         // Top-Level conflict: unsat
		    case  0: conflictC++; continue;  // Theory conflict: time for bcp
		    case  1: return l_True;          // Found smt model
		    default: assert( false );
		  }
		}

// Added code
//=================================================================================================

                if (next == lit_Undef)
                    // Model found:
                    return l_True;
            }

            // Increase decision level and enqueue 'next'
            assert(value(next) == l_Undef);
            newDecisionLevel();
            uncheckedEnqueue(next);
        }
    }
}


double MinisatSMTSolver::progressEstimate() const
{
    double  progress = 0;
    double  F = 1.0 / nVars();

    for (int i = 0; i <= decisionLevel(); i++){
        int beg = i == 0 ? 0 : trail_lim[i - 1];
        int end = i == decisionLevel() ? trail.size() : trail_lim[i];
        progress += pow(F, i) * (end - beg);
    }

    return progress / nVars();
}


bool MinisatSMTSolver::solve(const vec<Lit>& assumps)
{
    model.clear();
    conflict.clear();

    if (!ok) return false;

    assumps.copyTo(assumptions);

    double  nof_conflicts = restart_first;
    double  nof_learnts   = nClauses() * learntsize_factor;
    lbool   status        = l_Undef;

//=================================================================================================
// Previous code
/*
    if (verbosity >= 1){
        reportf("============================[ Search Statistics ]==============================\n");
        reportf("| Conflicts |          ORIGINAL         |          LEARNT          | Progress |\n");
        reportf("|           |    Vars  Clauses Literals |    Limit  Clauses Lit/Cl |          |\n");
        reportf("===============================================================================\n");
    }

    // Search:
    while (status == l_Undef){
        if (verbosity >= 1)
            reportf("| %9d | %7d %8d %8d | %8d %8d %6.0f | %6.3f %% |\n", (int)conflicts, order_heap.size(), nClauses(), (int)clauses_literals, (int)nof_learnts, nLearnts(), (double)learnts_literals/nLearnts(), progress_estimate*100), fflush(stdout);
	
        status = search((int)nof_conflicts, (int)nof_learnts);
        nof_conflicts *= restart_inc;
        nof_learnts   *= learntsize_inc;
    }

    if (verbosity >= 1)
        reportf("===============================================================================\n");
*/
// Previous code
//=================================================================================================

//=================================================================================================
// Added code

#ifndef SMTCOMP
    reportf("# ----------+--------------------------+----------+------------+-----------\n");
    reportf("# Conflicts |          LEARNT          | Progress | Cpu time   | Memory    \n");
    reportf("#           |    Limit  Clauses Lit/Cl |          |            |           \n");
    reportf("# ----------+--------------------------+----------+------------+-----------\n");
#endif

    // Search:
    while (status == l_Undef){
#ifndef SMTCOMP
	reportf("# %9d | %8d %8d %6.0f | %6.3f %% | %8.3f s | %6.3f MB\n", (int)conflicts, (int)nof_learnts, nLearnts(), (double)learnts_literals/nLearnts(), progress_estimate*100, cpuTime( ), memUsed( ) / 1048576.0 ), fflush(stdout);
#endif

        status = search((int)nof_conflicts, (int)nof_learnts);
        nof_conflicts *= restart_inc;
        nof_learnts   *= learntsize_inc;
    }

#ifndef SMTCOMP
    reportf("# ----------+--------------------------+----------+------------+-----------\n");
    reportf("#\n");
#endif

// Added code
//=================================================================================================

    if (status == l_True){
        // Extend & copy model:
        model.growTo(nVars());
        for (int i = 0; i < nVars(); i++) model[i] = value(i);
// Previous line
// #ifndef NDEBUG
#ifndef SMTCOMP
        verifyModel();
	// printModel();
#endif
    }else{
        assert(status == l_False);
        if (conflict.size() == 0)
            ok = false;
    }

    cancelUntil(0);
    return status == l_True;
}

//=================================================================================================
// Added code

bool MinisatSMTSolver::addSMTClause( vector< Enode * > & smt_clause ) 
{ 
  vec< Lit > sat_clause;

  for ( vector< Enode * >::iterator it = smt_clause.begin( ) ;
        it != smt_clause.end( ) ;
	it ++ )
  {
    Enode * e = *it;
    // Do not add false literals
    if ( e->isFalse( ) ) continue; 
    // If a literal is true, the clause is true
    if ( e->isTrue( ) )
      return true;
    Lit l = theory_handler->enodeToLit( e );
    sat_clause.push( l );
  }

  return addClause( sat_clause );
}

lbool MinisatSMTSolver::smtSolve( ) { return solve(); }

int MinisatSMTSolver::checkTheory( bool complete ) 
{ 
  bool res = theory_handler->assertLits( )
	  && theory_handler->check( complete );
  // Problem is T-Satisfiable
  if ( res ) return 1;
  // Top-level conflict, problem is T-Unsatisfiable
  if ( decisionLevel() == 0 ) return -1;

  conflicts++;

  vec< Lit > conflicting;
  vec< Lit > learnt_clause;
  int        max_decision_level;
  int        backtrack_level;

  theory_handler->getConflict( conflicting, max_decision_level );
  assert( max_decision_level <= decisionLevel( ) );
  cancelUntil( max_decision_level );

  // Top-level conflict, problem is unsat
  if ( decisionLevel() == 0 ) return -1;

  // Allocate a temporary clause
  Clause* confl = Clause_new( conflicting );

  learnt_clause.clear();
  analyze( confl, learnt_clause, backtrack_level );

  // Remove temporary clause allocated with Clause_new above
  free(confl);

  cancelUntil(backtrack_level);
  assert(value(learnt_clause[0]) == l_Undef);

  if (learnt_clause.size() == 1){
    uncheckedEnqueue(learnt_clause[0]);
  }else{
    Clause* c = Clause_new(learnt_clause, true);
    learnts.push(c);
    attachClause(*c);
    claBumpActivity(*c);
    uncheckedEnqueue(learnt_clause[0], c);
  }

  varDecayActivity();
  claDecayActivity();

  return 0;
}

int MinisatSMTSolver::deduceTheory( )
{
  Lit ded = theory_handler->getDeduction( );  
  
  if ( ded == lit_Undef )
    return 0;

  do
  {
    if ( decisionLevel( ) == 0 )
      uncheckedEnqueue( ded ); 
    else
      uncheckedEnqueue( ded, fake_clause );

    ded = theory_handler->getDeduction( );  
  }
  while( ded != lit_Undef );

  bool res = theory_handler->assertLits( );
  assert( res );

  return 1;
}

// Added code
//=================================================================================================

//=================================================================================================
// Debug methods:

void MinisatSMTSolver::verifyModel()
{
    bool failed = false;
    for (int i = 0; i < clauses.size(); i++){
        assert(clauses[i]->mark() == 0);
        Clause& c = *clauses[i];
        for (int j = 0; j < c.size(); j++)
            if (modelValue(c[j]) == l_True)
                goto next;

        reportf("unsatisfied clause: ");
        printClause(*clauses[i]);
        reportf("\n");
        failed = true;
    next:;
    }

    assert(!failed);

    // Modified line
    // reportf("Verified %d original clauses.\n", clauses.size());
    reportf("# Verified %d original clauses.\n#\n", clauses.size());
}

void MinisatSMTSolver::checkLiteralCount()
{
    // Check that sizes are calculated correctly:
    int cnt = 0;
    for (int i = 0; i < clauses.size(); i++)
        if (clauses[i]->mark() == 0)
            cnt += clauses[i]->size();

    if ((int)clauses_literals != cnt){
        fprintf(stderr, "literal count: %d, real value = %d\n", (int)clauses_literals, cnt);
        assert((int)clauses_literals == cnt);
    }
}

//=================================================================================================
// Added code

void MinisatSMTSolver::printTrail( )
{
  for (int i = 0; i < trail.size(); i++)
  {
    Var v = var(trail[i]);
    cerr << "# Trail[" << i << "] lev [" << level[var(trail[i])] << "] ";
    printLit( trail[i] );
    cerr << " " << (sign(trail[i]) ? "!" : " " ) << theory_handler->varToEnode( v );
    cerr << endl;
  }
}

void MinisatSMTSolver::printModel( )
{
  cerr << "(and";
  for (Var v = 0; v < model.size(); v++)
  {
    Enode * e = theory_handler->varToEnode( v );
    assert( model[ v ] != l_Undef );
    int tmp;
    if ( model[ v ] == l_True 
      && sscanf( e->getCar( )->getName( ), "CNF_DEF_%d", &tmp ) != 1 )
      cerr << " " << e;
  }
  cerr << ")" << endl;
}

// Added code
//=================================================================================================
