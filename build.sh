#!/bin/bash

if [ ! -d ./bin ]; then
	mkdir ./bin
fi

code="$PWD"
opts="-g -std=c99"
cd bin > /dev/null
gcc $opts $code/calc.c -o calculate.exe
cd $code > /dev/null
