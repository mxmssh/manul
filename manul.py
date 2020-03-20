#   Manul - main module
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

from os import listdir
from os.path import isfile, join
import shutil
from ctypes import *
import multiprocessing
import argparse
from timeit import default_timer as timer
import ntpath
from printing import *
from manul_utils import *
from manul_win_utils import *
import manul_network
import random
import afl_fuzz
import zlib
import importlib
import dbi_mode
import radamsa

PY3 = sys.version_info[0] == 3

if PY3:
    string_types = str,
    xrange = range
else:
    string_types = basestring,
    xrange = xrange

import subprocess, threading
import signal

net_process_is_up = None
net_sleep_between_cases = 0

INIT_WAIT_TIME = 0

class ForkServer(object):
    def __init__(self, timeout):
        self.control = os.pipe()
        self.status = os.pipe()
        self.r_fd = None
        self.timeout = timeout

    def init_forkserver(self, cmd):
        processid = os.fork()
        if processid:
            # This is the parent process
            time.sleep(INIT_WAIT_TIME)
            self.r_fd = os.fdopen(self.status[0], 'rb')
            res = self.r_fd.read(4)
            if len(res) != 4:
                ERROR("Failed to init forkserver")
            INFO(0, bcolors.OKGREEN, None, "Forkserver init completed successfully")
        else:

            # This is the child process

            os.dup2(self.control[0], 198)
            os.dup2(self.status[1], 199)

            null_fds = [os.open(os.devnull, os.O_RDWR) for x in xrange(2)]
            # put /dev/null fds on 1 and 2
            os.dup2(null_fds[0], 1)
            os.dup2(null_fds[1], 2)

            cmd = cmd.split()

            # TODO: we need to close some fds before we actually start execv
            # more details: https://lcamtuf.blogspot.com/2014/10/fuzzing-binaries-without-execve.html
            os.execv(cmd[0], cmd[0:])
            ERROR("Failed to start the target using forkserver")
            sys.exit(0) # this shouldn't be happen


    def run_via_forkserver(self):
        # TODO: timeouts for read/write otherwise we can wait infinitely

        res = os.write(self.control[1], b"go_!") # ask forkserver to fork
        if res != 4:
            ERROR("Failed to communicate with forkserver (run_via_forkserver, write). Unable to send go command")

        fork_pid = self.r_fd.read(4)
        if len(fork_pid) != 4:
            ERROR("Failed to communicate with forkserver (run_via_forkserver, read). Unable to confirm fork")

        status = self.r_fd.read(4) # TODO: we need timeout here because our target can go idle
        if len(status) != 4:
            ERROR("Failed to communicate with forkserver (run_via_forkserver, read). Unable to retrieve child status")

        return bytes_to_int(status)


class Command(object):
    def __init__(self, target_ip, target_port, target_protocol, timeout, fokserver_on, dbi_persistence_handler,
                 dbi_persistence_mode):
        self.process = None
        self.forkserver_on = fokserver_on
        self.forkserver_is_up = False
        self.forkserver = None
        self.returncode = 0

        if self.forkserver_on:
            self.forkserver = ForkServer(timeout)

        self.out = None
        self.err = None
        self.timeout = timeout
        if target_ip:
            self.target_ip = target_ip
            self.target_port = int(target_port)
            self.target_protocol = target_protocol
        self.net_class = None

        self.dbi_persistence_on = dbi_persistence_handler
        self.dbi_persistence_mode = dbi_persistence_mode
        self.dbi_restart_target = True

    def init_target_server(self, cmd):
        global net_process_is_up
        INFO(1, bcolors.BOLD, None, "Launching %s" % cmd)
        if sys.platform == "win32":
            self.process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        else:
            self.process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                            preexec_fn=os.setsid)
        if not is_alive(self.process.pid):
            ERROR("Failed to start target server error code = %d, output = %s" % (self.process.returncode, self.process.stdout))

        net_process_is_up = True
        time.sleep(INIT_WAIT_TIME)


    def net_send_data_to_target(self, data, net_cmd):
        global net_process_is_up

        if not net_process_is_up:
            INFO(1, None, None, "Target network server is down, starting")
            self.init_target_server(net_cmd)
            self.net_class = manul_network.Network(self.target_ip, self.target_port, self.target_protocol)

        if not net_process_is_up:  # is it the first run ?
            ERROR("The target network application is not started, aborting")

        self.net_class.send_test_case(data)

        time.sleep(net_sleep_between_cases)

        if not is_alive(self.process.pid):
            INFO(1, None, None, "Target is dead")
            if sys.platform == "win32":
                returncode = EXCEPTION_FIRST_CRITICAL_CODE  # just take the first critical
            else:
                returncode = 11
            net_process_is_up = False
            self.net_class = None

            return returncode, "[Manul message] Target is dead"

        return 0, ""


    def exec_command_forkserver(self, cmd):
        if not self.forkserver_is_up:
            self.forkserver.init_forkserver(cmd)
            self.forkserver_is_up = True
        status = self.forkserver.run_via_forkserver()

        return status


    def handle_dbi_pre(self):
        res = self.dbi_persistence_on.recv_command()
        if res == 'P':
            # our target successfully reached the target function and is waiting for the next command
            INFO(1, None, None, "Target successfully reached the target function (pre_handler)")
            send_res = self.dbi_persistence_on.send_command('F') # notify the target that we received command
        elif res == 'K' and self.dbi_persistence_mode == 2:
            INFO(1, None, None, "Target successfully reached the target function (pre_loop_handler) for the first time")
            send_res = self.dbi_persistence_on.send_command('P') # notify the target that we received command
        elif res == 'Q':
            INFO(1, None, None, "Target notified about exit (after post_handler in target)")
            self.dbi_restart_target = True
            return True
        elif res == 'T': # TODO can it happen when we are sending command ?
            self.dbi_restart_target = True
            return True
        else:
            ERROR("Received wrong command from the instrumentation library (pre_handler): %s" % res)

        return False


    def handle_dbi_post(self):
        res = self.dbi_persistence_on.recv_command()
        if res == 'K':
            INFO(1, None, None, "Target successfully exited from the target function (post_handler)")
        elif res == 'T':
            WARNING(None, "The target failed to answer within given timeframe, restarting")
            self.dbi_restart_target = True
            return 0
        elif res == "":
            WARNING(None, "No answer from the target, restarting.")
            # the target should be restarted after this (it can be a crash)
            self.dbi_restart_target = True
            return 1
        elif res == "C": # target sent crash signal, handling and restarting
            self.dbi_restart_target = True
            return 2
        else:
            ERROR("Received wrong command from the instrumentation library (post_handler)")
        return 0


    def exec_command_dbi_persistence(self, cmd):
        if self.dbi_restart_target:
            if self.process != None and is_alive(self.process.pid):
                INFO(1, None, None, "Killing the target")
                kill_all(self.process.pid)
            self.dbi_persistence_on.close_ipc_object() # close if it is not a first run

            self.dbi_persistence_on.setup_ipc_object()

            if sys.platform == "win32":
                self.process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                if not is_alive(self.process.pid):
                    ERROR("Failed to start the target error code = %d, output = %s" %
                          (self.process.returncode, self.process.stdout))
                self.dbi_persistence_on.connect_pipe_win()
            else:
                self.process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                                preexec_fn=os.setsid)

            if not is_alive(self.process.pid):
                ERROR("Failed to start the target error code = %d, output = %s" %
                      (self.process.returncode, self.process.stdout))

            INFO(1, None, None, "Target successfully started, waiting for result")

            self.dbi_restart_target = False

        if self.handle_dbi_pre():
            # It means that the target issued quit command or we failed to send command, we should handle it properly
            return 0, ""

        if self.dbi_persistence_mode == 1:
            res = self.handle_dbi_post()
            if res == 1:
                self.handle_return(1) # we use custom timeout of 5 seconds here to check if our target is still alive
                return self.process.returncode, self.err
            elif res == 2: # TODO: it is only for windows, make it consistent
                return EXCEPTION_FIRST_CRITICAL_CODE, "Segmentation fault"
        else:
            ERROR("Persistence mode not yet supported")

        return 0, ""


    def handle_return(self, default_timeout):
        INFO(1, None, None, "Requesting target state")
        if PY3:
            try:
                self.out, self.err = self.process.communicate(timeout=default_timeout)
            except subprocess.TimeoutExpired:
                INFO(1, None, None, "Timeout occured")
                kill_all(self.process.pid)
                return False
        else:
            self.out, self.err = self.process.communicate() # watchdog will handle timeout if needed in PY2
        INFO(1, None, None, "State %s %s" % (self.out, self.err))
        return True


    def exec_command(self, cmd):
        if self.forkserver_on:
            self.returncode = self.exec_command_forkserver(cmd)
            self.err = ""
            return

        if self.dbi_persistence_on:
            INFO(1, None, None, "Persistence mode")
            self.returncode, self.err = self.exec_command_dbi_persistence(cmd)
            return


        if sys.platform == "win32":
            self.process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        else:
            self.process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                            preexec_fn=os.setsid)

        INFO(1, None, None, "Target successfully started, waiting for result")
        self.handle_return(self.timeout)

    def run(self, cmd):

        self.exec_command(cmd)

        if isinstance(self.err, (bytes, bytearray)):
            self.err = self.err.decode("utf-8", 'replace')

        if self.forkserver_on or self.dbi_persistence_on:
            return self.returncode, self.err
        return self.process.returncode, self.err


