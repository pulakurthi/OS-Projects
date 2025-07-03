# Project 3: Concurrency

Refer: \
https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/concurrency-pzip \
https://pages.cs.wisc.edu/~remzi/Classes/537/Spring2018/Projects/p1a.html

note: the code was developed on macOS
compile using gcc -o wzip wzip.c -Wall -Werror -pthread -O

RUN: ./test-wzip.sh -v \
OUTPUT: \
running test 1: basic test - some 'a' characters  \
test:      ./wzip tests/1.in \
test 1: passed \
 \
running test 2: multiple files on command line  \
test:      ./wzip tests/1.in tests/1.in tests/1.in \
test 2: passed \
 \
running test 3: no files (error) \
test:      ./wzip \
test 3: passed \
 \
running test 4: multi-line file with some longer lines \
test:      ./wzip tests/4.in \
test 4: passed \
 \
running test 5: does compression always compress? \
test:      ./wzip tests/5.in \
test 5: passed \
 \
running test 6: large file with some compressability - prelude to a contest input? \
test:      ./wzip tests/6.in \
test 6: passed 
