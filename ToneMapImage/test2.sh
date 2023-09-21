#!/bin/bash

make clean

make

./tonemap files/vinesunset.hdr

make clean