class Fuzzer:
    def __init__(self, list_of_files, fuzzer_id, virgin_bits_global, args, stats_array, restore_session, crash_bits,
                 dbi_setup, radamsa_path):
        # local fuzzer config
        INFO(1, None, None, "Performing intialization of fuzzer %d" % fuzzer_id)
        global SHM_SIZE, net_sleep_between_cases
        self.SHM_SIZE = SHM_SIZE
        self.CALIBRATIONS_COUNT = 7
        self.SHM_ENV_VAR = "__AFL_SHM_ID"

        self.deterministic = args.deterministic_seed
        if self.deterministic:
            random.seed(a=self.fuzzer_id)

        self.dbi = args.dbi
        self.afl_fuzzer = dict()
        self.radamsa_path = radamsa_path
        if "linux" in sys.platform and "radamsa:0" not in args.mutator_weights:
            self.radamsa_fuzzer = radamsa.RadamsaFuzzer(RAND(MAX_SEED))
            self.radamsa_fuzzer.load_library(self.radamsa_path)
        else:
            self.radamsa_fuzzer = None

        self.token_dict = list()
        self.timeout = args.timeout
        self.disable_volatile_bytes = args.disable_volatile_bytes
        net_sleep_between_cases = float(args.net_sleep_between_cases)

        self.user_mutators = dict()
        self.mutator_weights = OrderedDict()
        total_weights = 0
        try:
            weights = args.mutator_weights.split(",")
            for weight in weights:
                name, weight = weight.split(":")
                total_weights += int(weight)
                self.mutator_weights[name] = total_weights
        except:
            ERROR("Invalid format for mutator_weights string, check manul.config file")

        if total_weights != 10:
            ERROR("Weights in mutator_weights should have 10 in sum, check manul.config file")

        try:
            if args.dict:
                fd = open(args.dict, 'r')
                content = fd.readlines()
                fd.close()
                for line in content:
                    line = line.replace("\n", "")
                    if line.startswith("#") or line == "":
                        continue
                    line = bytearray(line, "utf-8")
                    self.token_dict.append(line)
        except:
            WARNING(None, "Failed to parse dictionary file, dictionary is in invalid format or not accessible")

        self.current_file_name = None
        self.prev_hashes = dict()  # used to store hash of coverage bitmap for each file
        for file_name in list_of_files:
            self.prev_hashes[file_name] = None

        self.cmd_fuzzing = args.cmd_fuzzing

        if args.user_signals:
            self.user_defined_signals = args.user_signals.split(",")
        else:
            self.user_defined_signals = None

        self.dbi_pipe_handler = None
        if dbi_setup:
            self.dbi_engine_path = dbi_setup[0]
            self.dbi_tool_path = dbi_setup[1]
            self.dbi_tool_params = dbi_setup[2]
            if args.dbi_persistence_mode >= 1:
                INFO(1, None, None, "Getting PIPE name for fuzzer %d" % fuzzer_id)
                self.dbi_pipe_handler = dbi_mode.IPCObjectHandler(self.timeout)
                obj_name = self.dbi_pipe_handler.get_ipc_obj_name()
                INFO(1, None, None, "IPC object name in %s" % (obj_name))
                self.dbi_tool_params += "-ipc_obj_name %s" % (obj_name)


        self.target_ip = None
        self.target_port = None
        self.target_protocol = None
        if args.target_ip_port:
            self.target_ip = args.target_ip_port.split(':')[0]
            self.target_port = args.target_ip_port.split(':')[1]
            self.target_protocol = args.target_protocol

        self.list_of_files = list_of_files
        self.fuzzer_id = fuzzer_id
        self.virgin_bits = list()
        self.virgin_bits = [0xFF] * SHM_SIZE

        self.global_map = virgin_bits_global
        self.crash_bits = crash_bits  # happens not too often
        self.bitmap_size = 0
        self.avg_bitmap_size = 0
        self.avg_exec_per_sec = 0

        self.stats_array = stats_array
        self.restore = restore_session

        # creating output dir structure
        self.output_path = args.output + "/%d" % fuzzer_id
        self.queue_path = self.output_path + "/queue"
        if not args.custom_path:
            self.mutate_file_path = self.output_path + "/mutations"
        else:
            self.mutate_file_path = args.custom_path

        self.crashes_path = self.output_path + "/crashes"
        self.unique_crashes_path = self.crashes_path + "/unique"

        self.enable_logging = args.logging_enable
        self.log_file = None

        self.user_sync_freq = args.sync_freq
        self.sync_bitmap_freq = -1

        if not self.restore:
            try:
                os.mkdir(self.output_path)
            except:
                ERROR("Failed to create required output dir structure (unique dir)")
            try:
                os.mkdir(self.queue_path)
            except:
                ERROR("Failed to create required output dir structure (queue)")
            try:
                os.mkdir(self.crashes_path)
            except:
                ERROR("Failed to create required output dir structure (crashes)")
            try:
                os.mkdir(self.unique_crashes_path)
            except:
                ERROR("Failed to create required output dir structure (unique crashes)")

            if not args.custom_path:
                try:
                    os.mkdir(self.mutate_file_path)
                except:
                    ERROR("Failed to create output directory for mutated files")

        self.is_dumb_mode = args.simple_mode
        self.input_path = args.input
        self.target_binary_path = args.target_binary  # and its arguments

        self.fuzzer_stats = FuzzerStats()
        self.stats_file = None
        self.disable_save_stats = args.no_stats

        if not self.is_dumb_mode:
            self.trace_bits = self.setup_shm()

            for i in range(0, self.SHM_SIZE):
                if self.virgin_bits[i] != 0xFF:
                    self.global_map[i] = self.virgin_bits[i]
                elif self.global_map[i] != 0xFF and self.virgin_bits[i] == 0xFF:
                    self.virgin_bits[i] = self.global_map[i]

        if self.restore:
            if not isfile(self.output_path + "/fuzzer_stats"):
                ERROR("Fuzzer stats file doesn't exist. Make sure your output is actual working dir of manul")

            self.stats_file = open(self.output_path + "/fuzzer_stats", 'r')
            content = self.stats_file.readlines()
            line = None
            for line in content:  # getting last line from file to restore session
                pass
            if line is None:
                ERROR("Failed to restore fuzzer %d from stats. Invalid fuzzer_stats format" % self.fuzzer_id)

            last = line[:-2]  # skipping last symbol space and \n
            INFO(0, None, None, "Restoring last stats %s" % last)
            self.stats_file.close()

            bitmap = None
            if not self.is_dumb_mode:
                self.bitmap_file = open(self.output_path + "/fuzzer_bitmap", "rb")
                bitmap = self.bitmap_file.read()
                self.bitmap_file.close()

            self.restore_session(last, bitmap)

        if not self.disable_save_stats:
            self.stats_file = open(self.output_path + "/fuzzer_stats", 'a+')
            self.bitmap_file = open(self.output_path + "/fuzzer_bitmap", 'wb')

        if self.enable_logging:
            self.log_file = open(self.output_path + "/fuzzer_log", 'a')

        self.init_mutators()

        self.net_cmd = False
        if self.target_ip:
            self.net_cmd = self.prepare_cmd_to_run(None, True)

        self.forkserver_on = args.forkserver_on
        INFO(1, None, None, "Initalization is done for %d" % fuzzer_id)
        self.command = Command(self.target_ip, self.target_port, self.target_protocol, self.timeout, args.forkserver_on,
                               self.dbi_pipe_handler, args.dbi_persistence_mode)


    def sync_bitmap(self):
        self.sync_bitmap_freq += 1
        if (self.sync_bitmap_freq % self.user_sync_freq) != 0:
            return
        if self.is_dumb_mode:
            return
        for i in range(0, self.SHM_SIZE):
            if self.virgin_bits[i] != 0xFF:
                self.global_map[i] = self.virgin_bits[i]
            elif self.global_map[i] != 0xFF and self.virgin_bits[i] == 0xFF:
                self.virgin_bits[i] = self.global_map[i]


    def restore_session(self, last, bitmap):
        # parse previously saved stats line
        last = last.split(" ")[1:]  # cut timestamp
        for index, stat in enumerate(last):
            stat = float(stat.split(":")[1])  # taking actual value
            if PY3:
                stat_name = list(self.fuzzer_stats.stats.items())[index][0]
            else:
                stat_name = self.fuzzer_stats.stats.items()[index][0]
            self.fuzzer_stats.stats[stat_name] = stat

        if bitmap:
            # restoring and synchronizing bitmap
            '''for i in range(0, SHM_SIZE):
                self.virgin_bits[i] = bitmap[i]
                self.sync_bitmap_freq = self.user_sync_freq # little trick to enable synchronization
                self.sync_bitmap()
                self.sync_bitmap_freq = 0'''

            # restoring queue
            final_list_of_files = list()
            new_files = [f for f in os.listdir(self.queue_path) if os.path.isfile(os.path.join(self.queue_path, f))]
            for file_name in new_files:
                final_list_of_files.append((1, file_name))  # this is how we add new files

            self.list_of_files = self.list_of_files + final_list_of_files

        if self.deterministic:  # skip already seen seeds
            for i in range(0, self.fuzzer_stats.stats['executions']):
                random.seed(seed=self.fuzzer_id)


    def save_stats(self):
        if self.stats_file is None:
            return

        self.stats_file.write(str(time.time()) + " ")
        for index, (k,v) in enumerate(self.fuzzer_stats.stats.items()):
            self.stats_file.write("%d:%.2f " % (index, v))
        self.stats_file.write("\n")
        self.stats_file.flush()

        # saving AFL state
        for file_name in self.list_of_files:
            if not isinstance(file_name, string_types) : file_name = file_name[1]
            self.afl_fuzzer[file_name].save_state(self.output_path)


    def prepare_cmd_to_run(self, target_file_path, is_net):
        if self.dbi:
            dbi_tool_opt = "-c"
            if self.dbi == "pin":
                dbi_tool_opt = "-t"

            binary_path = "".join(self.target_binary_path)
            if self.cmd_fuzzing:
                target_file_path = extract_content(target_file_path)  # now it is the file content

            if not is_net:
                binary_path = binary_path.replace("@@", target_file_path)

            final_string = "%s %s %s %s -- %s" % (self.dbi_engine_path, dbi_tool_opt, self.dbi_tool_path,
                                                  self.dbi_tool_params, binary_path)
        else:
            final_string = "".join(self.target_binary_path)
            if self.cmd_fuzzing:
                target_file_path = extract_content(target_file_path)  # now it is the file content

            if not is_net:
                final_string = final_string.replace("@@", target_file_path)

        return final_string


    def setup_shm_win(self):
        from ctypes.wintypes import DWORD, HANDLE, LPCWSTR, LPVOID
        FILE_MAP_ALL_ACCESS = 0xF001F
        PAGE_READWRITE = 0x04
        sh_name = "%s_%s" % (str(int(round(time.time()))), self.fuzzer_id)
        szName = c_wchar_p(sh_name)

        kernel32_dll = windll.kernel32

        create_file_mapping_func = kernel32_dll.CreateFileMappingW
        create_file_mapping_func.argtypes = (HANDLE, LPVOID, DWORD, DWORD, DWORD, LPCWSTR)
        create_file_mapping_func.restype = HANDLE
        map_view_of_file_func = kernel32_dll.MapViewOfFile
        map_view_of_file_func.restype = LPVOID

        hMapObject = create_file_mapping_func(-1, None,
                                              PAGE_READWRITE, 0, self.SHM_SIZE,
                                              szName)
        if not hMapObject or hMapObject == 0:
            ERROR("Could not open file mapping object, GetLastError = %d" % GetLastError())

        pBuf = map_view_of_file_func(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0,
                                             self.SHM_SIZE)
        if not pBuf or pBuf == 0:
            ERROR("Could not map view of file, GetLastError = %d" % GetLastError())

        INFO(0, None, self.log_file, "Setting up shared mem %s for fuzzer:%d" % (sh_name,
             self.fuzzer_id))
        os.environ[self.SHM_ENV_VAR] = sh_name

        return pBuf


    def setup_shm(self):
        if sys.platform == "win32":
            return self.setup_shm_win()

        IPC_PRIVATE = 0

        try:
            rt = CDLL('librt.so')
        except:
            rt = CDLL('librt.so.1')

        shmget = rt.shmget
        shmget.argtypes = [c_int, c_size_t, c_int]
        shmget.restype = c_int
        shmat = rt.shmat
        shmat.argtypes = [c_int, POINTER(c_void_p), c_int]
        shmat.restype = c_void_p#POINTER(c_byte * self.SHM_SIZE)

        shmid = shmget(IPC_PRIVATE, self.SHM_SIZE, 0o666)
        if shmid < 0:
            ERROR("shmget() failed")

        addr = shmat(shmid, None, 0)

        INFO(0, None, self.log_file, "Setting up shared mem %d for fuzzer:%d" % (shmid, self.fuzzer_id))
        os.environ[self.SHM_ENV_VAR] = str(shmid)

        return addr


    def init_mutators(self):
        INFO(0, bcolors.BOLD + bcolors.HEADER, self.log_file, "Initializing mutators")

        for module_name in self.mutator_weights:
            if "afl" == module_name or "radamsa" == module_name:
                continue
            try:
                self.user_mutators[module_name] = importlib.import_module(module_name)
            except ImportError as exc:
                ERROR("Unable to load user provided mutator %s. %s" % (module_name, exc.message))

            self.user_mutators[module_name].init()

        # init AFL fuzzer state
        for file_name in self.list_of_files:
            if not isinstance(file_name, string_types): file_name = file_name[1]
            self.afl_fuzzer[file_name] = afl_fuzz.AFLFuzzer(self.token_dict, self.queue_path, file_name)  #assign AFL for each file
            if self.restore:
                self.afl_fuzzer[file_name].restore_state(self.output_path)


    def dry_run(self):

        INFO(0, bcolors.BOLD + bcolors.HEADER, self.log_file, "Performing dry run")

        useless = 0

        for file_name in self.list_of_files:
            # if we have tuple and not string here it means that this file was found during execution and located in queue
            self.current_file_name = file_name
            if not isinstance(file_name, string_types):
                file_name = file_name[1]
                full_input_file_path = self.queue_path + "/" + file_name
            else:
                full_input_file_path = self.input_path + "/" + file_name

            shutil.copy(full_input_file_path, self.mutate_file_path + "/.cur_input")
            full_input_file_path = self.mutate_file_path + "/.cur_input"

            memset(self.trace_bits, 0x0, SHM_SIZE)

            if self.target_ip:
                err_code, err_output = self.command.net_send_data_to_target(extract_content(full_input_file_path), self.net_cmd)
            else:
                cmd = self.prepare_cmd_to_run(full_input_file_path, False)
                INFO(1, bcolors.BOLD, self.log_file, "Launching %s" % cmd)
                err_code, err_output = self.command.run(cmd)

            if err_code and err_code != 0:
                INFO(1, None, self.log_file, "Initial input file: %s triggers an exception in the target" % file_name)
                if self.is_critical(err_output, err_code):
                    WARNING(self.log_file, "Initial input %s leads target to crash (did you disable leak sanitizer?). "
                                           "Enable --debug to check actual output" % file_name)
                    INFO(1, None, self.log_file, err_output)
                elif self.is_problem_with_config(err_code, err_output):
                    WARNING(self.log_file, "Problematic file %s" % file_name)

            trace_bits_as_str = string_at(self.trace_bits, SHM_SIZE)

            # count non-zero bytes just to check that instrumentation actually works
            non_zeros = [x for x in trace_bits_as_str if x != 0x0]
            if len(non_zeros) == 0:
                INFO(1, None, self.log_file, "Output from target %s" % err_output)
                if "is for the wrong architecture" in err_output:
                    ERROR("You should run 32-bit drrun for 32-bit targets and 64-bit drrun for 64-bit targets")
                ERROR("%s doesn't cover any path in the target, Make sure the binary is actually instrumented" % file_name)

            ret = self.has_new_bits(trace_bits_as_str, True, list(), self.virgin_bits, False, full_input_file_path)
            if ret == 0:
                useless += 1
                WARNING(self.log_file, "Test %s might be useless because it doesn't cover new paths in the target, consider removing it" % file_name)
            else:
                self.sync_bitmap()

        if useless != 0:
            WARNING(self.log_file, "%d out of %d initial files are useless" % (useless, len(self.list_of_files)))

        INFO(0, bcolors.BOLD + bcolors.OKBLUE, self.log_file, "Dry run finished")
        self.fuzzer_stats.stats['executions'] += 1.0
        self.update_stats()


    def has_new_bits(self, trace_bits_as_str, update_virgin_bits, volatile_bytes, bitmap_to_compare, calibration, full_input_file_path):

        ret = 0

        #print_bitmaps(bitmap_to_compare, trace_bits_as_str, full_input_file_path)

        if not calibration:
            hash_current = zlib.crc32(trace_bits_as_str) & 0xFFFFFFFF

            if not isinstance(self.current_file_name, string_types):
                self.current_file_name = self.current_file_name[1]

            prev_hash = self.prev_hashes.get(self.current_file_name, None)

            if prev_hash and hash_current == prev_hash:
                return 0
            self.prev_hashes[self.current_file_name] = hash_current

        for j in range(0, SHM_SIZE):
            if j in volatile_bytes:
                continue  # ignoring volatile bytes

            if PY3:
                trace_byte = trace_bits_as_str[j]  # optimize it and compare by 4-8 bytes or even use xmm0?
            else:
                trace_byte = ord(trace_bits_as_str[j]) # self.trace_bits.contents[j])#

            if not trace_byte:
                continue

            virgin_byte = bitmap_to_compare[j]

            if trace_byte and (trace_byte & virgin_byte):
                if ret < 2:
                    if virgin_byte == 0xff:
                        ret = 2  # new path discovered
                        if update_virgin_bits:
                            self.bitmap_size += 1
                    else:
                        ret = 1  # new hit of existent paths

                virgin_byte = virgin_byte & ~trace_byte

                if update_virgin_bits:
                    bitmap_to_compare[j] = virgin_byte  # python will handle potential synchronization issues

        return ret

    def calibrate_test_case(self, full_file_path):
        volatile_bytes = list()
        trace_bits_as_str = string_at(self.trace_bits, self.SHM_SIZE)  # this is how we read memory in Python

        bitmap_to_compare = list("\x00" * self.SHM_SIZE)
        for i in range(0, self.SHM_SIZE):
            if PY3:
                bitmap_to_compare[i] = trace_bits_as_str[i]
            else:
                bitmap_to_compare[i] = ord(trace_bits_as_str[i])

        cmd, data = None, None
        if self.target_ip:  # in net mode we only need data
            data = extract_content(full_file_path)
        else:
            cmd = self.prepare_cmd_to_run(full_file_path, False)

        for i in range(0, self.CALIBRATIONS_COUNT):
            INFO(1, None, self.log_file, "Calibrating %s %d" % (full_file_path, i))

            memset(self.trace_bits, 0x0, SHM_SIZE)
            if self.target_ip:  # in net mode we only need data
                err_code, err_output = self.command.net_send_data_to_target(data, self.net_cmd)
            else:
                INFO(1, None, self.log_file, cmd)

                if self.cmd_fuzzing:
                    try:
                        err_code, err_output = self.command.run(cmd)
                    except TypeError:
                        WARNING("Failed to send this input over command line into the target")
                        continue
                else:
                    err_code, err_output = self.command.run(cmd)

            if err_code and err_code > 0:
                INFO(1, None, self.log_file, "Target raised exception during calibration for %s" % full_file_path)

            trace_bits_as_str = string_at(self.trace_bits, SHM_SIZE)  # this is how we read memory in Python

            if not self.disable_volatile_bytes:
                for j in range(0, SHM_SIZE):
                    if PY3:
                        trace_byte = trace_bits_as_str[j]
                    else:
                        trace_byte = ord(trace_bits_as_str[j])
                    if trace_byte != bitmap_to_compare[j]:
                        if j not in volatile_bytes:
                            volatile_bytes.append(j)  # mark offset of this byte as volatile

                INFO(1, None, self.log_file, "We have %d volatile bytes for this new finding" % len(volatile_bytes))

        # let's try to check for new coverage ignoring volatile bytes
        self.fuzzer_stats.stats['blacklisted_paths'] = len(volatile_bytes)

        return self.has_new_bits(trace_bits_as_str, True, volatile_bytes, self.virgin_bits, True, full_file_path)

    def update_stats(self):
        for i, (k,v) in enumerate(self.fuzzer_stats.stats.items()):
            self.stats_array[i] = v

    def is_problem_with_config(self, exc_code, err_output):
        if exc_code == 127 or exc_code == 126:  # command not found or permissions
            ERROR("Thread %d unable to execute target. Bash return %s" % (self.fuzzer_id, err_output))
        elif exc_code == 124:  # timeout
            WARNING(self.log_file, "Target failed to finish execution within given timeout, try to increase default timeout")
            return True
        return False

    def generate_new_name(self, file_name):
        iteration = int(round(self.fuzzer_stats.stats['executions']))
        if file_name.startswith("manul"): # manul-DateTime-FuzzerId-iteration_original.name
            base_name = file_name[file_name.find("_")+1:]
            file_name = base_name

        now = int(round(time.time()))
        return "manul-%d-%d-%d_%s" % (now, self.fuzzer_id, iteration, file_name)


    def is_critical_win(self, exception_code):

        if exception_code == STATUS_CONTROL_C_EXIT:
            return False

        if exception_code >= EXCEPTION_FIRST_CRITICAL_CODE and exception_code < EXCEPTION_LAST_CRITICAL_CODE:
            return True

        return False


    def is_critical_mac(self, exception_code):
        if exception_code in critical_signals_nix:
            return True

        return False

    def is_critifcal_linux(self, exception_code):
        if exception_code in critical_signals_nix:
            return True
        if self.forkserver_on and os.WIFSIGNALED(exception_code):
            return True

        return False

    def is_critical(self, err_str, err_code):

        if err_str and "Sanitizer" in err_str or "SIGSEGV" in err_str or "Segmentation fault" in err_str or \
           "core dumped" in err_str or "floating point exception" in err_str:
            return True

        if self.user_defined_signals and err_code in self.user_defined_signals:
            return True

        if sys.platform == "win32":
            return self.is_critical_win(err_code)
        elif sys.platform == "darwin":
            return self.is_critical_mac(err_code)
        else:  # looks like Linux
            return self.is_critifcal_linux(err_code)

    def mutate_radamsa(self, full_input_file_path, full_output_file_path):
        if "linux" in sys.platform: # on Linux we just use a shared library to speed up test cases generation
            data = extract_content(full_input_file_path)
            data_new = self.radamsa_fuzzer.radamsa_generate_output(bytes(data))
            save_content(data_new, full_output_file_path)
            return 0

        new_seed_str = ""
        if self.deterministic:
            new_seed = random.randint(0, sys.maxsize)
            new_seed_str = "--seed %d " % new_seed

        cmd = "%s %s%s > %s" % (self.radamsa_path, new_seed_str, full_input_file_path, full_output_file_path)

        INFO(1, None, self.log_file, "Running %s" % cmd)
        try:
            subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)  # generate new input
        except subprocess.CalledProcessError as exc:
            WARNING(self.log_file,
                    "Fuzzer %d failed to generate new input from %s due to some problem with radamsa. Error code %d. Return msg %s" %
                    (self.fuzzer_id, full_input_file_path, exc.returncode, exc.output))
            return 1
        return 0

    def mutate_afl(self, file_name, full_input_file_path, full_output_file_path):
        data = extract_content(full_input_file_path)
        res = self.afl_fuzzer[file_name].mutate(data, self.list_of_files,
                                                self.fuzzer_stats.stats['exec_per_sec'],
                                                self.avg_exec_per_sec, self.bitmap_size,
                                                self.avg_bitmap_size, 0) # TODO: handicap
        if not res:
            WARNING(self.log_file, "Unable to mutate data provided using afl")
            return 1
        if len(data) <= 0:
            WARNING(self.log_file, "AFL produced empty file for %s", full_input_file_path)

        save_content(data, full_output_file_path)
        return 0

    def mutate_input(self, file_name, full_input_file_path, full_output_file_path):
        execution = self.fuzzer_stats.stats['executions'] % 10
        for name in self.mutator_weights:
            weight = self.mutator_weights[name]
            if execution < weight and name == "afl":
                return self.mutate_afl(file_name, full_input_file_path, full_output_file_path)
            elif execution < weight and name == "radamsa":
                return self.mutate_radamsa(full_input_file_path, full_output_file_path)
            elif execution < weight:
                mutator = self.user_mutators.get(name, None)
                if not mutator:
                    ERROR("Unable to load user provided mutator %s at mutate_input stage" % name)
                data = extract_content(full_input_file_path)
                data = mutator.mutate(data)
                if not data:
                    ERROR("No data returned from user provided mutator. Exciting.")
                save_content(data, full_output_file_path)
                return 0
            else:
                continue

    def run(self):
        if not self.is_dumb_mode:
            self.dry_run()

        last_stats_saved_time = 0

        if self.restore:
            INFO(0, bcolors.BOLD + bcolors.OKBLUE, self.log_file, "Session successfully restored")

        start_time = timer()
        cycle_id = 0

        while True:  # never return
            new_files = list()  # empty the list
            elapsed = 0
            cycle_id += 1

            for i, file_name in enumerate(self.list_of_files):
                self.current_file_name = file_name
                crash_found = False
                self.fuzzer_stats.stats['file_running'] = i

                full_input_file_path = self.input_path + "/"
                # if we have tuple and not string here it means that this file was found during execution and located in queue
                if not isinstance(file_name, string_types):
                    file_name = file_name[1]
                    full_input_file_path = self.queue_path + "/"
                full_input_file_path += file_name

                if not self.is_dumb_mode:
                    memset(self.trace_bits, 0x0, SHM_SIZE) # preparing our bitmap for new run

                mutated_name = ".cur_input"
                full_output_file_path = self.mutate_file_path + "/" + mutated_name

                # command to generate new input using one of selected mutators
                res = self.mutate_input(file_name, full_input_file_path, full_output_file_path)

                if res != 0:
                    ERROR("Fuzzer %d failed to generate and save new input on disk" % self.fuzzer_id)

                timer_start = timer()

                if self.target_ip:
                    data = extract_content(full_output_file_path)
                    exc_code, err_output = self.command.net_send_data_to_target(data, self.net_cmd)
                else:
                    cmd = self.prepare_cmd_to_run(full_output_file_path, False)
                    first_iteration = False
                    INFO(1, None, self.log_file, "Running %s" % cmd)

                    if self.cmd_fuzzing:
                        try:
                            exc_code, err_output = self.command.run(cmd)
                        except TypeError:
                            WARNING("Failed to give this input from bash into the target")
                            continue
                    else:
                        exc_code, err_output = self.command.run(cmd)

                self.fuzzer_stats.stats['executions'] += 1.0
                elapsed += (timer() - timer_start)

                if exc_code and exc_code != 0:
                    self.fuzzer_stats.stats['exceptions'] += 1
                    INFO(1, None, self.log_file, "Target raised exception and returns 0x%x error code" % (exc_code))

                    if self.is_critical(err_output, exc_code):
                        INFO(0, bcolors.BOLD + bcolors.OKGREEN, self.log_file, "New crash found by fuzzer %d" % self.fuzzer_id)
                        self.fuzzer_stats.stats["last_crash_time"] = time.time()

                        new_name = self.generate_new_name(file_name)
                        shutil.copy(full_output_file_path, self.crashes_path + "/" + new_name)  # copying into crash folder
                        self.fuzzer_stats.stats['crashes'] += 1

                        if not self.is_dumb_mode:
                            trace_bits_as_str = string_at(self.trace_bits, SHM_SIZE)  # this is how we read memory in Python
                            ret = self.has_new_bits(trace_bits_as_str, True, list(), self.crash_bits, False, full_output_file_path)
                            if ret == 2:
                                INFO(0, bcolors.BOLD + bcolors.OKGREEN, self.log_file, "Crash is unique")
                                self.fuzzer_stats.stats['unique_crashes'] += 1
                                shutil.copy(full_output_file_path, self.unique_crashes_path + "/" + new_name)  # copying into crash folder with unique crashes

                        crash_found = True

                    elif self.is_problem_with_config(exc_code, err_output):
                        WARNING(self.log_file, "Problematic file: %s" % file_name)

                if not crash_found and not self.is_dumb_mode:
                    # Reading the coverage

                    trace_bits_as_str = string_at(self.trace_bits, SHM_SIZE)  # this is how we read memory in Python
                    # we are not ready to update coverage at this stage due to volatile bytes
                    ret = self.has_new_bits(trace_bits_as_str, False, list(), self.virgin_bits, False, full_output_file_path)
                    if ret == 2:
                        INFO(1, None, self.log_file, "Input %s produces new coverage, calibrating" % file_name)
                        if self.calibrate_test_case(full_output_file_path) == 2:
                            self.fuzzer_stats.stats['new_paths'] += 1
                            self.fuzzer_stats.stats['last_path_time'] = time.time()
                            INFO(1, None, self.log_file, "Calibration finished successfully. Saving new finding")

                            new_coverage_file_name = self.generate_new_name(file_name)
                            INFO(1, None, self.log_file, "Copying %s to %s" % (full_output_file_path,
                                 self.queue_path + "/" + new_coverage_file_name))

                            shutil.copy(full_output_file_path, self.queue_path + "/" + new_coverage_file_name)

                            new_files.append((1, new_coverage_file_name))
                            # for each new file assign new AFLFuzzer
                            self.afl_fuzzer[new_coverage_file_name] = afl_fuzz.AFLFuzzer(self.token_dict, self.queue_path,
                                                                                         new_coverage_file_name)
                            self.prev_hashes[new_coverage_file_name] = None

                self.update_stats()

            self.sync_bitmap()

            if len(new_files) > 0:
                self.list_of_files = self.list_of_files + new_files

            self.fuzzer_stats.stats['files_in_queue'] = len(self.list_of_files)

            self.update_stats()

            end_time = timer() - start_time
            self.fuzzer_stats.stats['exec_per_sec'] = self.fuzzer_stats.stats['executions'] / end_time
            self.avg_exec_per_sec += int(self.fuzzer_stats.stats['exec_per_sec'] / cycle_id)
            self.avg_bitmap_size += int(self.bitmap_size / cycle_id)

            last_stats_saved_time += elapsed
            if last_stats_saved_time > 1:  # we save fuzzer stats per iteration or once per second to avoid huge stats files
                self.save_stats()
                last_stats_saved_time = 0


