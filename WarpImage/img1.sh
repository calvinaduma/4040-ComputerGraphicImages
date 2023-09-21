#!/bin/bash

clear
make clean
make
rm output.png
./warper images/imgtest.png output.png
