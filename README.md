# Hash Map in C
Hash map implementation in C capable of storing multiple values per key. All key-value pairs are stored as strings (char*) and the values with the same key are stored in a linked list. This hash map uses double hashing with linear probing to deal with collisions.

## Usage
On Ubuntu terminal:
To compile and run tests:
```
$ make -f fmvm_adt.mk run
```
To remove all executables:
```
$ make -f fmvm_adt.mk clean
```
To check for memory leaks with sanitize and valgrind:
```
$ make -f fmvm_adt.mk memleaks
```

## Disclaimer
This work was submitted for the COMSM1201 Programming in C at University of Bristol in 2019.
