AC_PREREQ(2.59)
AC_INIT([OpenSMT], [0.2], [roberto.bruttomesso@gmail.com])
AC_CONFIG_AUX_DIR(autotools)
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
AM_PROG_LEX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_DISABLE_SHARED
AC_PROG_LIBTOOL
#
# Statically linked version
#
AC_ARG_ENABLE(statically,
  AC_HELP_STRING([--enable-statically],
                 [enable statically linked libraries (--static) (default=no)]),
  enablestatically=$enableval,
  enablestatically="no")

if test $enablestatically = "yes" ;
then
  config_statically="--static"
elif test $enablestatically = "no" ;
then
  config_statically="" 
else
  config_statically=$enablestatically
fi
echo "setting static flags to... ${config_statically:-none}"
# 
# Profiling
#
AC_ARG_ENABLE(profiling,
  AC_HELP_STRING([--enable-profiling],
                 [enable profiling (-pg) (default=no)]),
  enableprofiling=$enableval,
  enableprofiling="no")

if test $enableprofiling = "yes" ;
then
  config_profiling="-pg"
elif test $enableprofiling = "no" ;
then
  config_profiling=""
else
  config_profiling=$enableprofiling
fi
echo "setting profiling flags to... ${config_profiling:-none}"
#
# Optimization
#
AC_ARG_ENABLE(optimization,
  AC_HELP_STRING([--enable-optimization],
                 [enable optimization by compiler (-O3) (default=yes)]),
  enableoptimization=$enableval,
  enableoptimization="yes")

if test $enableoptimization = "yes" ;
then
  config_optimization="-O3 -DOPTIMIZE -DNDEBUG"
elif test $enableoptimization = "no" ;
then
  config_optimization=""
else
  config_optimization=$enableoptimization
fi
echo "setting optimization flags to... ${config_optimization:-none}"
#
# Enable Pedantic Assertion Checking
#
AC_ARG_ENABLE(pedantic_debug,
  AC_HELP_STRING([--enable-pedantic_debug],
                 [enable pedantic assertion checking (default=no)]),
  enablepedantic_debug=$enableval,
  enablepedantic_debug="no")

if test $enablepedantic_debug = "yes" ;
then
  config_pedantic_debug="-DPEDANTIC_DEBUG"
elif test $enablepedantic_debug = "no" ;
then
  config_pedantic_debug=""
else
  config_pedantic_debug=$enablepedantic_debug
fi
echo "setting pedantic debug flags to... ${config_pedantic_debug:-none}"
#
# Enable External Tool Checking
#
AC_ARG_ENABLE(external_tool,
  AC_HELP_STRING([--enable-external_tool],
                 [enable external tool checking (default=no)]),
  enableexternal_tool=$enableval,
  enableexternal_tool="no")

if test $enableexternal_tool = "yes" ;
then
  config_external_tool="-DEXTERNAL_TOOL"
elif test $enableexternal_tool = "no" ;
then
  config_external_tool=""
else
  config_external_tool=$enableexternal_tool
fi
echo "setting external_tool flags to... ${config_external_tool:-none}"
#
# SMTCOMP version
#
AC_ARG_ENABLE(smtcomp,
  AC_HELP_STRING([--enable-smtcomp],
                 [enable smtcomp mode (default=no)]),
  enablesmtcomp=$enableval,
  enablesmtcomp="no")

if test $enablesmtcomp = "yes" ;
then
  config_smtcomp="-DSMTCOMP -march=opteron --static"
elif test $enablesmtcomp = "no" ;
then
  config_smtcomp=""
else
  config_smtcomp=$enablesmtcomp
fi
echo "setting smtcomp flags to... ${config_smtcomp:-none}"

# Complain if both pedantic debug and optimization are enabled
if test $enablepedantic_debug = "yes" -a $enableoptimization = "yes" ;
then
  echo "Error: cannot enable both pedantic debug and optimization"
  exit
fi

# Complain if both external tool and optimization are enabled
if test $enableexternal_tool = "yes" -a $enableoptimization = "yes" ;
then
  echo "Error: cannot enable both external_tool and optimization"
  exit
fi

# Complain if smtcomp is enabled and optimization is disabled
if test $enablesmtcomp = "yes" -a $enableoptimization = "no" ;
then
  echo "Error: smtcomp requires optimizations"
  exit
fi

AM_CFLAGS="-W -Wall -g -Wno-deprecated $config_profiling $config_optimization $config_external_tool $config_pedantic_debug $config_smtcomp $config_statically"
AM_CXXFLAGS="-W -Wall -g  -Wno-deprecated $config_profiling $config_optimization $config_external_tool $config_pedantic_debug $config_smtcomp $config_statically"
CFLAGS=""
CXXFLAGS=""
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
#
# List of directories to include
#
config_includedirs="-I\${top_srcdir}/src \\\
-I\${top_srcdir}/src/otl \\\
-I\${top_srcdir}/src/minisat/mtl \\\
-I\${top_srcdir}/src/minisat/core \\\
-I\${top_srcdir}/src/egraph \\\
-I\${top_srcdir}/src/smtsolvers \\\
-I\${top_srcdir}/src/simplifiers \\\
-I\${top_srcdir}/src/cnfizers \\\
-I\${top_srcdir}/src/tsolvers \\\
-I\${top_srcdir}/src/tsolvers/emptysolver \\\
-I\${top_srcdir}/src/tsolvers/bvsolver \\\
-I\${top_srcdir}/src/tsolvers/bvsolver/minisatp \\\
-I\${top_srcdir}/src/tsolvers/lrasolver \\\
-I\${top_srcdir}/src/tsolvers/dlsolver"

AC_SUBST(config_includedirs)

AC_CONFIG_FILES([ \
		  Makefile \
		  src/Makefile \
		  src/parsers/Makefile \
		  src/parsers/smt/Makefile \
		  src/parsers/ys/Makefile \
		  src/egraph/Makefile \
		  src/cnfizers/Makefile \
		  src/smtsolvers/Makefile \
		  src/simplifiers/Makefile \
		  src/tsolvers/Makefile \
		  src/tsolvers/emptysolver/Makefile \
		  src/tsolvers/bvsolver/Makefile \
		  src/tsolvers/lrasolver/Makefile \
		  src/tsolvers/bvsolver/minisatp/Makefile \
		  src/tsolvers/dlsolver/Makefile \
		  src/otl/Makefile \
		  ])
AC_OUTPUT
