#   Manul - printing module
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

from manul_utils import *
import psutil
import time
import datetime
from datetime import timedelta

DEBUG_PRINT = False
MANUL_VERSION = "0.3"

#TODO: colors doesn't work in Windows
def print_logo():
    print(r"""88EDdWkmWZMEg8QBQQQ#QQQB#QQQ8QDO0OZdIkomZPH0d5ZHyWOgQQgg8g0600$R06dMEZaH3s3zyhoH
    9DERGzj3sIkhM9gQBBQQgQB8Q8ZkxIj5R98Myue$g8BQQd6g0E5OD66E8QQQQQQ$g$gQ$90QQBB8gEDE
    Qg09dqPPMbZEgOW8Q8g0$Q8B8Q0PWMhKMZz*!)zmazwhHZ9$Dduhqo3DgB#QBggQ8B#B$$dwGdWkwab6
    $9dd9$D0$06Eg0dO$$$D6g$80R8QgdZ0day*~xVyITVbZ00E$$$BQQQ860gd0ODDOE$8$MuVyyTv)vuy
    QQBQ$gQBBQg9OZMbdEE8b536z}e$g8wij66qzuLVMEO05zVsQBgMlVzZ80d3PdmmIuhsIWmqHwYxrvVz
    $D$BBBBQQ896maMZ06Og$0Wu}TK0$EyuZ$qwwx)YO88P}v^k8Rs*>^rlIZdZjemGayzushMdHmIcTzmm
    gg8QQQQQ$696RODdEQg$OOODRDOZgD00Q9V)v~**yRgD$hzbZezTTvxVWKHWM5MjzhshuxuukeOD880d
    QBBBQBQgd6d00d08QBQ8Pm68Qgg$9D8Dmv~~r:=:*6gRdaZaPaIwVVVzhyIdOmyyIymMzTYcs56OMqqm
    ####BQQg$gg$666ZucuuxTI$B8$0dggOwcP6ykemm0EqywOqdMd5zjjwwkyVmdq5KmPPjoheH60DOdE$
    Q##BQg0DROZZqZd6$8dWz}}Vd0$R0QQROWMaTy5mZ3MWyP66ZWZ65IYviTYzoKhsPzwzVzIjaZZMM6Dg
    @@#Q$RE$EMPhkqgg$0EdqmjVcs0BggQ0MVT}}kojWWIKqbZbPkeo}cucVImm3HjTjWPMbMhVuVaMD$DR
    B8$EZd6EdWojIIzVx^>*=~*r|YukyIVclYTxTxY|uVyzuy}kuilTuuuuj3M5zV}LxVuuwMbKojhM0QQO
    ZHsIHO00ZzyT}vr~.'~^!_``._:<*!!=^*r^=!~**\^!:_;}Vx<".'-,""::*vxxx}lVkTYzWhkImRQ#
    OMHaHGM6ZI})~:=(!-*xr!..,::.,|r=:!^^=:!==!!"=vyx"_!:- `:>>>,`:**~rLY}jPeKWmzhMgg
    6ZPezIq3Vv!:_!lqG|:rlxv)vr:  ,Y|~!!>=!!<:,:^yV*` -^*~:!*vx|,:=",,"*Y}u3qIywY}wzG
    5Hmzomkx^^>~~xoM$Q6kx(*^!.  `"xlvv)x(*()*rr}3v.   _^vxxLxr~(k}*:__=r\Tu}TxY}uwkm
    azkux)|TceKI}}uu}}xxVM0QQg06HjVVyyycciuuuxYyjxx\|Lx}TVkmM9gRsTv^^~:,"ryIVzhIu}uw
    mV}}VjG0gQBBQEOO6Mk)!:!rzZEEwvvYwmoVuyYTuTY()*uq6B#QZjTii)iulTv*(v)r~,,=v}uwcuyI
    yuVsHZ$BBQ$dKyivuV}\ivvx}clx;r|Vyommaoyju}x*!"!*xLx^__!r}wmMOg0E066dmx=!!=vVVyjG
    ysdE$EOROZI}VPzx(\|vVmHWhyyx\v}zIW6QQ$MacLir!!!"!ruVv^!!":\VywKZ6$QEM5o})~=rvLVw
    GMZ6ERd5IYxYY}iuulyz06PjYuewxvVdQBBQg8Q0uv;"__:!:!;vl}x*<*xkeI}*vVsM6696qox*v}}u
    8g00DdmcYixxxxVWMd$QgRhxrs$QQEgQDZHjyyeqHjxvkIwVx~"~|TyTrr|xxx(()rvuhqPMMMyvr|Yw
    E$6ZPk}vr*iwPdEgER0$av)wqR8gEdWTuykVTu}v(oyuIPdDdy|==ryLujyY)(vxi|v}VoMOR6Ix=:~|
    $RZmTr^r)xTomZ0g0EbMPPkaE0QB9hxr".~**"`_=^(xysGyYT}^":*xVyyov)))^==~)VeGddMhV)!!
    0dei~::^xyHdZMR000QQ$0EOd$QBQQQ$s;,___"xad0$0$QB##Bg9bKVL)vu3myl)!,-,*lmbOERaur<
    Mc)!:<xmdE$E0gQBB####BQ88gDOE0g$dKV;:iwaO0Eddd6WmahoqPazVVuuVVyjzVr~=!riyPdEDm(;
    i**vhmsK6gQQ80DEg80ERZZqddbOdM5Wkx\)rr*\uhZZRRdOMdRdQQ$0MbZGmjzy}}x*)iukmMDDZZK}
    uVVTYiPgQEddO0QgEE$QQ$5ss3sszuTivr<:,!^rxlyjseGMHmMbZZ6088Q00gg63zu)rrvYjWD8Q8EP
    }uiTK0ROOOD8Q$HKH$##06azkVuxvx|rr)vr**~;~<***rv}TVzyG660Eg0$E$0$$bqqPzTzjyMO0$$0
    }}eE8$E6MRB8amHdQQQQDHWWHqWPGHKmPKPsjaIVVywzuixYYYceyjMMmzzq9$6Mdq6MT*vxyVW000g9
    MO8Q9MmPgQMaq6Q#ggB8ZZM5Md66OdMMMMMW5MZZZMbddZHmezVujuyPGohowH$$dMP3M3|":rLymDgR
    $BQ9MqdQQOOO$B#$gBQ6Odq5qMbdddZMqqGHPHq5MZZbddMHKhzwkeIaqZ3mIyV3$0emPPM}^_,*uVjd
    Qd609RQQ90$QB#0EQQ0$RdZ5G3m3W5MMMM5WGHHWqMMMWPKsjzhhsmMW556qeswVlsEqTvVoyc*__~vv
    Z900DgQ9g8BgQOHdQg$9ObZdqejIIImPGMMMq5qqWWWGamhkoIheePMMqMZ9mmIyulyda(;^*vzc:-,~
    qbdbO6ROd$qM$GM0Q06O96RdGesIzkkyzsmmeeehkyVzoIIwzsPHPKMdM5Pd5smey}i}qIr!^!~xk*:>
    emadG9OHdmhO0bR8g9E$DE9dMGWKzwyVyzjzzyVuYii}yjhjzzs5qaPqZMmaMomazu\rYPx=*<:=*oT^""")


