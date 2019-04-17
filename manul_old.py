from os import listdir
from os.path import isfile, join
import subprocess
import shutil
from ctypes import *
import multiprocessing
import argparse
from timeit import default_timer as timer
import ntpath
import printing
import threading
from printing import *
from manul_utils import *
import manul_network
import random

PY3 = sys.version_info[0] == 3

if PY3:
    string_types = str,
else:
    string_types = basestring,

class Fuzzer:
  def __init__(self, list_of_files, fuzzer_id, virgin_bits_global, input_path, output_path, is_dumb_mode, target_binary,
               timeout, stats_array, enable_logging, restore_session, dbi_name, determenistic, crash_bits):
    # local fuzzer config
    global SHM_SIZE
    self.SHM_SIZE = SHM_SIZE
    self.CALIBRATIONS_COUNT = 7
    self.SHM_ENV_VAR = "__AFL_SHM_ID"
    self.dbi = dbi_name

    self.list_of_files = list_of_files
    self.fuzzer_id = fuzzer_id
    self.virgin_bits = list()
    for i in range(0, SHM_SIZE):
        self.virgin_bits.append(0xFF)

    self.global_map = virgin_bits_global
    self.crash_bits = crash_bits # happens not too often

    self.timeout = timeout
    self.stats_array = stats_array
    self.restore = restore_session

    self.determenistic = determenistic
    if self.determenistic:
        random.seed(a=self.fuzzer_id)

    #creating output dir structure
    self.output_path = output_path + "/%d" % fuzzer_id
    self.queue_path = self.output_path + "/queue"
    self.crashes_path = self.output_path + "/crashes"
    self.unique_crashes_path = self.crashes_path + "/unique"

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


    self.is_dumb_mode = is_dumb_mode
    self.input_path = input_path
    self.target_binary_path = target_binary # and its arguments

    self.fuzzer_stats = FuzzerStats()
    self.stats_file = None
    if self.restore:
      if not isfile(self.output_path + "/fuzzer_stats"):
          ERROR("Fuzzer stats file doesn't exist. Make sure your output is actual working dir of manul")

      self.stats_file = open(self.output_path + "/fuzzer_stats", 'r')
      content = self.stats_file.readlines()
      line = None
      for line in content: # getting last line from file to restore ression
          pass
      if line is None:
          ERROR("Failed to restore fuzzer %d from stats. Invalid fuzzeR_stats format" % self.fuzzer_id)

      last = line[:-2] # skipping last symbol space and \n
      INFO(0, None, None, "Restoring last stats %s" % last)
      self.stats_file.close()
      self.restore_session(last)

      self.stats_file = open(self.output_path + "/fuzzer_stats", 'a')
    else:
      self.stats_file = open(self.output_path + "/fuzzer_stats", 'w')

    self.enable_logging = enable_logging
    self.log_file = None
    if self.enable_logging:
      self.log_file = open(self.output_path + "/fuzzer_log", 'a')

    if not self.is_dumb_mode:
      self.trace_bits = self.setup_shm()

    for i in range(0, self.SHM_SIZE):
      if self.virgin_bits[i] != 0xFF:
        self.global_map[i] = self.virgin_bits[i]
      elif self.global_map[i] != 0xFF and self.virgin_bits[i] == 0xFF:
        self.virgin_bits[i] = self.global_map[i]

