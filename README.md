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

![Short Demo](misc/manul_short_usage.gif)

# Dependencies
1. psutil
2. Python 2.7 or Python 3.5 (prefered)

# Coverage-guided fuzzing

Currently, Manul supports two types of instrumentation: afl-gcc and DBI.

## Coverage-guided fuzzing (AFL instrumentation mode)

Instrument your target with ```afl-gcc``` and ```Address Sanitizier``` (recommended for better results). For example:
```
CC=afl-gcc CXX=afl-g++ CFLAGS=-fsanitize=address CXXFLAGS=-fsanitize=address cmake <path_to_your_target>
make -j 8
```

See [this instruction](http://lcamtuf.coredump.cx/afl/QuickStartGuide.txt) for more details.

## Coverage-guided fuzzing (DBI mode)

You don't need to instrument your target in this mode but you need to download the latest version of DynamoRIO framework for Windows or Linux. The working version of Intel PIN is provided with Manul. You can find it in the ```dbi_clients_src/pin/pin-3.6-97554-g31f0a167d-gcc-linux``` folder.

Manul is distributed with x86/x64 pre-compiled clients for Linux and Windows. You can find them in the following folders:
```
linux/dbi_32|dbi_64/afl-pin.so (Intel PIN client)
linux/dbi_32|dbi_64/libbinafl.so (DynamoRIO client)
win/dbi_32|dbi_64/binafl.dll
```
Unfortunately, DynamoRIO is not officially supported on OS X. Intel PIN client on OS X is not yet ported.

### Using DynamoRIO to fuzz blackbox binaries

You can find DynamoRIO release packages under the following link [DynamoRIO download page](https://github.com/DynamoRIO/dynamorio/releases). Choose one of the archive available e.g.:

DynamoRIO-i386-Linux-7.91.18103-0.tar.gz - 32bit Linux.

DynamoRIO-x86_64-Linux-7.91.18103-0.tar.gz - 64bit Linux.

DynamoRIO-Windows-7.91.18103-0.zip - 32/64bit Windows.

You have to uncomment the following lines in the ```manul.config``` file and provide correct path to DynamoRIO launcher and client.

```
# Choose DBI framework to provide coverage back to Manul ("dynamorio" or "pin"). Example dbi = dynamorio
dbi = dynamorio
# If dbi parameter is not None the path to dbi engine launcher and dbi client should be specified.
dbi_root = /home/max/DynamoRIO/bin64/drrun
dbi_client_root = /home/max/manul/linux/dbi_64/libbinafl.so
dbi_client_libs = None
```

IMPORTANT NOTE: you should use 32bit launcher and 32bit client to fuzz 32bit binaries and 64bit launcher and 64bit client for 64bit binaries!

### Using Intel PIN to fuzz blackbox binaries on Linux

TBD

# Command-Line Arguments

The most frequently used options can be provided via the command line. The more options are supported using configuration file (```manul.config```).

```
Example: python3 manul.py -i corpus -o out_dir -n 40 "target @@"

positional arguments:
  target_binary  The target binary and options to be executed (don't forget to include quotes e.g. "target e @@").

optional arguments:
  -h, --help     show this help message and exit
  -n NFUZZERS    Number of parallel fuzzers
  -s             Run dumb fuzzing (no code instrumentation)
  -c CONFIG      Path to config file with additional options (see Configuration File Options section below)
  -r             Restore previous session

Required parameters:
  -i INPUT       Path to directory with initial corpus
  -o OUTPUT      Path to output directory

```

# Configuration File Options

Manul is distributed with default manul.config file where user can find all supported options and examples of their usage. Options should be specified in the following format ```Format: <option_name> = <value>```. Symbol ```#``` can be used to comment some lines.

#### Dictionary

```dict = /home/max/dictionaries/test.dict```. AFL mutation strategy allows user to specify list of custom tokens that can be inserted at random places in the fuzzed file. Manul supports this functionality via this command line (absolute paths prefered).

### Mutator weights
```mutator_weights=afl:7,radamsa:2,my_mutator:1```. Mutators weights allows user to tell manul how many mutations per 10 executions should be performed by certain fuzzer. In this example, AFL mutator will be executed in 7/10 mutations, Radamsa 2/10 and some custom ```my_mutator``` will get 1/10. If you want to disable certain mutator, the weight should be assigned to 0 (e.g. ```mutator_weights=afl:0,radamsa:1,my_mutator:9```).


# Network Applications Fuzzing

# Fuzzing Command Line Arguments

# Adding Custom Mutator 
Custom mutator can be added in the following way.
1. TBD
2. Enable mutator by specifying its name using ```mutator_weights``` in ```manul.config```. E.g. ```mutator_weights=afl:7,radamsa:2,my_mutator:1```. NOTE: AFL and Radamsa should be always specified. If you want to disable them just assign 0 weights.

# Technical Details

## Clients Compilation

## Tests Compilation

## Status Screen

![Status Screen](misc/status_screen.jpg?raw=true "Status Screen")
