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

GEN_TOTAL_FREQ = 100

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
    if is_print:
        print(out.decode("utf-8"), err.decode("utf-8"))
    return True

def execute_command(cmd_to_run, timeout, is_print):
    print(cmd_to_run)
    process = subprocess.Popen(cmd_to_run, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    preexec_fn=os.setsid)

    handle_return(process, timeout, is_print)
    return None

def parse_manul_dir(manul_out_dir):
    queue_list = list()
    for subdir, dirs, files in os.walk(manul_out_dir):
        if "queue" in subdir and ".state" not in subdir:
            for file in files:
                queue_list.append(os.path.join(subdir, file))
    return queue_list

def aggregate_coverage(out_dir, full_report_name):
    aggr_cmd_to_run = "lcov"
    for file_name in get_list_of_files(out_dir, ".info"):
        aggr_cmd_to_run += " -a " + file_name
    execute_command(aggr_cmd_to_run + " -o %s" % (full_report_name), 30, True)


def handle_coverage(src_files, out_dir, iteration):
    execute_command("lcov -c -d %s -o %s/tmp_%d.info" % (src_files, out_dir, iteration), 30, False) 
    if iteration > 0 and (iteration % GEN_TOTAL_FREQ == 0):
        # aggregating coverage into for every 100's run
        print("Aggregating coverage")
        aggregate_coverage(out_dir, "%s/total_%d.info" % (out_dir, iteration))
        execute_command("rm -f %s/tmp_*" % (out_dir), 30, False)
        execute_command("rm -f %s/total_%d.info" % (out_dir, i - GEN_TOTAL_FREQ), 30, False)

def generate_report(res_out_dir):
    aggregate_coverage(res_out_dir, "%s/total.info" % res_out_dir)
    execute_command("genhtml %s/total.info -o %s" % (res_out_dir, res_out_dir), 30, True)

def is_folder_clean(res_out_dir):
    if not os.path.exists(res_out_dir):
        print("%s does not exist, create it" % res_out_dir)
        sys.exit(0)
    if len(os.listdir(res_out_dir)) != 0:
        return False
    return True

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog = "evaluate_coverage.py",
                                     description = 'Script to obtain function level coverage from Manul or AFL output dir.',
                                     usage = '%(prog)s -src [path_to_src] -fuzzing_out [path_to_manul_out_dir] -out [dir_to_save_results] "/path/binary -a -b @@"')
    requiredNamed = parser.add_argument_group('Required parameters')
    requiredNamed.add_argument("-src", required=True, dest="src_files", help = "Path to directory with project source code compiled with gcov")
    requiredNamed.add_argument("-fuzzing_out", required=True, dest="manul_out_dir", help = "Path to directory where Manul or AFL saved results")
    requiredNamed.add_argument("-out", required=True, dest="res_out_dir", help = "Path to directory where coverage results should be saved")
    requiredNamed.add_argument('cmd_to_run', nargs='*', help="The target binary and options to be executed (quotes needed e.g. \"target -png @@\")")
    args = parser.parse_args()
    if not is_folder_clean(args.res_out_dir):
        print("Folder %s is not empty. Please clean it before launching the script" % args.res_out_dir)
        sys.exit(0)

    files_to_run = parse_manul_dir(args.manul_out_dir)
    if not files_to_run or len(files_to_run) == 0:
        print("Unable to enumerate files from queue in the directory provided. Please specify correct Manul or AFL dir")
        sys.exit(0)
    
    cmd_to_run = "".join(args.cmd_to_run)

    for i, file_to_execute in enumerate(files_to_run):
        print("%d out of %d" % (i, len(files_to_run)))
        final_cmd_to_run = cmd_to_run.replace("@@", file_to_execute)
        execute_command(final_cmd_to_run, 120, False)
        handle_coverage(args.src_files, args.res_out_dir, i)

    generate_report(args.res_out_dir)