def get_bytes_covered(virgin_bits):
    non_zeros = [x for x in virgin_bits if x != 0xFF]
    return len(non_zeros)


def run_fuzzer_instance(files_list, i, virgin_bits, args, stats_array, restore_session,
                        crash_bits, dbi_setup, radamsa_path):
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    printing.DEBUG_PRINT = args.debug  # FYI, multiprocessing causes global vars to be reinitialized.
    INFO(0, None, None, "Starting fuzzer %d" % i)

    fuzzer_instance = Fuzzer(files_list, i, virgin_bits, args, stats_array, restore_session,
                             crash_bits, dbi_setup, radamsa_path)
    fuzzer_instance.run()  # never return


def check_instrumentation(target_binary):
    with open(target_binary, 'rb') as f:
        s = f.read()
        res = s.find(b"__AFL_SHM_ID")
        if res == -1:
            return False
    return True


def which(target_binary):
    def is_binary(target_binary):
        return os.path.isfile(target_binary) and os.access(target_binary, os.X_OK)

    fpath, fname = os.path.split(target_binary)
    if fpath:
        if is_binary(target_binary):
            return target_binary
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            exec_file = os.path.join(path, target_binary)
            if is_binary(exec_file):
                return exec_file

    return None


def check_binary(target_binary):
    binary_path = which(target_binary)
    if binary_path is None:
        ERROR("Unable to find binary %s (required)" % target_binary)


