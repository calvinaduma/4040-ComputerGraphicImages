#!/bin/bash

clear
make clean
make
rm output.png
./warper images/cameraman.png output.png
