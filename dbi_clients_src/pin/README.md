Intro
=====
This is the fastest pintool afl-fuzzer out there currently.
And it runs with pintool 3.6, so 4.x x64 kernels are fine.
But ... pintool is super slow.
So this is basically only if you have no other option.
I am currently developing an alternative with DynamoRIO and is 10x faster -
but still, afl qemu mode is 5-10x faster than that ...


Installation
============
1. download, compile and install afl => https://github.com/mirrorer/afl
Optional: 2. download, compile and install dyninst => https://github.com/dyninst/dyninst
Optional: 3. download, compile and install afl-dyninst  => https://github.com/vanhauser-thc/afl-dyninst
4. download and unpack pin => https://software.intel.com/en-us/articles/pintool-downloads (download 3.6)
5. export PIN_ROOT=/path/to/pin directory
6. make a symlink to the afl folder in the afl-pin directory named "afl" , e.g. "ln -s ../afl-2.52b afl"
7. make
8. make install


Options
=======
-libs               also instrument the dynamic libraries
-exitpoint target   exit the program when this address/function is reached. speeeed!
-forkserver         install a forkserver. You must set PIN_APP_LD_PRELOAD - or use afl-fuzz-pin.sh
-entrypoint target  function or address where you want to install the forkserver
-alternative        a little bit faster but less quality


How to run
==========
Optional: 1. afl-dyninst.sh -i program -o program_instrumented -D
It is a good idea to add -e and -E with well selected function addresses to
make the fuzzing faster

2. afl-fuzz-pin.sh [normal afl-fuzz options]
If you did not do step 1, add the option -forkserver (-forkserver is faster
than afl-dyninst). You can increase speed more by selecting a good
"-entrypoint function_name" or "-entrypoing 0x123456" location.
That's it! If you fuzzing does not run, afl-fuzz might need more memory, set
AFL_MEM to a high value, e.g. 700 for 700MB
Using -forkserver requires
PIN_APP_LD_PRELOAD=/usr/local/lib/pintool/forkserver.so
but afl-fuzz-pin.sh takes care of this.


When to use it
==============
When you have no source code, normal afl-dyninst is crashing the binary,
qemu mode -Q is not an option and dynamorio is not working either.
Pin is even 90% slower than my dynamorio implementation ...


Limitations
===========
Pin is super slow ... it is the tool of last resort on x86/x64.


Who and where
=============
Marc "van Hauser" Heuse <mh@mh-sec.de> || <vh@thc.org>
https://github.com/vanhauser-thc/afl-pin