def get_available_id_for_backup(dir_name):
    id = 0
    tmp = dir_name + "_%d" % id
    while True:
        if not os.path.exists(tmp):
            return id
        id += 1
        tmp = dir_name + "_%d" % id


def configure_dbi(args, target_binary, is_debug):
    dbi_engine_path = args.dbi_root
    dbi_tool_path = args.dbi_client_root
    dbi_tool_libs = args.dbi_client_libs

    if dbi_engine_path is None or dbi_tool_path is None:
        ERROR("DBI_ROOT and/or DBI_CLIENT_ROOT paths not specified, unable to execute manul")

    check_binary(dbi_engine_path)
    check_binary(dbi_tool_path)

    dbi_tool_params = ""
    dbi_pipe_handler = None
    if args.dbi == "dynamorio":
        if args.dbi_persistence_mode >= 1:
            if args.dbi_target_module:
                dbi_tool_params += "-target_module %s " % args.dbi_target_module

            if args.dbi_thread_coverage:
                dbi_tool_params += "-thread_coverage"

            if args.dbi_target_method:
                dbi_tool_params += "-target_method %s " % args.dbi_target_method
            elif args.dbi_target_offset:
                dbi_tool_params += "-target_offset %s " % args.dbi_target_offset
            else:
                ERROR("Please specify target method or target offset in manul.config")

            dbi_tool_params += "-fuzz_iterations %d " % args.dbi_fuzz_iterations
            dbi_tool_params += "-persistence_mode %d " % args.dbi_persistence_mode

        dbi_tool_params += "-coverage_module %s " % ntpath.basename(target_binary)

        if dbi_tool_libs is not None:
            for target_lib in dbi_tool_libs.split(","):
                if target_lib == "":
                    continue
                dbi_tool_params += "-coverage_module %s " % target_lib
        if is_debug:
            dbi_tool_params += "-debug_manul "
    elif args.dbi == "pin":
        if sys.platform == "win32":
            ERROR("Intel PIN DBI engine is not supported on Windows")
        if dbi_tool_libs is not None:
            # adding desired libs to instrument
            fd = open("dbi_config", 'w')
            fd.write(dbi_tool_libs)
            fd.close()
            dbi_config_file_path = os.path.abspath("dbi_config")
            dbi_tool_params += " -libs %s" % dbi_config_file_path
    else:
        ERROR("Unknown dbi engine/option specified. Intel PIN or DynamoRIO are only supported")

    dbi_setup = (dbi_engine_path, dbi_tool_path, dbi_tool_params, dbi_pipe_handler)
    return dbi_setup