# TODO: what if length is too loooooong
def fill_table(first_title, second_title, value, value2, first_table_len, second_table_len):
    first_table = "| %s: %s" % (first_title, value)
    first_table_spaces = " " * (first_table_len  - len(first_table) - 1) # for |
    first_table += first_table_spaces + "|"

    actual_second_table_len = len(second_title) + len(value2)
    if (second_title == "Crashes" or second_title == "Unique crashes") and value2 != "n/a" and int(value2) > 0:
        second_title = bcolors.BOLD + bcolors.OKGREEN + second_title + bcolors.ENDC
        value2 = bcolors.BOLD + bcolors.OKGREEN + value2 + bcolors.ENDC

    second_table = "|  %s: %s" % (second_title, value2)
    actual_second_table_len = actual_second_table_len + 5 # "|  %s: %s"
    second_table_spaces = " " * (second_table_len  - actual_second_table_len - 1) # for |
    second_table += second_table_spaces + "|"

    return "|  %s   %s |" % (first_table, second_table)


def strfdelta(tdelta, fmt):
    d = {"days": tdelta.days}
    d["hours"], rem = divmod(tdelta.seconds, 3600)
    d["minutes"], d["seconds"] = divmod(rem, 60)
    return fmt.format(**d)


def get_mutator_string(mutators):
    mutators = mutators.split(",")
    generator_str = ""
    for mutator in mutators:
        name = mutator.split(":")[0]
        generator_str += name + " "
    return generator_str


