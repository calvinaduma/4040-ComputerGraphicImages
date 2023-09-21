#!/bin/bash

clear
make clean
make
rm output.png
./warper images/Lena.png output.png
