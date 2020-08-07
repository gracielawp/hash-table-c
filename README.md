# Hash Map in C
Hash map implementation in C capable of storing multiple values per key. All key-value pairs are stored as strings (char*) and the values with the same key are stored in a linked list. This hash map uses double hashing with linear probing to deal with collisions.

## Usage
#### On Ubuntu terminal:
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
#### Example of usage: finding homophones
The words with their phonemes are stored in cmudict.txt. To compile the program:
```
$ make -f hom.mk all
```
To find the homophones of a specific word, specify the number of phonemes that will match with the word, i.e. homophones of the word 'rhyme' with 3 identical phonemes are prime and crime.
```
$ ./homophones -n 3 RHYME
Output: RHYME (R AY1 M): PRIME RHYME ANTICRIME (1) CRIME ANTICRIME GRIME RIME
```

## Disclaimer
This work was submitted for the COMSM1201 Programming in C at University of Bristol in 2019.
