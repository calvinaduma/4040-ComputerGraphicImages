#!/bin/bash

make clean

rm output.png

make

./convolve filters/myfilt1.filt images/waves.png output.png
