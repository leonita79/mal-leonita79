# mal - perl

## numbers
perl mal uses the same numeric types as perl, parsing them with [Scalar::Util::looks\_like\_number](https://perldoc.perl.org/Scalar::Util#looks_like_number). This includes integers, decimals, and infinity.

While division is integer division as in standard mal, all four basic arithmetic functions take any number of arguments.   

## types
* Numbers, true, false, and nil are represented by perl scalars.
* Strings and Symbols are scalar references.
* Keywords are scalar references prepended by NUL ("\0").
* Lists and Vectors are array references
* Maps are hash references
* Functions and macros are code references

While mal types do not use object oriented perl code, the reference type is used to identify the type of the mal value

## functions, closures, and macros
User-defined functions are compiled into perl closures and stored as code references, the same as native functions.
Macros are distinguished from functions by the reference type

Macros and functions with metadata are compiled into wraping closures that use function goto to call the underlying function. This makes them slightly less efficient, but saves an indirection on normal calls.

## tail-call optimization
most functions that end with a call to EVAL use function goto to jump there. This avoids the loop recommended in the guide.