def split_files_by_count(files_list, threads_count):
    # split list of input files by fuzzer instances
    less_files = False
    if len(files_list) < threads_count:
        less_files = True
        WARNING(None, "Too many fuzzing instances for %d files, same files will be mutated with different seeds" % len(files_list))

    files = [[] for x in xrange(threads_count)]
    thread_index = 0
    # if list of files is less than number of required threads we run our fuzzer with different seed on the same files
    while thread_index < threads_count:
        for i, file_name in enumerate(files_list):
            if less_files and (thread_index + i) >= threads_count:
                break
            piece = (thread_index + i) % threads_count
            files[piece].append(file_name)
            thread_index += i + 1
    return files


def get_files_list(path):
    files_list = [f for f in listdir(path) if isfile(join(path, f))]  # let's process input directory
    files_list.sort()

    if len(files_list) == 0:
        ERROR("No files for fuzzing, exiting")

    return files_list


def check_if_exist(files_list, path):
    for file_name in files_list:
        if file_name == "":
            ERROR("File list has empty file name")
        elif isfile(path + "/" + file_name):
            continue
        else:
            ERROR("File %s doesn't exist in %s" % (file_name, path))


def allocate_files_per_jobs(args):
    if args.net_config_slave is not None:
        files = manul_network.get_files_list_from_master(args.net_config_slave, args.nfuzzers)  # ask master to provide list of files
        check_if_exist(files, args.input)
        return split_files_by_count(files, args.nfuzzers)

    files_list = get_files_list(args.input)

    if args.net_config_master is not None:

        ips = manul_network.get_slaves_ips(args.net_config_master)
        slaves, total_threads_count = manul_network.get_remote_threads_count(ips)  # ask slaves count and threads
        total_threads_count += args.nfuzzers

        files = split_files_by_count(files_list, total_threads_count)
        piece_id = 0
        for ip, port, slave_threads_count in slaves:
            manul_network.send_files_list(ip, port, files[piece_id:slave_threads_count + piece_id])  # send them files list
            piece_id += slave_threads_count
        files = files[piece_id:]
    else:
        files = split_files_by_count(files_list, args.nfuzzers)

    return files

