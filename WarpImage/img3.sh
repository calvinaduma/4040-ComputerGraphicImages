#!/bin/bash

clear
make clean
make
rm output.png
./warper images/rhino.png output.png
