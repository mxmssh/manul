# Manul
![Travis](https://travis-ci.org/mxmssh/manul.svg?branch=master)
![AppVeyor](https://ci.appveyor.com/api/projects/status/yj94vopjdrq7kdhe?svg=true)

![Logo](misc/manul_logo.jpg?raw=true "ASCII Logo of Manul")

Manul is a coverage-guided parallel fuzzer for open-source and black-box binaries on Windows, Linux and macOS (beta) written in pure Python.



# Quick Start
```
pip3 install psutil
git clone https://github.com/mxmssh/manul
cd manul
mkdir in
mkdir out
echo "AAAAAA" > in/test
python3 manul.py -i in -o out -n 4 "linux/test_afl @@"
```

### Installing Radamsa

```
sudo apt-get install gcc make git wget
git clone https://gitlab.com/akihe/radamsa.git && cd radamsa && make && sudo make install
```

There is no need to install radamsa on Windows, Manul is distributed with radamsa native library on this platform.



# List of Public CVEs

| CVE IDs                                      | Product  | Finder         |
|----------------------------------------------|----------|----------------|
| CVE-2019-9631 CVE-2019-7310 CVE-2019-9959    | Poppler  | Maksim Shudrak |
| CVE-2018-17019 CVE-2018-16807 CVE-2019-12175 | Bro/Zeek | Maksim Shudrak |

If you managed to find a new bug using Manul please contact me and I will add you in the list.



# Demo

![Short Demo](misc/manul_screen_demo.gif)



# Dependencies
1. [psutil](https://pypi.org/project/psutil/)
2. Python 2.7+ (will be deprecated after 1 Jan. 2020) or Python 3.7+ (preferred)



# Coverage-guided fuzzing

Currently, Manul supports two types of instrumentation: AFL-based (afl-gcc, afl-clang and [afl-clang-fast](https://github.com/mirrorer/afl/tree/master/llvm_mode)) and DBI.

## Coverage-guided fuzzing (AFL instrumentation mode)

Instrument your target with ```afl-gcc``` or ```afl-clang-fast``` and ```Address Sanitizer``` (recommended for better results). For example:
```
CC=afl-gcc CXX=afl-g++ CFLAGS=-fsanitize=address CXXFLAGS=-fsanitize=address cmake <path_to_your_target>
make -j 8
```

```
USE_ASAN=1 CC=afl-clang-fast CXX=afl-clang-fast++ cmake <path_to_your_target>
make -j 8
```

See [these instructions](http://lcamtuf.coredump.cx/afl/QuickStartGuide.txt) for more details.

## Coverage-guided fuzzing (DBI mode)

You don't need to instrument your target in this mode but you need to download the latest version of DynamoRIO framework for Windows or Linux. The working version of Intel PIN is provided with Manul. You can find it in the ```dbi_clients_src/pin/pin-3.6-97554-g31f0a167d-gcc-linux``` folder.

Manul is distributed with x86/x64 precompiled clients for Linux and Windows. You can find them in the following folders:
```
linux/dbi_32|dbi_64/afl-pin.so (Intel PIN client)
linux/dbi_32|dbi_64/libbinafl.so (DynamoRIO client)
win/dbi_32|dbi_64/binafl.dll
```
Unfortunately, DynamoRIO is not officially supported on OS X. Intel PIN client on OS X is not yet ported.

### Using DynamoRIO to fuzz black-box binaries

You can find DynamoRIO release packages at [DynamoRIO download page](https://github.com/DynamoRIO/dynamorio/wiki/Downloads). The supported version of DynamoRIO is 7.0.0-RC1 (see the next section if you need the latest version of DynamoRIO).

You have to uncomment the following lines in the ```manul.config``` file and provide correct path to DynamoRIO launcher and client.

```
# Choose DBI framework to provide coverage back to Manul ("dynamorio" or "pin"). Example dbi = dynamorio
dbi = dynamorio
# If dbi parameter is not None the path to dbi engine launcher and dbi client should be specified.
dbi_root = /home/max/DynamoRIO/bin64/drrun
dbi_client_root = /home/max/manul/linux/dbi_64/libbinafl.so
dbi_client_libs = None
```

IMPORTANT NOTE: You should use 32-bit launcher and 32-bit client to fuzz 32-bit binaries and 64-bit launcher and 64-bit client for 64-bit binaries!

#### Compiling DynamoRIO client library

If you want to use the latest version of [DynamoRIO](https://github.com/DynamoRIO/dynamorio/releases/download/) you need to compile instrumentation library from source code (see example below). The source code of instrumentation library can be found in ```dbi_clients_src``` located in the Manul main folder. On Windows, the compilation command (```cmake```) is the same as on Linux.

```
64-bit Linux

cd dbi_clients_src
wget https://github.com/DynamoRIO/dynamorio/releases/download/cronbuild-7.91.18124/DynamoRIO-x86_64-Linux-7.91.18124-0.tar.gz
tar xvf DynamoRIO-x86_64-Linux-7.91.18124-0.tar.gz
mkdir client_64
cd client_64
cmake ../dr_cov/ -DDynamoRIO_DIR=/home/max/manul/dbi_clients_src/DynamoRIO-x86_64-Linux-7.91.18124-0/cmake
make
```

If you need to compile 32-bit library, you should download DynamoRIO-i386-Linux-```*```.tar.gz archive instead of x86_64 and specify ```CFLAGS=-m32 CXXFLAGS=-m32``` before ```cmake``` command.


### Using Intel PIN to fuzz black-box binaries on Linux

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

Manul is distributed with default ```manul.config``` file where user can find all supported options and usage examples. Options should be specified in the following format ```Format: <option_name> = <value>```. Symbol ```#``` can be used to ignore a line.

#### Dictionary

```dict = /home/max/dictionaries/test.dict```. AFL mutation strategy allows user to specify a list of custom tokens that can be inserted at random places in the fuzzed file. Manul supports this functionality via this option (absolute paths preferred).

#### Mutator weights
```mutator_weights=afl:7,radamsa:2,my_mutator:1```. Mutator weights allow user to tell Manul how many mutations per 10 executions should be performed by certain fuzzer. In this example, AFL mutator will be executed in 7/10 mutations, Radamsa 2/10 and some custom ```my_mutator``` will get 1/10. If you want to disable certain mutator, the weight should be assigned to 0 (e.g. ```mutator_weights=afl:0,radamsa:1,my_mutator:9```).

#### Deterministic Seed (Radamsa Option)
```deterministic_seed = False|True```. By providing ```True```, Radamsa mutations will become deterministic thereby each run of Manul will lead to same outputs.

#### Print Summary per Thread
```print_per_thread = False|True```. By enabling this option, Manul will print summary for each thread being executed instead of total summary.

#### Disable Volatile Paths
```disable_volatile_bytes = False|True``` By enabling this option, Manul will not blacklist volatile paths.

#### AFL's forkserver (only UNIX)

```forkserver_on = False|True``` Enable or disable AFL's [forkserver](https://lcamtuf.blogspot.com/2014/10/fuzzing-binaries-without-execve.html).

#### DBI Options
```dbi = dynamorio|pin```. This option tells Manul which DBI framework will be used to instrument the target.

```dbi_root = <path>```. This options tells Manul where to find DBI framework main launcher.

```dbi_client_root = <path>```. This options tells Manul where to find DBI client to perform instrumentation.

```dbi_client_libs = name_#1,name_#2|None```. This option can be used to specify list of libraries that need to be instrumented along with the main target (e.g. you have executable that loads the target library where you want to find bugs).

#### Timeout
```timeout = 10```. Time to wait before kill the target and send the next test case.

#### init_wait
```init_wait = 1```. This option can be used to setup a timeout required for target to initialize.

#### Netslave and Netmaster Options

The options ```net_config_master``` and ```net_config_slave``` are used to distribute Manul instances over network. You have to perform the following 3 steps to run distributed fuzzing.
1. Create a file with a list of hosts in the following format: ```IP:port``` where your slaves will be executed.
2. Start all Manul slave instances on remote machines (with all required options and path to target binary) and enable the following option:
```net_config_slave = 0.0.0.0:1337```. Manul will launch the instance and will wait for incoming connection from master instance on port 1337.
3. Start the master instance and provide the file with a list of slave instances created on Step 1 using ```net_config_master = file_name```.

#### Debug Mode
```debug = False|True``` - print debug info.

```logging_enable = False|True``` - save debug info in the log.

#### Logo
```manul_logo = False|True``` - print Manul logo at the beginning.

#### Disable Stats
```no_stats = False|True``` - save statistics.

#### Bitmap Synchronization Frequency (5000 recommended for DBI mode)
```sync_freq = 10000```. Allows user to change coverage bitmap synchronization frequency. This options tells Manul how often it should synchronize coverage between parallel fuzzing instances. Lower value decreases performance but increases coordination between instances.

#### Custom Path to Save Output
```#custom_path = test_path``` - this option allows to save the test case in the custom folder (if target wants to load it from some predefined place).

#### Command Line Fuzzing (experimental)
```cmd_fuzzing = True|False```. If this option is enabled, Manul will provide the input in the target via command line instead of saving in the file.

#### Ignore Signals
```user_signals = 6,2,1|None```. User can tell Manul which signals from the target should be ignored (not considered as crash).

#### Network Fuzzing (experimental)

```target_ip_port = 127.0.0.1:7715|None``` - used to specify target IP and PORT.
```target_protocol = tcp|tcp``` - used to specify the protocol to send input in the target over network.
```net_sleep_between_cases = 0.0```. This option can be used to define a delay between test cases being send in the target.

Currently, network fuzzing is an experimental feature (see issues for more details).



# Adding Custom Mutator
Custom mutator can be added in the following three steps:
Step 1. Create a python (.py) file and give it some name (e.g. example_mutator.py)

Step 2. Create two functions ```def init(fuzzer_id)``` and ```def mutate(data)```. See [example_mutator](example_mutator.py) for more details. Manul will call ```init``` function during fuzzing initialization and ```mutate``` for each file being provided into the target.

Step 3. Enable mutator by specifying its name using ```mutator_weights``` in ```manul.config```. E.g. ```mutator_weights=afl:2,radamsa:0,example_mutator:8```.

NOTE: AFL and Radamsa mutators should always be specified. If you want to disable AFL and/or Radamsa just assign 0 weights to them.

## Technical Details
TBD

## Status Screen

![Status Screen](misc/status_screen.jpg?raw=true "Status Screen")
