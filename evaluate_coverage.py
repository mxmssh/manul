#   Manul - evaluate coverage script
#   -------------------------------------
#   Maksim Shudrak <mshudrak@salesforce.com> <mxmssh@gmail.com>
#
#   Copyright 2020 Salesforce.com, inc. All rights reserved.
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

import os
import sys
import psutil
import subprocess
import argparse
import signal
import hashlib
import shutil

GEN_TOTAL_FREQ = 100
DEBUG = False

def get_list_of_files(out_dir, ext = None):
    queue_list = list()
    for subdir, dirs, files in os.walk(out_dir):
        for file in files:
            if ext and file.endswith(ext):
                queue_list.append(os.path.join(subdir, file))
    return queue_list

def kill_process(p):
    try:
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

def handle_return(process, default_timeout, is_print):
    try:
        out, err = process.communicate(timeout=default_timeout)
    except subprocess.TimeoutExpired:
        kill_all(process.pid)
        return False
    if is_print or DEBUG:
        print(out.decode("utf-8"), err.decode("utf-8"))
    return True

def execute_command(cmd_to_run, timeout, is_print):
    print(cmd_to_run)
    if args.win:
        process = subprocess.Popen(cmd_to_run, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    else:
        process = subprocess.Popen(cmd_to_run, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                        preexec_fn=os.setsid)

    handle_return(process, timeout, is_print)
    return None

def get_hash(file_full_path):
    # BUF_SIZE is totally arbitrary, change for your app!
    BUF_SIZE = 65536  # lets read stuff in 64kb chunks!
    sha1 = hashlib.sha1()
    with open(file_full_path, 'rb') as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha1.update(data)

    return sha1.hexdigest()
    

def parse_manul_dir(manul_out_dir):
    queue_list = list()
    hash_list = list()
    for subdir, dirs, files in os.walk(manul_out_dir):
        if "queue" in subdir and ".state" not in subdir:
            for file in files:
                new_hash = get_hash(os.path.join(subdir, file))
                if new_hash in hash_list:
                    print("Duplicate file %s" % os.path.join(subdir, file))
                    continue
                else:
                    hash_list.append(new_hash)
                queue_list.append(os.path.join(subdir, file))
    return queue_list

def handle_coverage(src_files, out_dir, branch_coverage):
    exec_command = "lcov "
    if branch_coverage:
        exec_command += "--rc lcov_branch_coverage=1 "
    execute_command(exec_command + "-c -d %s -o %s/total.info" % (src_files, out_dir), 120, False) 

def generate_report(res_out_dir, branch_coverage):
    exec_command = "genhtml "
    if branch_coverage:
        exec_command += "--branch-coverage "
    execute_command(exec_command + "%s/total.info -o %s" % (res_out_dir, res_out_dir), 120, True)

def clean_src_dir(src_folder):
    execute_command("lcov -z -d %s" % src_folder, 120, True)
    execute_command("lcov --initial -c -d %s" % src_folder, 120, False)

def is_folder_clean(res_out_dir):
    if not os.path.exists(res_out_dir):
        print("%s does not exist, create it" % res_out_dir)
        sys.exit(0)
    if len(os.listdir(res_out_dir)) != 0:
        return False
    return True

def safe_file_to_special_path(src_file, rel_path):
    shutil.copy(src_file, rel_path)
    return rel_path

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog = "evaluate_coverage.py",
                                     description = 'Script to obtain function level coverage from Manul or AFL output dir.',
                                     usage = '%(prog)s -src [path_to_src] -fuzzing_out [path_to_manul_out_dir] -out [dir_to_save_results] "/path/binary -a -b @@"')
    requiredNamed = parser.add_argument_group('Required parameters')
    requiredNamed.add_argument("-dr_path", required=False, default=None, dest="dynamorio_path", help = "Evaluate coverage from winAFL/manul on Windows")
    requiredNamed.add_argument("-src", required=True, dest="src_files", help = "Path to directory with project source code compiled with gcov")
    
    requiredNamed.add_argument("-fuzzing_out", required=True, dest="manul_out_dir", help = "Path to directory where Manul or AFL saved results")
    requiredNamed.add_argument("-out", required=True, dest="res_out_dir", help = "Path to directory where coverage results should be saved")
    requiredNamed.add_argument("-win", required=False, default = False, action = 'store_true', help = "Evaluate coverage from wiNAFL/manul on Windows")
    requiredNamed.add_argument("-f", required=False, default = None, dest="rel_path", help = "Save file in specific location (similar to -f option for winAFL)")
    requiredNamed.add_argument("-branch", required=False, default = False, action = 'store_true', help = "Evaluate branch coverage on Linux")
    requiredNamed.add_argument('cmd_to_run', nargs='*', help="The target binary and options to be executed (quotes needed e.g. \"target -png @@\")")
    args = parser.parse_args()
    print(args)
    if args.win and not args.dynamorio_path:
        print("DynamoRIO path should be specified for Windows code coverage evaluation")
        sys.exit(0)
    if not is_folder_clean(args.res_out_dir):
        print("Folder %s is not empty. Please clean it before launching the script" % args.res_out_dir)
        sys.exit(0)

    files_to_run = parse_manul_dir(args.manul_out_dir)
    if not files_to_run or len(files_to_run) == 0:
        print("Unable to enumerate files from queue in the directory provided. Please specify correct Manul or AFL dir")
        sys.exit(0)
    cmd_to_run = "".join(args.cmd_to_run)
    if not args.win:
        clean_src_dir(args.src_files)
    
    for i, file_to_execute in enumerate(files_to_run):
        print("%d out of %d" % (i, len(files_to_run)))
        if args.rel_path:
           print("saving file as %s" % args.rel_path)
           file_to_execute = safe_file_to_special_path(file_to_execute, args.rel_path)
 
        final_cmd_to_run = cmd_to_run.replace("@@", file_to_execute)
        if args.win:
            final_cmd_to_run = "%s\\drrun.exe -t drcov -logdir %s -- %s" % (args.dynamorio_path, args.res_out_dir, final_cmd_to_run)
            execute_command(final_cmd_to_run, 180, False)
        else:
 
            execute_command(final_cmd_to_run, 120, False)

    if args.win:
        print("log files has been saved in %s dir. Provide them to lighthouse to get code coverage evaluation" % args.res_out_dir)
    else:
        handle_coverage(args.src_files, args.res_out_dir, args.branch)
        generate_report(args.res_out_dir, args.branch)
