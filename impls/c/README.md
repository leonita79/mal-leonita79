# mal - c

C mal uses the [linenoise](https://github.com/antirez/linenoise) library by antirez.

## garbage collection
C mal uses a generational mark-and-sweep garbage collector. After each call to rep(), new objects are collected or moved to the older generation.