def print_per_thread(all_threads_stats, bytes_cov, time_elapsed, active_threads_count, args, mutators):
    stats = FuzzerStats()
    max_cmd_length = 80
    print("")
    print(bcolors.HEADER + "                          Manul %s. Fuzzer details" % MANUL_VERSION + bcolors.ENDC)
    print("Active threads: %d" % active_threads_count)
    mode_def_str = "n/a"
    if args.simple_mode:
        mode_def_str = "Dumb-mode"
    elif args.dbi:
        mode_def_str = "DBI"
    else:
        mode_def_str = "afl-gcc"

    generator_str = get_mutator_string(mutators)

    mode_str = "Mode: %s             Strategy: %s" % (mode_def_str, generator_str)
    logging = "Logging: %s" % ("Enabled" if args.logging_enable else 'Disabled')
    spaces = " " * (max_cmd_length - len(mode_str) - len(logging))
    print("%s%s%s" % (mode_str, spaces, logging))

    time_elapsed_delta = timedelta(seconds=(time_elapsed))
    time_elapsed_str = "Time: " + strfdelta(time_elapsed_delta, "{days}d {hours}h {minutes}m {seconds}s")

    bitmap_cov_str = ""
    spaces = ""
    if not args.simple_mode:
        bitmap_cov = bytes_cov / float(SHM_SIZE) * 100
        bitmap_cov_str = "Bitmap coverage: %.2f%%" % bitmap_cov
        spaces = " " * (max_cmd_length - len(bitmap_cov_str) - len(time_elapsed_str) - 1)

    print("%s%s%s" % (time_elapsed_str, spaces, bitmap_cov_str))
    print("")

    for i, thread_stat in enumerate(all_threads_stats):
        stability = 0.0
        for j, element in enumerate(thread_stat):
            if PY3:
                (k, v) = list(stats.stats.items())[j]  # v always 0
            else:
                (k, v) = stats.stats.items()[j]  # v always 0
            if k == 'blacklisted_paths': stability = (100 - (element * 100 / float(SHM_SIZE)))
            stats.stats[k] = element

        timestamp_end = time.time()
        last_crash_time_str = "n/a"
        time_since_last_crash = stats.stats['last_crash_time']
        if time_since_last_crash != 0:
            time_since_last_crash = timedelta(seconds=(timestamp_end - stats.stats['last_crash_time']))
            last_crash_time_str = strfdelta(time_since_last_crash, "{days}d {hours}h {minutes}m {seconds}s")

        crashes_str = "%d" % stats.stats['crashes']
        unique_crashes_str = "" if args.simple_mode else "Unique crashes:%d" % stats.stats['unique_crashes']

        print("[Fuzzer %d] Last new crash:%s Crashes:%s %s" % (i, last_crash_time_str, crashes_str, unique_crashes_str))

        if not args.simple_mode:
            last_path_time_str = "n/a"
            time_since_last_path = stats.stats['last_path_time']
            if not args.simple_mode and time_since_last_path != 0:
                time_since_last_path = timedelta(seconds=(timestamp_end - stats.stats['last_path_time']))
                last_path_time_str = strfdelta(time_since_last_path, "{days}d {hours}h {minutes}m {seconds}s")

            new_paths_str = "%d" % stats.stats['new_paths']
            stability_str = "%.2f%%" % stability

            print("[Fuzzer %d] Last new path:%s New paths:%s Stability:%s" % (i, last_path_time_str, new_paths_str,
                  stability_str))

        executions_str = "%d" % stats.stats['executions']
        exceptions_str = "%d" % stats.stats['exceptions']
        exec_per_sec = "%.5f" % stats.stats['exec_per_sec']
        print("[Fuzzer %d] Executions:%s Exceptions:%s Exec/sec:%s" % (i, executions_str, exceptions_str, exec_per_sec))

        file_running_str = "%d" % stats.stats['file_running']
        files_in_queue_str = "%d" % stats.stats['files_in_queue']
        print("[Fuzzer %d] File running:%s/%s" % (i, file_running_str, files_in_queue_str))
        print("")
        print("-" * max_cmd_length)
        print("\n")