def enable_network_config(args):

    if (args.target_ip_port and not args.target_protocol) or (args.target_protocol and not args.target_ip_port):
        ERROR("Both target_ip_port and target_protocol should be specified")

    if args.target_ip_port and not args.target_protocol:
        ERROR("You need to provide target protocol (tcp or udp) in manul config along with ip and port")
    if args.target_protocol and not args.target_ip_port:
        ERROR("You need to provide target port and ip along with TCP/IP protocol in manul config")
    if args.target_protocol and args.target_protocol != "tcp" and args.target_protocol != "udp":
        ERROR("Invalid protocol. Should be tcp or udp.")
    if args.target_ip_port and args.nfuzzers > 1:
        ERROR("Multi-threaded network fuzzing is not supported, yet")
    if args.target_ip_port:
        target_ip_port = args.target_ip_port.split(":")
        if len(target_ip_port) != 2:
            ERROR("Invalid format for IP:PORT in manul config, received this: %s" % args.target_ip_port)
        target_ip = target_ip_port[0]
        if target_ip.count(".") != 3:
            ERROR("Invalid IP format in %s" % target_ip)
        target_port = target_ip_port[1]
        if int(target_port) > 65535 or int(target_port) <= 0:
            ERROR("Target port should be in range (0, 65535)")

