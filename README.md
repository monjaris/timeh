# What is timeh?
When developing C++ projects, compilation times and where it comes from is ambugious and annoying to deal with.
Timeh lets see you see how much time a header takes to compile on its own.
It also can print all the headers including others.

simple example:
```sh
timeh include/common.hpp
```
possible output:
```
duration: 305.959401 ms
```


# Installation
To build timeh, you need xmake on your system. first install it. then:
```sh
git clone https://github.com/monjaris/timeh
cd ./timeh
./build.sh  # invokes xmake
```
thats it. binary copied to project directory, try `./timeh -t iostream`


# Usage
Timeh can do two main jobs.
1. show how much time a header takes from you
2. which headers include which headers behind the scenes

1)
```sh
timeh iostream
timeh SDL3/SDL.h  # searchs inside /usr/include/
timeh ./include/parser.hpp
```

2)
```sh
timeh -t string  # shows a tree of includes
timeh -t vexa/vexa.hpp
```
