#   Manul - utils file
#   -------------------------------------
#   Maksim Shudrak <mshudrak@salesforce.com> <mxmssh@gmail.com>
#
#   Copyright 2019 Salesforce.com, inc. All rights reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#     http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

from collections import OrderedDict, namedtuple
import os
import printing
import time
import psutil
import signal
import sys
from helper import *
import zlib
import struct

'''
manul useful functions
'''
STATS_FREQUENCY = 1
SHM_SIZE = 65535
IGNORE_ABORT = True
UPDATE = True
MAX_SEED = 1024*1024*1024

PY3 = sys.version_info[0] == 3

if not sys.platform == "win32":
    #https://github.com/torvalds/linux/blob/556e2f6020bf90f63c5dd65e9a2254be6db3185b/include/linux/signal.h#L330
    critical_signals_nix = {signal.SIGQUIT, signal.SIGILL, signal.SIGTRAP, signal.SIGBUS, signal.SIGFPE, signal.SIGSEGV,
                              signal.SIGXCPU, signal.SIGXFSZ, signal.SIGSYS} #,signal.SIGABRT
else:
    critical_signals_nix = {}

class FuzzerStats:
    def __init__(self):
        self.stats = OrderedDict()
        self.stats["executions"] = 0.0
        self.stats["exceptions"] = 0.0
        self.stats["new_paths"] = 0.0
        self.stats["bitmap_coverage"] = 0.0
        self.stats["blacklisted_paths"] = 0.0
        self.stats["last_path_time"] = 0.0
        self.stats['unique_crashes'] = 0.0
        self.stats["crashes"] = 0.0
        self.stats["last_crash_time"] = 0.0
        self.stats["files_in_queue"] = 0.0
        self.stats["file_running"] = 0.0
        self.stats["exec_per_sec"] = 0.0
    def get_len(self):
        return len(self.stats)

if os.name == 'nt':
    class bcolors:
        HEADER = ''
        OKBLUE = ''
        OKGREEN = ''
        WARNING = ''
        GRAY = ''
        FAIL = ''
        ENDC = ''
        BOLD = ''
        UNDERLINE = ''
else:
    class bcolors:
        HEADER = '\033[95m'
        OKBLUE = '\033[94m'
        OKGREEN = '\033[92m'
        WARNING = '\033[93m'
        GRAY = '\033[30m'
        FAIL = '\033[91m'
        ENDC = '\033[0m'
        BOLD = '\033[1m'
        UNDERLINE = '\033[4m'

def bytes_to_int(byte_str):
    if PY3:
        return int.from_bytes(byte_str, "little")
    else:
        return struct.unpack("<L", byte_str)[0]

def parse_config(file_path):
    content = open(file_path, 'r').readlines()
    additional_cmd = ""
    args_dict = dict()
    for i, line in enumerate(content):
        line = line.replace("\n", "")
        if line.startswith("#") or len(line) <= 1:  # comments or empty lines
            continue
        line = line.replace(" ", "")
        if line.count("=") != 1:
            printing.ERROR("Wrong config format at line %d:%s, exiting" % (i, line))

        line = line.split("=")  # [arg_name, value]
        # parse Bool|Number|String
        value = line[1]
        if value == 'True':
            additional_cmd += "--%s " % (line[0])
        elif value == 'None' or value == 'False':
            continue
        else:
            additional_cmd += "--%s %s " % (line[0], value)

    additional_cmd = additional_cmd[:-1]
    return additional_cmd

def get_list_of_idle_processes(timeout):
    ids = list()
    pid = os.getpid()
    parent = psutil.Process(pid)
    children = parent.children(recursive=True)
    for p in children:
        # enumerating targets without name python in them
        child = psutil.Process(p.pid)
        if "python" in child.name() or "Python" in child.name() or "sh" == child.name():  # FYI: if target has python name we can't stop it
            continue
        created_at = child.create_time()
        elapsed = time.time() - created_at
        if elapsed > timeout:
            ids.append(p)
    return ids

