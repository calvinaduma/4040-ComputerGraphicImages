#!/bin/bash

make clean

rm output.png

make

./convolve filters/myfilt2.filt images/Lena.png output.png
