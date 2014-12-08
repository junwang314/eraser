import os
import time
import threading
import sys

'''
This script logs SPEC benchmark memory usuage by sampling the procfs.

There are still some TODOs left.

Author: Mingyi Zhao, Qiang Zeng
Date: 2014/02/06
'''

#We might need to set max pid to a large value before running this script.

cmd_keyword = "apache2"

spec_pids = []

spec_pid_name = {}

#This file is used to log SPEC related events. We set the buf size to be zero so that it can immediately print out.
f_summary = open('apache_mem_summary_' + str(int(time.time())) + '.txt', 'w', 0)

#Use this dict to access a pid's corresponding log file.
spec_log_file = {}

watch_dog_init = False
watch_dog_ready_to_quit = False

mem_sample_frequency = 0.02

'''
Search the /proc folder and find SPEC ref run related pids.
A tricky thing is that when we have a test run or train run, we should not quit.
'''
def search_spec_pid(thread_start):
    if thread_start:
        threading.Timer(1, search_spec_pid, [True]).start()
    pids = [pid for pid in os.listdir('/proc') if pid.isdigit()]
    global watch_dog_ready_to_quit
    for pid in pids:
        try:
            cmdline = open('/proc/' + str(pid) + '/cmdline', 'r')
            for line in cmdline:
                if cmd_keyword in line:
                    status = open('/proc/' + str(pid) + '/status', 'r')
                    name = "null"
                    for line2 in status:
                        if line2.startswith('Name:'):
                            name = line2.split('\t')[1].strip()
                        elif line2.startswith('Pid:'):
                            bench_pid = line2.split('\t')[1].strip()
                            if bench_pid not in spec_pids:
                                spec_pids.append(bench_pid)
                                f_summary.write("Find new apache process! " + str(line) + ' (' + str(pid) + ') at ' + str(time.ctime()) + '\n')
                                f_log = open("apache_mem_log_" + name + "_" + str(pid) + "_" + str(int(time.time())) + '.txt', 'w')
                                spec_log_file[bench_pid] = f_log
                                spec_pid_name[bench_pid] = name
                                watch_dog_ready_to_quit = False
                                break
                #elif 'run_base_test' in line or 'run_base_train' in line:
                #    f_summary.write('Find SPEC test or train run at ' + time.ctime() + '\n')
                #    watch_dog_ready_to_quit = False
        except IOError:
            f_summary.write("search_spec_pid: cannot open folder " + pid + " at " + time.ctime() + '\n')

'''
Read the folder of a SPEC process and output VmHWM and VmRSS
'''
def log_spec_mem():
    threading.Timer(mem_sample_frequency, log_spec_mem).start()
    for pid in spec_pids:
        try:
            status = open('/proc/' + str(pid) + '/status', 'r')
            VmHWM = 0
            VmRSS = 0
            for line in status:
                if line.startswith('VmHWM:'):
                    VmHWM = line.split('\t')[1].strip()
                elif line.startswith('VmRSS:'):
                    VmRSS = line.split('\t')[1].strip()
            spec_log_file[pid].write(str(VmRSS) + '\t' + str(VmHWM) + '\t' + str(time.ctime()) + '\n');
        except IOError:
            #An IOError will occur if the process is ended. This means that the benchmark run is finished and we can delete the pid.
            spec_pids.remove(pid)
            spec_log_file[pid].close()
            del spec_log_file[pid]
            f_summary.write("log_apache_mem: cannot open folder " + pid + " at " + time.ctime() + '\n')
            #Immediately search for a new SPEC process.
            search_spec_pid(False)


'''
Monitor the /proc folder and quit if we haven't seen a SPEC process for quite a long time.
'''
def quit_watch_dog():
    threading.Timer(3600, quit_watch_dog).start()
    global watch_dog_init
    global watch_dog_ready_to_quit
    f_summary.write("watch dog " + time.ctime() + '\n')
    if watch_dog_init:
        if len(spec_pids) == 0:
            if watch_dog_ready_to_quit:
                #We haven't seen a spec process for quite a long time, so I will say good by!
                f_summary.write("watch dog kills mem_profile at " + time.ctime() + '\n')
                f_summary.close()  #TODO: could there be a conflict between threads?
                os._exit(os.EX_OK)
            else:
                #Give it another chance...
                f_summary.write("watch dog ready to quit at " + time.ctime() + '\n')
                watch_dog_ready_to_quit = True
    else:
				watch_dog_init = True
    

print "start searching for apache pid at " + time.ctime()
print "sample mem at every " + str(mem_sample_frequency) + "s"
search_spec_pid(True)

log_spec_mem()

quit_watch_dog()