def is_alive(pid):
    if psutil.pid_exists(pid):
        # alive but zombie ?
        status = None
        try:
            proc = psutil.Process(pid)
            status = proc.status()
        except psutil.NoSuchProcess as exc:
            return False # already dead
        if status and status == psutil.STATUS_ZOMBIE:
            printing.WARNING(None, "The process is alive but Zombie, killing it")
            kill_all(pid) # avoid Zombies in our environment
            return False
        return True
    else:
        return False

def kill_process(p):
    try:
        if sys.platform == "win32":
            p.kill()
        else:
            p.send_signal(signal.SIGKILL)
    except psutil.NoSuchProcess:
        pass

def kill_all(pid):
    try:
        parent = psutil.Process(pid)
        children = parent.children(recursive=True)
    except psutil.NoSuchProcess as exc:
        return
    for p in children:
        kill_process(p)
    kill_process(parent)

def watchdog(timeout):  # used only in python2
    '''
    Watchdog runs infinitely and kills all idle processes
    :param timeout:
    :param target_name:
    :return:
    '''
    procs = dict()
    if sys.platform == "win32":
        sig = signal.CTRL_BREAK_EVENT
    else:
        sig = signal.SIGTERM  # FYI: sometimes it might be handled by the target. sigkill can cause FP?

    while True:
        # getting list of running targets
        try:
            proc_ids = get_list_of_idle_processes(timeout)
            for pid in proc_ids:
                target_id = pid.pid
                p = psutil.Process(pid.pid)
                printing.INFO(0, None, None, "Sending SIGKILL signal to %d %s" % (target_id, pid.name()))
                p.send_signal(sig)
        except psutil.NoSuchProcess as exc:
            pass  # already dead
        time.sleep(timeout/4)


def extract_content(file_name):
    fd = open(file_name, 'rb')
    if not fd:
        printing.ERROR("Failed to open input file, error code, stopping")
    # check file size and return empty string if 0
    if not os.path.getsize(file_name):
        fd.close()
        return b""
    content = bytearray(fd.read())
    if not content:
        printing.ERROR("Unable to read data from the file %s" % file_name)

    fd.close()
    return content

fd_dict = dict()
def save_content_lin(data, output_file_path):
    global fd_dict
    fd = fd_dict.get(output_file_path, None)
    if not fd:
        fd = open(output_file_path, 'wb')
        fd_dict[output_file_path] = fd
    else:
        fd.seek(0, 0)
    if not fd:
        printing.ERROR("Failed to open output file, aborting")
    fd.write(data)
    fd.flush()
    #fd.close()
    return 1

# on Windows, it is better to close the file before we can send it into the target to avoid any sync problems
def save_content_win(data, output_file_path):
    fd = open(output_file_path, 'wb')
    if not fd:
        printing.ERROR("Failed to open output file, aborting")
    fd.write(data)
    fd.flush()
    fd.close() # TODO: find the way to avoid closing it on Windows
    return 1

def save_content(data, output_file_path):
    if sys.platform == "win32":
        save_content_win(data, output_file_path)
    else: # #TODO: test it on MacOS
        save_content_lin(data, output_file_path)


def is_bytearrays_equal(data1, data2):
    if not PY3:
        data1 = data1.decode()
        data2 = data2.decode()
    hash1 = zlib.crc32(data1)
    hash2 = zlib.crc32(data2)
    if hash1 != hash2:
        return False
    return True

def locate_diffs(data1, data2, length):
    f_loc = -1
    l_loc = -1
    for i in range(0, length):
        if data1[i] != data2[i]:
            if f_loc == -1: f_loc = i
            l_loc = i
    return f_loc, l_loc


# source of this function
#https://stackoverflow.com/questions/18092354/python-split-string-without-splitting-escaped-character
def split_unescape(s, delim, escape='\\', unescape=True):
    ret = []
    current = []
    itr = iter(s)
    for ch in itr:
        if ch == escape:
            try:
                # skip the next character; it has been escaped!
                if not unescape:
                    current.append(escape)
                current.append(next(itr))
            except StopIteration:
                if unescape:
                    current.append(escape)
        elif ch == delim:
            # split! (add current to the list and reset it)
            ret.append(''.join(current))
            current = []
        else:
            current.append(ch)
    ret.append(''.join(current))
    return ret

