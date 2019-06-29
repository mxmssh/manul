#   manul - main module
#   -------------------------------------
#   Written and maintained by Maksim Shudrak <mshudrak@salesforce.com> <mxmssh@gmail.com>
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

'''
manul usefull functions
'''
STATS_FREQUENCY = 1
SHM_SIZE = 65535
IGNORE_ABORT = True
UPDATE = True

#https://github.com/torvalds/linux/blob/556e2f6020bf90f63c5dd65e9a2254be6db3185b/include/linux/signal.h#L330
critical_signals_linux = {signal.SIGQUIT, signal.SIGILL, signal.SIGTRAP, signal.SIGBUS, signal.SIGFPE, signal.SIGSEGV,
                          signal.SIGXCPU, signal.SIGXFSZ, signal.SIGSYS} #,signal.SIGABRT

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


def parse_config(file_path):
    content = open(file_path, 'r').readlines()
    additional_cmd = ""
    args_dict = dict()
    for i, line in enumerate(content):
        line = line.replace("\n", "")
        if line.startswith("#") or len(line) <= 1: # comments or empty lines
            continue
        line = line.replace(" ", "")
        if line.count("=") != 1:
            printing.ERROR("Wrong config format at line %d:%s, exiting" % (i, line))

        line = line.split("=") # [arg_name, value]
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
        #enumerating targets without name python in them
        child = psutil.Process(p.pid)
        if "python" in child.name() or "sh" == child.name(): # TODO: if target has python name than we can't stop it!
            continue
        created_at = child.create_time()
        elapsed = time.time() - created_at
        if elapsed > timeout:
            ids.append(p)
    return ids

def kill_all():
    pid = os.getpid()
    parent = psutil.Process(pid)
    children = parent.children(recursive=True)
    for p in children:
        try:
            p.send_signal(signal.SIGKILL)
        except psutil.NoSuchProcess as exc:
            pass # already dead

def watchdog(timeout):
    '''
    Watchdog runs infinitely and kills all idle processes
    :param timeout:
    :param target_name:
    :return:
    '''
    procs = dict()
    sig = signal.SIGTERM
    while True:
        # getting list of running targets
        try:
            proc_ids = get_list_of_idle_processes(timeout)
            for pid in proc_ids:
                target_id = pid.pid
                p = psutil.Process(pid.pid)
                printing.INFO(0, None, None, "Sending SIGTERM signal to %d %s" % (target_id, pid.name()))
                p.send_signal(sig)
        except psutil.NoSuchProcess as exc:
            pass # already dead
        time.sleep(timeout/4)


def extract_content(file_name):
    fd = open(file_name, 'rb')
    if not fd:
        printing.ERROR("Failed to open input file, error code, stopping")
    content = bytearray(fd.read())
    fd.close()
    return content

fd_dict = dict()
def save_content(data, output_file_path):
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

def convert_bitmap_from_string(data):
    # converting to shm
    bitmap_to_compare = list("\x00" * SHM_SIZE)  # TODO: duplicate, we have similar
    for i in range(0, SHM_SIZE):
        bitmap_to_compare[i] = ord(data[i])
    return bitmap_to_compare