#TODO: save timeouts options?
  def sync_bitmap(self):
    for i in range(0, self.SHM_SIZE):
      if self.virgin_bits[i] != 0xFF:
        self.global_map[i] = self.virgin_bits[i]
      elif self.global_map[i] != 0xFF and self.virgin_bits[i] == 0xFF:
        self.virgin_bits[i] = self.global_map[i]

  def restore_session(self, last):
    # parse previously saved stats line
    last = last.split(" ")[1:] # cut timestamp
    for i, stat in enumerate(last):
        stat = float(stat.split(":")[1])  # taking actual value
        # TODO: not compatible with Python3
        # https://stackoverflow.com/questions/10058140/accessing-items-in-an-collections-ordereddict-by-index
        stat_name = self.fuzzer_stats.stats.items()[i][0]
        self.fuzzer_stats.stats[stat_name] = stat

    if self.deterministic: # skip already seen seeds
        for i in range(0, self.fuzzer_stats.stats['iterations']):
            random.seed(seed=self.fuzzer_id)

  def save_stats(self):
      if self.stats_file is None:
          return

      self.stats_file.write(str(time.time()) + " ")
      for i, (k,v) in enumerate(self.fuzzer_stats.stats.items()):
          self.stats_file.write("%d:%.2f " % (i,v))
      self.stats_file.write("\n")

  def prepare_cmd_to_run(self, target_file_path):
    if self.dbi:
        dbi_tool_opt = "-c"
        if self.dbi == "pin":
            dbi_tool_opt = "-t"
        timeout_long = self.timeout * 30 # DBI introduce overhead ~ x30
        timeout_prefix = ("timeout %ds" % timeout_long)
        binary_path = "".join(self.target_binary_path)
        binary_path = binary_path.replace("@@", target_file_path)

        final_string = "%s %s %s %s %s -- %s" % (
                        timeout_prefix, DBI_ENGINE_PATH, dbi_tool_opt, DBI_TOOL_PATH, DBI_TOOL_PARAMS, binary_path)
    else:
        final_string = ("timeout %ds " % self.timeout) + "".join(self.target_binary_path)
        final_string = final_string.replace("@@", target_file_path)

    return final_string

  def setup_shm(self):
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
    shmat.restype = c_void_p

    shmid = shmget(IPC_PRIVATE, self.SHM_SIZE, 0o666)
    if shmid < 0:
      ERROR("shmget() failed")

    addr = bytearray(self.SHM_SIZE)
    addr = shmat(shmid, None, 0)

    INFO(0, None, self.log_file, "Setting up shared mem %d for fuzzer:%d" % (shmid, self.fuzzer_id))
    os.environ[self.SHM_ENV_VAR] = str(shmid)

    return addr

  def dry_run(self): # TODO: wait for dry run
    INFO(0, bcolors.BOLD + bcolors.HEADER, self.log_file, "Performing dry run")
    useless = 0

    for file_name in self.list_of_files:
      memset(self.trace_bits, 0x0, SHM_SIZE)

      cmd = self.prepare_cmd_to_run(self.input_path + "/" + file_name)

      INFO(1, bcolors.BOLD, self.log_file, "Running %s" % cmd)

      try:
        res = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
      except subprocess.CalledProcessError as exc:
        INFO(1, None, self.log_file, "Initial input %s triggers exception in the target" % file_name)
        if self.is_critical(exc):
          WARNING(self.log_file, "Initial input %s leads target to crash (did you disable leak sanitizer?). Enable -d or -l to check actual output" % file_name)
          INFO(1, None, self.log_file, exc.output)
        elif self.is_problem_with_config(exc):
          WARNING(self.log_file, "Problematic file %s" % file_name)
        res = exc.output

      trace_bits_as_str = string_at(self.trace_bits, SHM_SIZE)

      # count non-zero bytes just to check that instrumentation actually works
      non_zeros = [x for x in trace_bits_as_str if ord(x) != 0x0]
      if len(non_zeros) == 0:
        INFO(1, None, self.log_file, "Output from target %s" % res)
        ERROR("%s doesn't cover any path in the target, Make sure the binary is actually instrumented" % file_name)

      ret = self.has_new_bits(trace_bits_as_str, True, list(), self.virgin_bits)
      if ret == 0:
        useless += 1
        WARNING(self.log_file, "Test %s might be useless because it doesn't cover new paths in the target, consider removing it" % file_name)
      else:
        self.sync_bitmap()

    if useless != 0:
      WARNING(self.log_file, "%d out of %d initial files are useless" % (useless, len(self.list_of_files)))

    INFO(0, bcolors.BOLD + bcolors.OKBLUE, self.log_file, "Dry run finished")
    self.fuzzer_stats.stats['iterations'] += 1.0
    self.update_stats()

  def has_new_bits(self, trace_bits_as_str, update_virgin_bits, volatile_bytes, bitmap_to_compare):
    ret = 0
    for j in range(0, SHM_SIZE):

      if j in volatile_bytes:
        continue # ignoring volatile bytes

      trace_byte = ord(trace_bits_as_str[j])
      virgin_byte = bitmap_to_compare[j]

      if trace_byte and (trace_byte & virgin_byte):
        if ret < 2 and (trace_byte != 0x0 and virgin_byte == 0xff):
          ret = 2 # new path discovered
        else:
          ret = 1

        virgin_byte = virgin_byte & ~trace_byte

        if update_virgin_bits:
            bitmap_to_compare[j] = virgin_byte # python will handle syncronization issues

    return ret

  def calibrate_test_case(self, file_name):
    volatile_bytes = list()
    trace_bits_as_str = string_at(self.trace_bits, self.SHM_SIZE) # this is how we read memory in Python

    bitmap_to_compare = list("\x00" * self.SHM_SIZE)
    for i in range(0, self.SHM_SIZE):
      bitmap_to_compare[i] = ord(trace_bits_as_str[i])

    for i in range(0, self.CALIBRATIONS_COUNT):
      INFO(1, None, self.log_file, "Calibrating %s %d" % (file_name, i))

      memset(self.trace_bits, 0x0, SHM_SIZE)

      cmd = self.prepare_cmd_to_run(self.queue_path + "/" + file_name)
      try:
        INFO(1, None, self.log_file, cmd)
        res = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True) # running our target again
      except subprocess.CalledProcessError as exc:
        INFO(1, None, self.log_file, "Error is not expected during calibration, something wrong with %s" % file_name)
        INFO(1, None, self.log_file, exc)

      trace_bits_as_str = string_at(self.trace_bits, SHM_SIZE) # this is how we read memory in Python

      for j in range(0, SHM_SIZE):
        trace_byte = ord(trace_bits_as_str[j])
        if trace_byte != bitmap_to_compare[j]:
          if j not in volatile_bytes:
            volatile_bytes.append(j) # mark offset of this byte as volatile

    INFO(1, None, self.log_file, "We have %d volatile bytes for this new finding" % len(volatile_bytes))
    # let's try to check for new coverage ignoring volatile bytes
    self.fuzzer_stats.stats['blacklisted_paths'] = len(volatile_bytes)
    return self.has_new_bits(trace_bits_as_str, True, volatile_bytes, self.virgin_bits) # it means that we're returning result of the last run (might be wrong)

  def update_stats(self):
    for i, (k,v) in enumerate(self.fuzzer_stats.stats.items()):
      self.stats_array[i] = v

  def is_problem_with_config(self, exc):
      if exc.returncode == 127 or exc.returncode == 126: # command not found or permissions
          ERROR("Thread %d unable to execute target. Bash return %s" % (self.fuzzer_id, exc.output))
      elif exc.returncode == 124: #timeout
          WARNING(self.log_file, "Target failed to finish execution within given timeout, try to increase default timeout")
          return True
      return False

  def generate_new_name(self, file_name):
      iteration = int(round(self.fuzzer_stats.stats['iterations']))
      if file_name.startswith("raiter"): # raiterDateTime:FuzzerId:iteration_original_name
          base_name = file_name[file_name.find("_")+1:]
          file_name = base_name

      now = int(round(time.time()))
      return "raiter%d:%d:%d_%s" % (now, self.fuzzer_id, iteration, file_name)

  def is_critical(self, exc):
      if "Sanitizer" in exc.output or "SIGSEGV" in exc.output:
          return True
      if exc.returncode > 128:  # this is the way to check for errors in no-ASAN mode
          if IGNORE_ABORT and exc.returncode == (128 + 6): # signal 6 is SIABRT
            return False
          return True
      return False

  def run(self):
    if not self.is_dumb_mode:
      self.dry_run()

    last_stats_saved_time = 0

    if self.restore:
      INFO(0, bcolors.BOLD + bcolors.OKBLUE, self.log_file, "Session sucessfully restored")

    while True: # never return
      new_files = list() # empty the list
      elapsed = 0
      init_start = timer()
      new_seed_str = ""
      if self.determenistic:
          new_seed = random.randint(0, sys.maxsize)
          new_seed_str = "--seed %d " % new_seed

      for i, file_name in enumerate(self.list_of_files):
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

        mutated_name = file_name + "_mutated"
        full_output_file_path = self.queue_path + "/" + mutated_name

        # command to generate new input using radamsa
        cmd = "radamsa %s%s > %s" % (new_seed_str, full_input_file_path, full_output_file_path)
        INFO(1, None, self.log_file, "Running %s" % cmd)
        try:
          res = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True) # generate new input
        except subprocess.CalledProcessError as exc:
          WARNING(self.log_file, "Fuzzer %d failed to generate new input from %s due to some problem with radamsa. Error code %d. Return msg %s" % 
                  (self.fuzzer_id, file_name, exc.returncode, exc.output))
          continue

        cmd = self.prepare_cmd_to_run(full_output_file_path)
        INFO(1, None, self.log_file, "Running %s" % cmd)
        start = timer()
        try:
          res = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True) # run target with new input
        except subprocess.CalledProcessError as exc:
          elapsed += (timer() - start)
          self.fuzzer_stats.stats['exceptions'] += 1
          INFO(1, None, self.log_file, "Target raised exception and returns %d error code" % exc.returncode)

          if self.is_critical(exc):
            INFO(0, bcolors.BOLD + bcolors.OKGREEN, self.log_file, "New crash found by fuzzer %d" % self.fuzzer_id)
            self.fuzzer_stats.stats["last_crash_time"] = time.time()

            new_name = self.generate_new_name(file_name)
            shutil.copy(full_output_file_path, self.crashes_path + "/" + new_name) # copying into crash folder
            self.fuzzer_stats.stats['crashes'] += 1

            if not self.is_dumb_mode:
              trace_bits_as_str = string_at(self.trace_bits, SHM_SIZE) # this is how we read memory in Python
              ret = self.has_new_bits(trace_bits_as_str, True, list(), self.crash_bits)
              if ret == 2:
                INFO(0, bcolors.BOLD + bcolors.OKGREEN, self.log_file, "Crash is unique")
                self.fuzzer_stats.stats['unique_crashes'] += 1
                shutil.copy(full_output_file_path, self.unique_crashes_path + "/" + new_name) # copying into crash folder with unique crashes

            crash_found = True

          elif self.is_problem_with_config(exc):
            WARNING(self.log_file, "Problematic file: %s" % file_name)

        elapsed += (timer() - start)

        if not crash_found and not self.is_dumb_mode:
          # Reading the coverage
          trace_bits_as_str = string_at(self.trace_bits, SHM_SIZE) # this is how we read memory in Python
          ret = self.has_new_bits(trace_bits_as_str, False, list(), self.virgin_bits) # we are not ready to update coverage at this stage due to volatile bytes
          if ret == 2:
            INFO(1, None, self.log_file, "Input %s produces new coverage, calibrating" % file_name)
            if self.calibrate_test_case(mutated_name) == 2:
              self.fuzzer_stats.stats['new_paths'] += 1
              self.fuzzer_stats.stats['last_path_time'] = timer()
              INFO(1, None, self.log_file, "Calibration finished sucessfully. Saving new finding")
              new_coverage_file_name = self.generate_new_name(file_name)
              shutil.copy(full_output_file_path, self.queue_path + "/" + new_coverage_file_name)
              new_files.append((1, new_coverage_file_name))

        self.update_stats()

      self.sync_bitmap()

      init_end = timer() - init_start
      # calculating execution speed
      if init_end != 0:
        self.fuzzer_stats.stats['exec_per_sec'] = len(self.list_of_files) / init_end

      if len(new_files) > 0:
        self.list_of_files = self.list_of_files + new_files
      self.fuzzer_stats.stats['iterations'] += 1.0
      self.fuzzer_stats.stats['files_in_queue'] = len(self.list_of_files)

      self.update_stats()
      last_stats_saved_time += elapsed
      if last_stats_saved_time > 1: # we save fuzzer stats per iteration or once per second to avoid huge stats files
        self.save_stats()
        last_stats_saved_time = 0