def print_summary(all_threads_stats, bytes_cov, time_elapsed, active_threads_count, args, update, mutators):
    stats_total = FuzzerStats()
    max_cmd_length = 80
    first_table_max_len = 42
    second_table_max_len = 30

    cpu_load = psutil.cpu_percent()

    bitmap_cov = bytes_cov / float(SHM_SIZE) * 100
    for thread_stat in all_threads_stats:
        for j, element in enumerate(thread_stat):
            (k, v) = list(stats_total.stats.items())[j]
            if k == 'last_path_time' or k == 'last_crash_time' or k == 'blacklisted_paths':
                if stats_total.stats[k] < element:
                    stats_total.stats[k] = element
            else:
                stats_total.stats[k] += element  # cumulative element

    if stats_total.stats['executions'] == 0.0:  # wait while at least one thread finish dry run
        return

    if update and not args.debug:
        os.system('cls' if os.name == 'nt' else 'clear')

    timestamp_end = time.time()

    time_elapsed_delta = timedelta(seconds=time_elapsed)
    time_elapsed_str = strfdelta(time_elapsed_delta, "{days}d {hours}h {minutes}m {seconds}s")

    last_crash_time_str = "n/a"
    time_since_last_crash = stats_total.stats['last_crash_time']
    if time_since_last_crash != 0:
        time_since_last_crash = timedelta(seconds=(timestamp_end - time_since_last_crash))
        last_crash_time_str = strfdelta(time_since_last_crash, "{days}d {hours}h {minutes}m {seconds}s")

    last_path_time_str = "n/a"
    time_since_last_path = stats_total.stats['last_path_time']
    if not args.simple_mode and time_since_last_path != 0:
        time_since_last_path = timedelta(seconds=(timestamp_end - time_since_last_path))
        last_path_time_str = strfdelta(time_since_last_path, "{days}d {hours}h {minutes}m {seconds}s")

    blacklisted_paths_str = "n/a"
    bitmap_cov_str = "n/a"
    new_paths_str = "n/a"
    unique_crashes_str = "n/a"
    if not args.simple_mode:  # only related to coverage-guided mode
        blacklisted_paths_str = "%d" % stats_total.stats['blacklisted_paths']
        bitmap_cov_str = ("%.2f%%" % bitmap_cov)
        new_paths_str = ("%d" % stats_total.stats['new_paths'])
        unique_crashes_str = ("%d" % stats_total.stats['unique_crashes'])

    print(bcolors.BOLD)
    print(bcolors.HEADER + "                          Manul %s. All fuzzers summary                     " % MANUL_VERSION + bcolors.ENDC)
    active_threads_str = "---------Active threads: %d " % active_threads_count
    cpu_load_str = "CPU: %.2f%%-----" % cpu_load
    line_len = max_cmd_length - len(active_threads_str) - len(cpu_load_str)
    line = "-"*line_len
    print("%s%s%s" % (active_threads_str, line, cpu_load_str))

    print("|" + " " * 78 + "|")

    if args.simple_mode:
        mode_def_str = "Dumb-mode"
    elif args.dbi:
        mode_def_str = "DBI"
    else:
        mode_def_str = "afl-gcc"

    generator_str = get_mutator_string(mutators)

    mode_str = "|  Mode: %s             Strategy: %s" % (mode_def_str, generator_str)
    logging = "Logging: %s  |" % ("Enabled" if args.logging_enable else 'Disabled')
    spaces = " " * (max_cmd_length - len(mode_str) - len(logging))
    print("%s%s%s" % (mode_str, spaces, logging))

    print("|" + " " * 78 + "|")

    print("|  --Timing----------------------------------   --Results--------------------- |")
    print(fill_table("Time", "Crashes", time_elapsed_str, ("%d" % stats_total.stats['crashes']),
                     first_table_max_len, second_table_max_len))
    print(fill_table("Last new crash found", "Unique crashes", last_crash_time_str, unique_crashes_str,
                     first_table_max_len, second_table_max_len))
    print(fill_table("Last new path found", "Exceptions", last_path_time_str,
                     ("%d" % stats_total.stats['exceptions']), first_table_max_len, second_table_max_len))
    print ("|  ------------------------------------------   ------------------------------ |")

    print("|  --Coverage statistics---------------------   ---Performance---------------- |")
    print(fill_table("Volatile bytes", "Exec/sec", blacklisted_paths_str, ("%.5f" % stats_total.stats['exec_per_sec']),
                     first_table_max_len, second_table_max_len))
    print(fill_table("Bitmap coverage", "Executions", bitmap_cov_str, ("%d" % stats_total.stats['executions']),
                     first_table_max_len, second_table_max_len))
    print(fill_table("New paths found", "Files in queue", new_paths_str, ("%d" % stats_total.stats['files_in_queue']),
                     first_table_max_len, second_table_max_len))
    print ("|  ------------------------------------------   ------------------------------ |")

    print("--------------------------------------------------------------------------------")

    print(bcolors.ENDC)


