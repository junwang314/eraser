import os
import sys
import subprocess
import signal
import time
import random
import re
import math

n_exp = 10
n_req = 10000
n_concur = 100
#target_addr = "http://localhost/"
target_addr = "http://localhost/test/Yahoo.html"
prefix = "Requests per second:    "


def mean(data, n):
	return sum(data) / n

def stdev(data, n):
	ave = mean(data, n)
	sq_sum = 0
	for i in range(0, n):
		sq_sum += math.pow(data[i] - ave, 2)
	return math.sqrt(sq_sum / n)

# Throughput overhead evaluation

throughput_results = []

for i in range(0, n_exp):
	ab_output = subprocess.check_output(["ab", "-c", str(n_concur), "-n", str(n_req), target_addr])

	throughput = 0.0

	for line in ab_output.splitlines():
		if "Requests per second" in line:
			strs = line.split(" ")
			for item in strs:
				if re.match("^\d+?\.\d+?$", item):
					throughput = float(item)

	assert throughput > 0

	throughput_results.append(throughput)

 

# Memory overhead evaluation

mem_profiler = subprocess.Popen(["python", "mem_profile.py"])

for i in range(0, n_exp):
	ab_output = subprocess.check_output(["ab", "-c", str(n_concur), "-n", str(n_req), target_addr])			

os.kill(mem_profiler.pid, signal.SIGKILL)

max_hwm = 0
for f_name in os.listdir("."):
	if "apache_mem_log_apache2" in f_name:
		f = open(f_name, "r")
		for line in f:
			strs = line.split('\t')
			apache_mem_summaryVmRSS = int(strs[0].split(' ')[0])
			VmHWM = int(strs[1].split(' ')[0])
			if max_hwm < VmHWM:
				max_hwm = VmHWM
		f.close()
		os.rename(f_name, "mem_result/" + f_name)
	elif "apache_mem_summary" in f_name:
		os.rename(f_name, "mem_result/" + f_name)
	
	
	
# Output result

print "throughput mean is " + str(mean(throughput_results, n_exp)) + ", stdev is " + str(stdev(throughput_results, n_exp))
print "Max mem is " + str(max_hwm)

