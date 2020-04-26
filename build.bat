@echo off

IF NOT EXIST bin mkdir bin

SET dbg_opts= -GR- -EHa- -nologo -Zi -W4 -WX -wd4204 -wd4996
SET rel_opts= -GR- -EHa- -nologo -O2 -W4 -WX -wd4204 -wd4996
SET code=%cd%
PUSHD bin
cl.exe %dbg_opts% %code%\calc.c -Fecalculate.exe
POPD