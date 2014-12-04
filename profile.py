import sys, os, re
import numpy as np
import matplotlib.pyplot as plt

def profile_malloc(path):
	filepath = path+'/malloc.log'
	size=[]
	with open(filepath) as f:
		for line in f:
			if line != '\n':
				size.append(int(line))
	size=np.array(size)
	print("# malloc(): %d"%len(size))
	print("max (bytes): %d"%size.max())
	print("min (bytes): %d"%size.min())
	print("mean (bytes): %d"%size.mean())
	plt.figure()
	plt.hist(size, 10, color='r', alpha=0.5);
	plt.title("malloc")

def profile_memcpy(path):
	filepath = path+'/memcpy.log'
	size=[]
	with open(filepath) as f:
		for line in f:
			if line != '\n':
			    size.append(int(line))
	size=np.array(size)
	print("# memcpy(): %d"%len(size))
	print("max (bytes): %d"%size.max())
	print("min (bytes): %d"%size.min())
	print("mean (bytes): %d"%size.mean())
	print("total (bytes): %d"%size.sum())
	plt.figure()
	plt.hist(size, 10, color='b', alpha=0.5);
	plt.title("memcpy")

def info_leak_metric_1(filepath):
	print("----------metric 1----------")
	num_00 = 0
	num_06 = 0
	with open(filepath, "rb") as f:
		buf = f.read(4096)
		while buf:
			for byte in buf:
				#print byte, ord(byte)
				if ord(byte) == 0:
					num_00 += 1
				elif ord(byte) == 6:
					num_06 += 1
			buf = f.read(4096)
	statinfo = os.stat(filepath)
	print("file size: %d"%statinfo.st_size)
	print("0x00 size: %d"%num_00)
	print("0x06 size: %d"%num_06)
	print("info leak ratio (only consider 0x06): %.1f%%"%(100-num_06*100.0/statinfo.st_size))
	print("info leak ratio (consider 0x00 and 0x06): %.1f%%"%(100-(num_00+num_06)*100.0/statinfo.st_size))

	#size = statinfo.st_size
	#os.system("gzip -c -f "+filepath+" >/tmp/leak.log.gz")
	#statinfo = os.stat("/tmp/leak.log.gz")
	#print(".gz file size: %d (compress ratio %.1f%%)"%(statinfo.st_size, statinfo.st_size*100.0/size))

	#os.system("bzip2 -c -f "+filepath+">/tmp/leak.log.bz2")
	#statinfo = os.stat("/tmp/leak.log.bz2")
	#print(".bz2 file size: %d (compress ratio %.1f%%)"%(statinfo.st_size, statinfo.st_size*100.0/size))

	#os.system("xz -c -f "+filepath+">/tmp/leak.log.xz")
	#statinfo = os.stat("/tmp/leak.log.xz")
	#print(".xz file size: %d (compress ratio %.1f%%)"%(statinfo.st_size, statinfo.st_size*100.0/size))

def info_leak_metric_2(path):
	'''
	firstname=Great1028330&lastname=User1028330&nickname=user1028330&email=Great1028330.User1028330%40rubis.com&password=password1028330
	'''
	print("----------metric 2----------")
	def find_leak(filepath):
		leak = []
		with open(filepath, "r") as f:
			for line in f:
				#res = re.findall("Great[0-9]+\.User[0-9]+%40rubis\.com", line)	
				res = re.findall("password[0-9]+|user[0-9]+|Great[0-9]+\.User[0-9]+%40rubis\.com|User[0-9]+|Great[0-9]+", line)	
				leak += res
		return leak

	def retrieve_ids(filepath):
		ids = {}
		with open(filepath, "r") as f:
			for line in f:
				ids[line.strip()] = True
		return ids
	
	#def rm_invalid_userid(before, filepath):
	#	after = []
	#	ids = {}
	#	with open(filepath, "r") as f:
	#		for line in f:
	#			ids[line.strip()] = True
	#	for item in before:
	#		res = re.findall("[0-9]+", item)
	#		valid = True
	#		for uid in res:
	#			if not ids.has_key(uid):
	#				valid = False
	#				break;
	#		if valid:
	#			after.append(item)
	#	return after
	
	def search_leftover(leak, ids):
		leftover = []
		credential = []
		passwd = {}
		userid = {}
		for item in leak:
			res = re.findall("[0-9]+", item)
			for uid in res:
				if ids.has_key(uid):
					leftover.append(item)
					if item == "password"+uid:
						passwd[uid] = True
						if userid.has_key(uid):
							credential.append(("user"+uid, item))
					elif item == "user"+uid:
						userid[uid] = True
						if passwd.has_key(uid):
							credential.append((item, "password"+uid))
		return (leftover, credential)

	leak = find_leak(path+"/leak-2.log")
	ids = retrieve_ids("/tmp/userid.txt.1")
	(leftover, credential) = search_leftover(leak, ids)
	print("sensitive data leftover (%d)"%(len(leftover)))
	print("credential leftover (%d)"%(len(credential)))
		

def info_leak(path):
	#info_leak_metric_1(path+"/leak.log.1")
	info_leak_metric_1(path+"/leak-0.log")
	info_leak_metric_2(path)

if __name__ == '__main__':
	#profile_malloc(sys.argv[1])
	#profile_memcpy(sys.argv[1])
	#plt.show()
	#info_leak("/home/jun/src/ArbiterThreadApp/download/webserver-1.2.2/cherokee/leak.log")
	#info_leak("/var/log/apache2/leak.log")
	#info_leak("/home/jun/src/ArbiterThreadApp/download/webserver-1.2.2/cherokee/leak-eraser.log")
	info_leak("/var/log/apache2/")