def LOG(log_file, msg):
    if log_file is None:
        return
    current_dt = datetime.datetime.now()

    formated_time = current_dt.strftime("%Y-%m-%d %H:%M:%S")
    try:
        log_file.write("[%s] %s \n" % (formated_time, msg))
    except:
        WARNING(None, "failed to log info to the log file due to exception in LOG write %s")

def ERROR(msg):
    print(bcolors.BOLD + bcolors.FAIL + "[ERROR] %s" % msg + bcolors.ENDC)
    sys.exit(-1)

def WARNING(log_file, msg):
    if log_file is not None:
        LOG(log_file, msg)

    print(bcolors.WARNING + "[WARNING] %s" % msg + bcolors.ENDC)

def INFO(lvl, color, log_file, msg):
    if log_file is not None:
        LOG(log_file, msg)

    if not DEBUG_PRINT and lvl >= 1:
      return

    if color: print(color + "[INFO] %s" % msg + bcolors.ENDC)
    else: print("[INFO] %s" % msg)


''' Use it if you want to print bitmap in the has_new_bits function'''
def print_bitmaps(bitmap_original, bitmap, output_file_path):
    ret = 0
    fd = open("bitmap_debug", 'a')
    if output_file_path:
        content = open(output_file_path, 'r').readlines()
        fd.write("--------------------------------------------\n")
        fd.write(str(content) + "\n")

    for i in range(0, SHM_SIZE):
        trace_byte = ord(bitmap[i])
        virgin_byte = bitmap_original[i]
        if trace_byte and (trace_byte & virgin_byte):
            if ret < 2:
                if virgin_byte == 0xff:
                    ret = 2  # new path discovered
                else:
                    ret = 1  # new hit of existent paths
        if ord(bitmap[i]):
            fd.write("%d: %s %s\n" % (i, hex(virgin_byte), hex(trace_byte)))
    fd.write("Will return %d\n" % ret)
    fd.close()
