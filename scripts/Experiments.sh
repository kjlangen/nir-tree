#!/bin/bash

# Compile
EXP=1 make clean all

# R
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform2.1000.a.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform2.1000.b.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform2.1000.c.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform2.1000.d.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform2.1000.e.out

# R+
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform2.1000.a.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform2.1000.b.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform2.1000.c.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform2.1000.d.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform2.1000.e.out

# R*
bin/main -t 2 -m 3 -a 25 -b 50 > data/rstartree.cali.50.a.out
bin/main -t 2 -m 3 -a 25 -b 50 > data/rstartree.cali.50.b.out
bin/main -t 2 -m 3 -a 25 -b 50 > data/rstartree.cali.50.c.out
bin/main -t 2 -m 3 -a 25 -b 50 > data/rstartree.cali.50.d.out
bin/main -t 2 -m 3 -a 25 -b 50 > data/rstartree.cali.50.e.out

# NIR
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform2.1000.a.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform2.1000.b.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform2.1000.c.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform2.1000.d.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform2.1000.e.out

# Set dimensionality
sed -i 's/-DDIM=2/-DDIM=3/' Makefile

# Compile
EXP=1 make clean all

# R
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform3.1000.a.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform3.1000.b.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform3.1000.c.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform3.1000.d.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform3.1000.e.out

# R+
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform3.1000.a.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform3.1000.b.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform3.1000.c.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform3.1000.d.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform3.1000.e.out

# R*
bin/main -t 2 -m 4 -a 25 -b 50 > data/rstartree.bio.50.a.out
bin/main -t 2 -m 4 -a 25 -b 50 > data/rstartree.bio.50.b.out
bin/main -t 2 -m 4 -a 25 -b 50 > data/rstartree.bio.50.c.out
bin/main -t 2 -m 4 -a 25 -b 50 > data/rstartree.bio.50.d.out
bin/main -t 2 -m 4 -a 25 -b 50 > data/rstartree.bio.50.e.out

# NIR
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform3.1000.a.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform3.1000.b.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform3.1000.c.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform3.1000.d.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform3.1000.e.out

# Set dimensionality
sed -i 's/-DDIM=3/-DDIM=5/' Makefile

# Compile
EXP=1 make clean all

# R
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform5.1000.a.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform5.1000.b.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform5.1000.c.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform5.1000.d.out
#bin/main -t 0 -m 0 -n 10000000 -b 1000 > data/rtree.uniform5.1000.e.out

# R+
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform5.1000.a.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform5.1000.b.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform5.1000.c.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform5.1000.d.out
#bin/main -t 1 -m 0 -n 10000000 -b 1000 > data/rplustree.uniform5.1000.e.out

# R*
bin/main -t 2 -m 5 -a 25 -b 50 > data/rstartree.forest.50.a.out
bin/main -t 2 -m 5 -a 25 -b 50 > data/rstartree.forest.50.b.out
bin/main -t 2 -m 5 -a 25 -b 50 > data/rstartree.forest.50.c.out
bin/main -t 2 -m 5 -a 25 -b 50 > data/rstartree.forest.50.d.out
bin/main -t 2 -m 5 -a 25 -b 50 > data/rstartree.forest.50.e.out

# NIR
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform5.1000.a.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform5.1000.b.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform5.1000.c.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform5.1000.d.out
#bin/main -t 3 -m 0 -n 10000000 -b 1000 > data/nirtree.uniform5.1000.e.out

# Set dimensionality
sed -i 's/-DDIM=5/-DDIM=2/' Makefile

