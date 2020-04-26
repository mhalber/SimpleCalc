@echo off

set opts= -GR- -EHa- -nologo -Zi -W4 -WX -wd4204 -wd4996
rem set opts= -GR- -EHa- -nologo -O2 -W4 -WX -wd4204 -wd4996
set code=%cd%
pushd bin
cl.exe %opts% %code%\calc.c -Fecalculate.exe
popd