def parse_args():
    global INIT_WAIT_TIME
    parser = argparse.ArgumentParser(prog = "manul.py",
                                     description = 'Manul - coverage-guided parallel fuzzing for native applications.',
                                     usage = '%(prog)s -i /home/user/inputs_dir -o /home/user/outputs_dir -n 40 "target -png @@"')
    requiredNamed = parser.add_argument_group('Required parameters')
    requiredNamed.add_argument('-i', required=True, dest='input', help = "Path to directory with initial corpus")
    requiredNamed.add_argument('-o', dest='output', required=True, default="manul_output",
                               help = "Path to output directory")


    parser.add_argument('-n', default=1, type=int, dest='nfuzzers', help = "Number of parallel fuzzers")
    parser.add_argument('-s', default=False, action='store_true', dest="simple_mode",
                        help = "Run dumb fuzzing (no code instrumentation)")
    parser.add_argument('-c', default="manul.config", dest = "config",
                        help = "Path to config file with additional options (see manul.config)")
    parser.add_argument('-r', default=False, action='store_true', dest = "restore", help = "Restore previous session")

    # these options should be specified through config file and hidden
    parser.add_argument('--deterministic_seed', default=False, action='store_true', help = argparse.SUPPRESS)
    parser.add_argument('--print_per_thread', default=False, action='store_true', dest="threads_info", help = argparse.SUPPRESS)

    parser.add_argument('--dbi', default = None, help = argparse.SUPPRESS)
    parser.add_argument('--dbi_root', help = argparse.SUPPRESS)
    parser.add_argument('--dbi_client_root', help = argparse.SUPPRESS)
    parser.add_argument('--dbi_client_libs', help = argparse.SUPPRESS)
    parser.add_argument("--dbi_persistence_mode", default = 0, type=int, help = argparse.SUPPRESS)
    parser.add_argument("--dbi_target_method", default = None, help = argparse.SUPPRESS)
    parser.add_argument("--dbi_target_offset", default = None, help= argparse.SUPPRESS)
    parser.add_argument("--dbi_target_module", default = None, help = argparse.SUPPRESS)
    parser.add_argument("--dbi_fuzz_iterations", default = 5000, type=int, help = argparse.SUPPRESS)
    parser.add_argument("--dbi_thread_coverage", default = False, action = 'store_true', help = argparse.SUPPRESS)

    parser.add_argument('--timeout', default=10, type=int, help = argparse.SUPPRESS)
    parser.add_argument('--net_config_master', help = argparse.SUPPRESS)
    parser.add_argument('--net_config_slave', help = argparse.SUPPRESS)
    parser.add_argument('--debug', default=False, action='store_true', help = argparse.SUPPRESS)
    parser.add_argument('--manul_logo', default=False, action='store_true', help = argparse.SUPPRESS)
    parser.add_argument('--logging_enable', default=False, action='store_true', help = argparse.SUPPRESS)
    parser.add_argument('--sync_freq', default=1000000, type=int, help = argparse.SUPPRESS)
    parser.add_argument('--cmd_fuzzing', default=False, action='store_true', help = argparse.SUPPRESS)
    parser.add_argument('--target_ip_port', default = None, help = argparse.SUPPRESS)
    parser.add_argument('--target_protocol', default = None, help = argparse.SUPPRESS)
    parser.add_argument('--mutator_weights', default = None, help = argparse.SUPPRESS)
    parser.add_argument('--user_signals', default = None, help = argparse.SUPPRESS)
    parser.add_argument("--dict", default = None, help = argparse.SUPPRESS)
    parser.add_argument("--restore", default = None, action = 'store_true', help = argparse.SUPPRESS)
    parser.add_argument("--no_stats", default = None, action = "store_true", help = argparse.SUPPRESS)
    parser.add_argument("--custom_path", default = None, help=argparse.SUPPRESS)
    parser.add_argument("--init_wait", default = 0.0, help = argparse.SUPPRESS)
    parser.add_argument("--net_sleep_between_cases", default = 0.0, help = argparse.SUPPRESS)
    parser.add_argument("--disable_volatile_bytes", default = None, action = 'store_true', help = argparse.SUPPRESS)
    parser.add_argument("--stop_after_nseconds", default = 0.0, type=int, help = argparse.SUPPRESS)
    parser.add_argument("--forkserver_on", default = False, action = 'store_true', help = argparse.SUPPRESS)


    parser.add_argument('target_binary', nargs='*', help="The target binary and options to be executed (quotes needed e.g. \"target -png @@\")")

    args = parser.parse_args()
    additional_args = parse_config(args.config)
    # A little hack here. We actually adding commands from config to cmd string and then parse it all together.
    final_cmd_to_parse = "%s %s" % (" ".join(sys.argv[1:-1]), additional_args)

    final_cmd_to_parse = final_cmd_to_parse.split(" ")
    final_cmd_to_parse.append("%s" % sys.argv[-1])

    args = parser.parse_args(final_cmd_to_parse)

    if args.manul_logo:
        printing.print_logo()

    if not args.target_ip_port and "@@" not in args.target_binary[0]:
        ERROR("Your forgot to specify @@ for your target. Call manul.py -h for more details")

    if args.simple_mode and args.dbi is not None:
        ERROR("Options mismatch. Simple mode can't be executed with DBI mode together (check manul.config).")

    if not args.mutator_weights:
        ERROR("At least one mutator should be specified")

    if args.custom_path and not os.path.isdir(args.custom_path):
        ERROR("Custom path provided does not exist or not a directory")

    enable_network_config(args)

    if args.dict:
        if not os.path.isfile(args.dict):
            WARNING(None, "Unable to read dictionary file from %s, file doesn't exist" % args.dict)

    if args.forkserver_on and not sys.platform.startswith('linux'):
        INFO(0, None, None, "Forkserver is not supported on this platform, switching to classic mode")
        args.forkserver_on = False

    if args.simple_mode or args.dbi:
        args.forkserver_on = False # we don't have forkserver for simple or DBI modes

    #TODO: check that DBI params are correctly set

    INIT_WAIT_TIME = float(args.init_wait)

    return args


