# llubench #
## Source ## 
 - [Paper](http://zilles.cs.illinois.edu/papers/health.pdf) 
 - [Original source code](http://zilles.cs.illinois.edu/papers/llubenchmark.c)
 
## Modifications ##
 - Element size is changed to 64. 
 - A verbose mode is added to dump pointer values. 

## Build ## 
Run with the command
```
clang -O3 llubenchmark.c -o llubenchmark
```

## Run ## 
Run with the command 
```
./llubenchmark -i 1024 -g .333 -d -t -n 1024 -s 1048576 -v
```

