## SimpleCalc

Small command line utility for evaluating simple mathematical expressions. Implements a recursive descent parser converting input, infix expression to a postfix expression, which is then evaluated.

## Usage
Simply invoke the binary followed by some expression you wish to evaluate
~~~
$ calculate 2*2
2*2 = 24.000000
~~~

Supports `+`, `-`, `*` ,`/`, `^` operators, as well as `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `log` and `exp` functions.

## Build
On windows, run `build.bat` (assuming you are running a terminal that has developer tools, i.e. cl.exe)  
On linux/macos, run `./build.sh` (assuming you are running bash and have gcc)

