# Manul

![Logo](misc/manul_logo.jpg?raw=true "ASCII Logo of Manul")

Manul is a coverage-guided parallel fuzzer for open-source and blackbox binaries on Windows, Linux and MacOS (beta) written in pure Python.

# Quick Start
```
pip3 install psutil
git clone https://github.com/mxmssh/manul
cd manul
mkdir in
mkdir out
echo "AAAAAA" > in
python3 manul.py -i in -o out -n 4 "linux/test_afl @@"
```

# Demo


# Dependencies
1. psutil
2. Python 2.7 or Python 3.5 (prefered)

# Coverage-guided fuzzing

Currently, Manul supports two types of instrumentation: afl-gcc and DBI.

## Coverage-guided fuzzing (AFL instrumentation mode)

Instrument your target with ```afl-gcc``` and ```Address Sanitizier``` (recommended). For example:
```
CC=afl-gcc CXX=afl-g++ CFLAGS=-fsanitize=address CXXFLAGS=-fsanitize=address cmake <path_to_your_target>
make -j 8
```

See [this instruction](http://lcamtuf.coredump.cx/afl/QuickStartGuide.txt) for more details.

## Coverage-guided fuzzing (DBI mode)

You don't need to instrument your target in this mode but you need to download appropriate DBI framework for Windows or Linux. Manul is distributed with x86/x64 pre-compiled clients for Linux and Windows. You can find them in the following folders:
```
linux/dbi_32|dbi_64/afl-pin.so (Intel PIN client)
linux/dbi_32|dbi_64/libbinafl.so (DynamoRIO client)
win/dbi_32|dbi_64/binafl.dll
```
Unfortunately, DynamoRIO is not officially supported on OS X. Intel PIN client on OS X is not yet ported.

### Using DynamoRIO to fuzz blackbox binaries on Linux

### Using DynamoRIO to fuzz blackbox binaries on Windows

### Using Intel PIN to fuzz blackbox binaries on Linux

# Command-Line Arguments

# Configuration File Options

# Network Applications Fuzzing

# Fuzzing Command Line Arguments

# Adding Custom Mutator 

# Technical Details

## Clients Compilation

## Tests Compilation
