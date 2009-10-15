#!/bin/bash
#
# Author: Roberto Bruttomesso
#
# Description: Given a name for a solver and a directory 
#              to be put in, the script automatically
#              sets up directories, Makefiles, and so on
# 
# Usage: sh create_tsolver.sh <solver name> <solver subdir>
#

# Checks that input parameters is exactly 2
if [ $# -ne 2 ]; then
  echo "Usage: sh create_tsolver.sh <solver name> <solver dir>" 
  echo "Example: sh create_tsolver.sh MySolver mysolver" 
  exit 1
fi

name=$1

# Some names cannot be used, as they clash with existing software constructs
# TODO: The list is incomplete
if [[ $name = "TSolver" || $name = "tsolver" ]]; then
  echo "Can't create solver: given name is forbidden"
fi

namelower=`echo $1 | tr "[:upper:]" "[:lower:]"`
nameupper=`echo $1 | tr "[:lower:]" "[:upper:]"`
dir=$2

# Checks that the directory does not exists
if [ -d src/tsolvers/$dir ]; then
  echo "Can't create solver: directory already exists"
  exit 1
fi

# Creates directory
mkdir src/tsolvers/$dir

# Copy empty solver as src/tsolvers/$dir
echo "Creating src/tsolvers/$dir/$name.C src/tsolvers/$dir/$name.h"
cp src/tsolvers/emptysolver/EmptySolver.C src/tsolvers/$dir/$name.C
cp src/tsolvers/emptysolver/EmptySolver.h src/tsolvers/$dir/$name.h

# Copy Makefile
echo "Creating src/tsolvers/$dir/Makefile.am"
cp src/tsolvers/emptysolver/Makefile.am src/tsolvers/$dir

# Adapt sources and Makefile
sed -i s/EmptySolver/$name/g src/tsolvers/$dir/*
sed -i s/emptysolver/$namelower/g src/tsolvers/$dir/Makefile.am 
sed -i s/EMPTYSOLVER/$nameupper/g src/tsolvers/$dir/$name.h

# Backing up src/tsolvers/Makefile.am to src/tsolvers/.Makefile.am.old
echo "[Backing up src/tsolvers/Makefile.am as src/tsolvers/.Makefile.am.old]"
cp -f src/tsolvers/Makefile.am src/tsolvers/.Makefile.am.old

# Adjusting src/tsolvers/Makefile.am
echo "Modifying src/tsolvers/$dir/Makefile.am"
subdirs=`grep SUBDIRS src/tsolvers/Makefile.am`
middle="noinst_LTLIBRARIES = libtsolvers.la\n\nINCLUDES=\$(config_includedirs)\n\nlibtsolvers_la_SOURCES = TSolver.h THandler.C THandler.h"
libadd=`grep LIBADD src/tsolvers/Makefile.am`
echo "$subdirs $dir" > src/tsolvers/Makefile.am
echo >> src/tsolvers/Makefile.am
echo -e $middle >> src/tsolvers/Makefile.am
echo "$libadd $dir/lib$namelower.la" >> src/tsolvers/Makefile.am

# Backing up src/tsolvers/Makefile.am to src/tsolvers/.Makefile.am.old
echo "[Backing up configure.ac as .configure.ac.old]"
cp -f configure.ac .configure.ac.old

# Adjusting configure.ac
echo "Modifying configure.ac"
awk -v dirawk=$dir '// { if ( $0 ~ /-I\\\${top_srcdir}\/src\/tsolvers \\\\\\/ ) printf("%s\n%s%s%s\n", $0, "-I\\${top_srcdir}/src/tsolvers/", dirawk, " \\\\\\" ); else print $0; }' configure.ac > /tmp/conf.tmp
cp /tmp/conf.tmp configure.ac
awk -v dirawk=$dir '// { if ( $0 ~ /src\/tsolvers\/Makefile \\/ ) printf("%s\n%s%s%s\n", $0, "                  src/tsolvers/", dirawk, "/Makefile \\" ); else print $0; }' configure.ac > /tmp/conf.tmp
cp /tmp/conf.tmp configure.ac

echo "Reconfiguring"
autoreconf &> /dev/null

echo "*************************************************************"
echo "* Done                                                      *"
echo "*************************************************************"
echo "*                                                           *"
echo "* Don't forget to adjust:                                   *"
echo "* - inform( ) (src/tsolvers/$dir)                           *"
echo "* - belongToT( ) (src/tsolvers/$dir)                        *"
echo "* - push/popBacktrackPoint( ) (src/tsolvers/$dir)           *"
echo "* - initialize the solver                                   *"
echo "*   (src/egraph/EgraphSolver.C:initializeTheorySolvers.C)   *"
echo "*                                                           *"
echo "*************************************************************"
