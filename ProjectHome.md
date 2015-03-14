# Info #

OpenSMT was created and lead by **Roberto Bruttomesso** while working as a postdoc at the University of Lugano (Formal Verification Group) from 2008 and 2011. Important contributions came from Edgar and Aliaksei.

The project in this repo is now discontinued after Roberto has joined industry. Chances are high that the project will be re-opened one day in a newer and more devastating architecture.

# Intro #

This page is the SVN **source code** repository for OpenSMT (click Source tab).

Quick tip: sources can be download with
`svn checkout http://opensmt.googlecode.com/svn/trunk/ opensmt`.

Also you need to install the following software:
  * gcc/g++ version >= 4.3.2
  * autotools
  * libtool
  * flex
  * bison

(for Ubuntu users `sudo apt-get install autoconf libtool g++ bison flex`).

In addition you need to install [The GNU MP Bignum Library](http://gmplib.org/).

More details are available from wiki page [BuildOpenSMTFromSources](BuildOpenSMTFromSources.md).

Precompiled **statically linked binaries**, and **source snapshots** are also available in the download section.

The discussion group is http://groups.google.com/group/opensmt.

If you find bugs please report them to us (using the Issues tab).

# Latest Version #

What you get from SVN

# Extension to Pseudo-Boolean Problems #

Anders Franzen developed an extension of OpenSMT to handle Pseudo-Boolean constraints, including optimization problems. The extension is called **PB/CT** and it is available [here](http://www.residual.se).

# License #

GNU GPL 3 -- If you need a different license please contact [Roberto Bruttomesso](http://www.inf.usi.ch/postdoc/bruttomesso).

# Questions #

Please post your questions directly to our discussion group at
http://groups.google.com/group/opensmt.

# Past Members/Contributors #

We wish to thank the following people for their contributions:
  * Aliaksei Tsitovich
  * Simone Fulvio Rollini
  * Edgar Pek
  * David Monniaux