def get_bytes_covered(virgin_bits):
    non_zeros = [x for x in virgin_bits if x != 0xFF]
    return len(non_zeros)

def run_fuzzer_instance(files_list, i, virgin_bits, args, stats_array, restore_session, crash_bits):
    INFO(0, None, None, "Starting fuzzer %d" % i)
    fuzzer_instance = Fuzzer(files_list, i, virgin_bits, args.input, args.output, args.simple_mode, args.target_binary,
                             args.timeout, stats_array, args.logging_enable, restore_session, args.dbi,
                             args.determinstic_seed, crash_bits)
    fuzzer_instance.run() # never return

def check_instrumentation(target_binary):
    with open(target_binary, 'rb') as f:
        s = f.read()
    res = s.find("__AFL_SHM_ID")
    if res == -1:
        return False
    return True

def which(program):
    def is_binary(fpath):
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_binary(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            exec_file = os.path.join(path, program)
            if is_binary(exec_file):
                return exec_file

    return None

def check_binary(target_binary):
    target_binary = which(target_binary)
    if target_binary is None:
        ERROR("Unable to find target binary %s" % target_binary)

def get_available_id_for_backup(dir_name):
    id = 0
    tmp = dir_name + "_%d" % id
    while True:
        if not os.path.exists(tmp):
            return id
        id += 1
        tmp = dir_name + "_%d" % id

def configure_dbi(dbi_name, target_binary):
    global DBI_ENGINE_PATH, DBI_TOOL_PATH, DBI_TOOL_PARAMS
    DBI_ENGINE_PATH = os.getenv('DBI_ROOT')
    DBI_TOOL_PATH = os.getenv('DBI_CLIENT_ROOT')
    dbi_tool_libs = os.getenv('DBI_CLIENT_LIBS')

    if DBI_ENGINE_PATH is None or DBI_TOOL_PATH is None:
        ERROR("DBI_ROOT and/or DBI_CLIENT_ROOT paths not specified, unable to execute manul")

    check_binary(DBI_ENGINE_PATH)
    check_binary(DBI_TOOL_PATH)

    DBI_TOOL_PARAMS = ""
    if dbi_name == "dynamorio":
        #TODO: corner case https://stackoverflow.com/questions/8384737/extract-file-name-from-path-no-matter-what-the-os-path-format
        DBI_TOOL_PARAMS += "-coverage_module %s " % ntpath.basename(target_binary)
        if dbi_tool_libs is not None:
            for target_lib in dbi_tool_libs.split(","):
                if target_lib == "":
                    continue
                DBI_TOOL_PARAMS += "-coverage_module %s " % target_lib
    elif dbi_name == "pin": # TODO: handle when dbi_tool_libs is None
        if dbi_tool_libs is not None:
            # adding desired libs to instrument
            fd = open("dbi_config", 'w')
            fd.write(dbi_tool_libs)
            fd.close()
            dbi_config_file_path = os.path.abspath("dbi_config")
            DBI_TOOL_PARAMS += " -libs %s" % dbi_config_file_path
    else:
        ERROR("Unknown dbi engine/option specified. Intel PIN or DynamoRIO are only supported")

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
    if args.net_slave is not None:
        files = manul_network.get_files_list_from_master(args.net_slave, args.nfuzzers) # ask master to provide list of files
        check_if_exist(files, args.input)
        return split_files_by_count(files, args.nfuzzers)

    files_list = get_files_list(args.input)

    if args.net_master_path is not None:

        ips = manul_network.get_slaves_ips(args.net_master_path)
        slaves, total_threads_count = manul_network.get_remote_threads_count(ips) # ask slaves count and threads
        total_threads_count += args.nfuzzers

        files = split_files_by_count(files_list, total_threads_count)
        piece_id = 0
        for ip, port, slave_threads_count in slaves:
            manul_network.send_files_list(ip, port, files[piece_id:slave_threads_count + piece_id]) #  send them files list
            piece_id += slave_threads_count
        files = files[piece_id:]
    else:
        files = split_files_by_count(files_list, args.nfuzzers)

    return files

def parse_args():
    #TODO: better usage print formating (argparse support it by default, see formatter class) + dbi and simple are not used together
    parser = argparse.ArgumentParser(prog = "manul.py", 
                                     description = '????',
                                     usage = '%(prog)s -i /home/user/inputs_dir -o /home/user/outputs_dir -n 40 "pdftocairo -png @@"')
    requiredNamed = parser.add_argument_group('Required parameters')
    requiredNamed.add_argument('-i', '--input', required=True, help = "Path to directory that contains initial files for fuzzing")
    requiredNamed.add_argument('-o', '--output', required=True, default="manul_output", help = "Path to directory where manul should save new findings and results")

    parser.add_argument('-r', '--restore', default=False, action='store_true', help = "Restore previous session")
    parser.add_argument('-n', '--nfuzzers', default=1, type=int, help = "Number of parallel fuzzers")
    parser.add_argument('-x', '--determinstic_seed', default=False, action='store_true', help = "Use determenistic seed for test cases generation (radamsa)")
    parser.add_argument('-s', '--simple', default=False, action='store_true', dest="simple_mode", help = "Run dumb fuzzing (no code instrumentation)")
    parser.add_argument('-p', '--print_per_thread', default=False, action='store_true', dest="threads_info", help = "Print fuzzing summary per thread instead of total summary")
    parser.add_argument('-b', '--dbi', help = "Choose DBI backend to provide coverage back to manul (dynamorio or pin)")
    parser.add_argument('-t', '--timeout', default=10, type=int, help = "Specify timeout for target binary")
    parser.add_argument('-z', '--net_master_path', help = "[Master option] Specify path to config file with a list of IP:port addresses")
    parser.add_argument('-c', '--net_slave', help="[Slave option] specify IP and port to listen for connection from master (e.g. 0.0.0.0:1337)")
    parser.add_argument('-d', '--debug', default=False, action='store_true', help = "Run in debug mode, print details in console")
    parser.add_argument('-m', '--manul_logo', default=False, action='store_true', help = "Print manul ASCII logo")
    parser.add_argument('-l', '--logging_enable', default=False, action='store_true', help = "Save debug messages to log files (one per thread)")
    parser.add_argument('target_binary', nargs='*', help="The target binary and options to be executed.")
    args = parser.parse_args()

    if args.manul_logo:
        printing.print_logo()

    if "@@" not in args.target_binary[0]:
        ERROR("Your forgot to specify @@ for your target. Call manul.py -h for more details")
    modes_count = 0
    if args.simple_mode and args.dbi is not None:
        ERROR("Options mismatch. Simple mode can't be executed with DBI mode together. Use -s or -b dynamorio")

    if args.restore and not args.simple_mode:
        ERROR("Session restore for coverage-guided mode is not yet supported")

    return args

if __name__ == "__main__":
    start = timer()
    args = parse_args()

    printing.DEBUG_PRINT = args.debug

    binary_to_check = args.target_binary[0]
    target_binary = binary_to_check.split(" ")[0] # TODO: here we assume that our path to binary doesn't have spaces

    if args.dbi is not None:
        configure_dbi(args.dbi, target_binary)

    check_binary(target_binary) # check if our binary exists and is actually instrumented

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

    files = allocate_files_per_jobs(args)

    # TODO: allocate only in case of smart fuzzing
    virgin_bits = multiprocessing.Array("i", SHM_SIZE)
    crash_bits = multiprocessing.Array("i", SHM_SIZE)
    for i in range(0, SHM_SIZE):
        virgin_bits[i] = 255 # initalizing with all 0xFFs
        crash_bits[i] = 255


    # allocating data structures where we store all statistics about our fuzzers
    stats = FuzzerStats()
    all_threads_stats = list()
    all_threads_handles = list()
    for i, files_piece in enumerate(files):
        stats_array = multiprocessing.Array("d", stats.get_len())
        t = multiprocessing.Process(target=run_fuzzer_instance, args=(files_piece, i, virgin_bits, args, stats_array,
                                                                      args.restore, crash_bits))
        t.start()
        all_threads_stats.append(stats_array)
        all_threads_handles.append(t)

    time.sleep(1) # TODO: wait while all threads are finishing dry run (+ we have zeros in some stats till iteration 1)
    INFO(0, None, None, "%d fuzzer instances sucessfully started" % args.nfuzzers)

    if (args.net_slave is not None or args.net_master_path is not None) and not args.simple_mode:
        # TODO: print status info about this thread
        INFO(1, None, None, "Allocating special thread for bitmap syncronization")
        ips = None
        if args.net_master_path is not None:
            ips = manul_network.get_slaves_ips(args.net_master_path)
            sync_t = threading.Thread(target=manul_network.sync_remote_bitmaps,
                                      args=(virgin_bits, ips))
        elif args.net_slave is not None:
            sync_t = threading.Thread(target=manul_network.receive_bitmap_slave,
                                      args=(args.net_slave, virgin_bits))
        sync_t.setDaemon(True)
        sync_t.start()

    try:
        while True: #TODO: https://stackoverflow.com/questions/2564137/python-how-to-terminate-a-thread-when-main-program-ends
            threads_inactive = 0
            for i, t in enumerate(all_threads_handles):
                if not t.is_alive():
                    threads_inactive += 1
                    WARNING(None, "Fuzzer %d unexpectedly terminated" % i)
                    if not args.simple_mode:
                        continue
                    WARNING(None, "Restoring %d fuzzer" % i)
                    files_list = files[i]
                    stats_array = all_threads_stats[i]
                    t = multiprocessing.Process(target=run_fuzzer_instance, # trying to restore fuzzer
                                                args=(files_list, i, virgin_bits, args, stats_array, True))
                    t.start()
                    all_threads_handles[i] = t
            end = timer() - start
            bytes_cov = get_bytes_covered(virgin_bits)
            active_threads_count = len(all_threads_handles) - threads_inactive
            # printing statistics
            if args.threads_info:
                printing.print_per_thread(all_threads_stats, bytes_cov, end, active_threads_count, args)
            else:
                printing.print_summary(all_threads_stats, bytes_cov, end, active_threads_count, args, UPDATE)

            time.sleep(STATS_FREQUENCY)
    except (KeyboardInterrupt, SystemExit):
        INFO(0, None, None, "Stopping all fuzzers and threads")
        sys.exit()
