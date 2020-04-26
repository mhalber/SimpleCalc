#!/bin/bash

code="$PWD"
opts=-g
cd bin > /dev/null
g++ $opts $code/calc.c -o calculate.exe
cd $code > /dev/null
