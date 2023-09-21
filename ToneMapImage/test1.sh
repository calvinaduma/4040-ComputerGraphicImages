#!/bin/bash

make clean

make

./tonemap files/memorial.exr

make clean
