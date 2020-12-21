#!/bin/sh

#######################################################################
#
# Copyright 2012 Alexandre Albuquerque Arnt <qtgzmanager@gmail.com>
# All rights reserved.
#
# This script downloads QTGZManager source code from SourceForge.net
# and builds a package in /tmp according to it's SlackBuild.
#
#######################################################################

PRGNAM=qtgzmanager
VERSION=${VERSION:-1.0.3}

svn export \
svn://svn.code.sf.net/p/jtgzmanager/code/ \
$PRGNAM-$VERSION

# Copies slack-desc to the current dir
cp -a $PRGNAM-$VERSION/slack-desc .

# Creates the sources tarball
tar -cvvf $PRGNAM-$VERSION-src.tar $PRGNAM-$VERSION/
bzip2 $PRGNAM-$VERSION-src.tar

# Executes the SlackBuild
sh ./$PRGNAM.SlackBuild

# Removes temporary files.
rm -rf $PRGNAM-$VERSION
rm $PRGNAM-$VERSION-src.tar.bz2
rm slack-desc
