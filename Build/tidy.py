#! /bin/env python

"""
Convert all source file from Windows to Uinx line endings
"""

# http://groups.google.com/group/comp.lang.python/browse_thread/thread/c4b63debe91d51c7?pli=1
def dos2unix(file):
	import os
	text = open(file, 'rb').read().replace('\r\n', '\n')
	open(file, 'wb').write(text) 
	print file

def listFiles(rootdir):
	import os, glob
	for dir, subFolders, files in os.walk(rootdir):
		for file in glob.glob(dir + '/*.h'):
			dos2unix(file)
		for file in glob.glob(dir + '/*.cpp'):
			dos2unix(file)

def main():
	dirs = ["../MCD/", "../Test"]
	for dir in dirs:
		listFiles(dir);

try:
	main()
except KeyboardInterrupt:
	pass 