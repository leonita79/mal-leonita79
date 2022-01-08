# mal - c

C mal uses the [linenoise](https://github.com/antirez/linenoise) library by antirez.
## Hash tables
Mal maps and environments use a custom hash table implementation that uses open addressing and linear probing. The hash function used is 32-bit [FNV-1A](http://www.isthe.com/chongo/tech/comp/fnv/index.html)  

## garbage collection
C mal uses a mark-and-sweep garbage collector that collects after each call to rep().
