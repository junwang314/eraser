import sys, os
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
	plt.figure()
	plt.hist(size, 10, color='b', alpha=0.5);
	plt.title("memcpy")

def info_leak(filepath):
	leak = 0
	with open(filepath, "rb") as f:
		byte = f.read(1)
		while byte:
			#print byte, ord(byte)
			if ord(byte) != 6:
				leak += 1
			byte = f.read(1)
	statinfo = os.stat(filepath)
	print("file size: %d"%statinfo.st_size)
	print("info leak: %d"%leak)

if __name__ == '__main__':
	profile_malloc(sys.argv[1])
	profile_memcpy(sys.argv[1])
	plt.show()
	#info_leak("/home/jun/src/ArbiterThreadApp/download/webserver-1.2.2/cherokee/leak.log")
	#info_leak("/home/jun/src/ArbiterThreadApp/download/webserver-1.2.2/cherokee/leak-eraser.log")
