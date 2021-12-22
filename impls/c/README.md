# mal - c

C mal uses the [linenoise](https://github.com/antirez/linenoise) library by antirez.

## garbage collection
Mal values are tagged unions that contain either a numeric value or a pointer to additional memory.

If possible, this is shared with the input string or allocated from a dynamically-sized buffer called the mal stack and freed between calls to rep(). Longer-term objects are managed through reference counting. Reference counted objects cannot contain pointers to the stack and stack objects do not increase the reference count of objects they contain.

