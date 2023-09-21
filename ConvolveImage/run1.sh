#!/bin/bash

make clean

rm output.png

make

./convolve filters/sobol-vert.filt images/squares.png output.png