if __name__ == "__main__":
    start = timer()
    args = parse_args()

    printing.DEBUG_PRINT = args.debug

    binary_to_check = args.target_binary[0]
    target_binary = split_unescape(binary_to_check, ' ', '\\')[0]

    dbi_setup = None
    if args.dbi is not None:
        dbi_setup = configure_dbi(args, target_binary, args.debug)

    check_binary(target_binary)  # check if our binary exists and is actually instrumented

    if not args.simple_mode and args.dbi is None and not check_instrumentation(target_binary):
        ERROR("Failed to find afl's instrumentation in the target binary, try to recompile or run manul in dumb mode")

    if not os.path.isdir(args.input):
        ERROR("Input directory doesn't exist")

    if not os.path.isdir(args.output):
        ERROR("Output directory doesn't exist")

    if args.output.endswith('/'):
        args.output = args.output[:-1]
    if args.input.endswith('/'):
        args.input = args.input[:-1]

    if not args.restore and os.listdir(args.output):
        WARNING(None, "Output directory is not empty, creating backup of output folder")
        id = get_available_id_for_backup(args.output)
        os.rename(args.output, args.output + "_%d" % id)
        os.mkdir(args.output)
        INFO(0, None, None, "Done")

    # if radamsa weight is not zero, check that we can actually execute it
    radamsa_path = None
    if "radamsa:0" not in args.mutator_weights:
        #get relative path to radamsa binary
        radamsa_path = __file__
        radamsa_path = radamsa_path.replace("manul.py", "")
        if sys.platform == "win32":
            radamsa_path = radamsa_path + "radamsa.exe"
        else:
            radamsa_path = radamsa_path + "./libradamsa/libradamsa.so"
        INFO(1, None, None, "Full relative path to radamsa %s" % radamsa_path)
        check_binary(radamsa_path)

    files = allocate_files_per_jobs(args)

    virgin_bits = None
    crash_bits = None
    if not args.simple_mode:
        virgin_bits = multiprocessing.Array("i", SHM_SIZE)
        crash_bits = multiprocessing.Array("i", SHM_SIZE)
        for i in range(0, SHM_SIZE):
            virgin_bits[i] = 255  # initializing with all 0xFFs
            crash_bits[i] = 255

    # allocating data structures where we store all statistics about our fuzzers
    stats = FuzzerStats()
    all_threads_stats = list()
    all_threads_handles = list()

    for i, files_piece in enumerate(files):
        stats_array = multiprocessing.Array("d", stats.get_len())
        t = multiprocessing.Process(target=run_fuzzer_instance, args=(files_piece, i, virgin_bits, args, stats_array,
                                                                      args.restore, crash_bits, dbi_setup, radamsa_path))
        t.start()
        all_threads_stats.append(stats_array)
        all_threads_handles.append(t)

    INFO(0, None, None, "%d fuzzer instances successfully launched" % args.nfuzzers)

    sync_t = None
    if (args.net_config_slave is not None or args.net_config_master is not None) and not args.simple_mode:
        INFO(1, None, None, "Allocating special thread for bitmap synchronization")
        ips = None
        if args.net_config_master is not None:
            ips = manul_network.get_slaves_ips(args.net_config_master)
            sync_t = threading.Thread(target=manul_network.sync_remote_bitmaps,
                                      args=(virgin_bits, ips))
        elif args.net_config_slave is not None:
            sync_t = threading.Thread(target=manul_network.receive_bitmap_slave,
                                      args=(args.net_config_slave, virgin_bits))
        sync_t.setDaemon(True)
        sync_t.start()

    if not PY3 and not args.target_ip_port and not args.forkserver_on:
        watchdog_t = threading.Thread(target=watchdog, args=(args.timeout,))
        watchdog_t.setDaemon(True)
        watchdog_t.start()

    try:
        while True:
            threads_inactive = 0
            for i, t in enumerate(all_threads_handles):
                if not t.is_alive():
                    threads_inactive += 1
                    WARNING(None, "Fuzzer %d unexpectedly terminated" % i)

            if sync_t is not None and not sync_t.alive():
                WARNING(None, "Synchronization thread is not alive")

            end = timer() - start

            bytes_cov = 0.0
            if not args.simple_mode:
                bytes_cov = get_bytes_covered(virgin_bits)
            active_threads_count = len(all_threads_handles) - threads_inactive
            # printing statistics
            if args.threads_info:
                printing.print_per_thread(all_threads_stats, bytes_cov, end, active_threads_count, args, args.mutator_weights)
            else:
                printing.print_summary(all_threads_stats, bytes_cov, end, active_threads_count, args, UPDATE, args.mutator_weights)

            if args.stop_after_nseconds != 0.0 and args.stop_after_nseconds < end:
                INFO(0, None, None, "Stopping manul due to stop_after_nseconds option %d" % end)
                #kill_all(os.getpid())
                sys.exit(0)

            time.sleep(STATS_FREQUENCY)
    except (KeyboardInterrupt, SystemExit):
        INFO(0, None, None, "Stopping all fuzzers and threads")
        kill_all(os.getpid())
        # TODO: ideally, if we have UDS opened we should clean them with unlink() function here.
        INFO(0, None, None, "Stopped, exiting")
        sys.exit()
