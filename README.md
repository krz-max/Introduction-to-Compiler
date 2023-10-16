# Introduction-to-Compiler
* [NYCU 2023 Fall] Introduction to Compiler
* Professor: [Yi-Ping You](https://www.cs.nycu.edu.tw/members/detail/ypyou)

## HW1: Lexical Definition
In this assignment, you have to write a scanner for the P language in lex. This document provides the lexical definitions of the P language. You should follow those lexical definitions to implement the scanner.
### Structure
* README.md
* /src
  * Makefile
  * scanner.l
* /report
  * READMD.md
### Build and Execute
* Get HW1 docker image: `make docker-pull`
* Activate docker environment: `./activate_docker.sh`
* Build: `make`
* Execute: `./scanner <input file>`
* Test: `make test`
#### Build project
TA will use src/Makefile to build your project by simply typing make clean && make. Normally, you don't need to modify this file in this assignment, but if you do, it is your responsibility to make sure this makefile has at least the same make targets we provided to you.
#### Test your scanner
We provide all the test cases in the test folder. Simply type make test at the root directory of your repository to test your scanner. The grade you got will be shown on the terminal. You can also check diff.txt in test/result directory to know the diff result between the outputs of your scanner and the sample solutions.

Please using student_ as the prefix of your own tests to prevent TAs from overriding your files. For example: student_identifier_test.

