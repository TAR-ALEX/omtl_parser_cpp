# omtl_parser_cpp

Library for parsing OMTL structures (Object Message Tuple List). This format is similar to json in functionality and follows the following pattern,


### Tokens

A Token is the basic element. A token can either be a number, string, name, or comment.

Number: `-10.0001` any decimal number

Comment: `(this is a comment)` anything in parenthesis

String: `"This is a string\t\n"` anything in quotes and escaped

Name: `x` anything else


### Statments

A Statment follows the following format: unique tokens must be separated by whitespace.

Example statement: `x + y z m 0.1 var1 "string is here" (some comment)`


### Tuples

A Tuple/List is an entity that starts with a `[` symbol and ends with a `]` symbol. All entries must be separated by commas (`,`). A tuple consists only of stetements. Statments may be given names using a colon (`:`) before the Statement. Tuples may contain other Tuples.

Example tuple: `[val1: 2, val2: 3, 4, [5, 6]]`



The makefile will build and run the main file. (modify the main file to try out the library)

To use this project with a dependency manager install the cpp-dependency-manager project from https://github.com/TAR-ALEX/Cpp-Dependency-Manager.git

and create a vendor.txt file and add the following entries:

```
git "https://github.com/TAR-ALEX/estd-string-util.git" master "./include" "./vendor/include",
git "https://github.com/TAR-ALEX/big_numbers_cpp.git" master "./include" "./vendor/include",
git "https://github.com/TAR-ALEX/estd_logging.git" master "./include" "./vendor/include",
git "https://github.com/TAR-ALEX/clone_ptr_cpp.git" master "./include" "./vendor/include",
git "https://github.com/TAR-ALEX/omtl_parser_cpp.git" master "./include" "./vendor/include",

```
