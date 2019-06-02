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

'''
manul config options
'''
STATS_FREQUENCY = 3
SHM_SIZE = 65535
IGNORE_ABORT = True
UPDATE = True

class FuzzerStats:
    def __init__(self):
        self.stats = OrderedDict()
        self.stats["iterations"] = 0.0
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


def convert_bitmap_from_string(data):
    # converting to shm
    bitmap_to_compare = list("\x00" * self.SHM_SIZE)  # TODO: duplicate, we have similar
    for i in range(0, SHM_SIZE):
        bitmap_to_compare[i] = ord(data[i])
    return bitmap_convered