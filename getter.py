import sys
import os

import urllib

from HTMLParser import HTMLParser

cur = ""

class my_parser(HTMLParser):
	def handle_starttag(self, tag, attrs):
		global cur
		print("CWD: " + str(os.getcwd()))
		print("Tag: " + str(tag))
		print("Attributes: " + str(attrs))
		if tag == "a":
			#print("Tag: " + str(tag))
			#print("Attributes: " + str(attrs))
			href = attrs[0][1]
			if(href[0] == '/' or href[0] == '?'): #query string or parent directory will
				print("Not useful reference")
			elif(href[-1] == '/'): #folder we need to go deeper
				if os.path.exists(cur + href):
					print("Directory Exists")
				else:
					print("Making dir " + href)
					os.mkdir(href[:-1])
				thisDir = cur
				cur = cur + href
				get_files(sys.argv[1] + cur)
				cur = thisDir
			else:
				print("Getting File " + href)
				with open(cur + href, 'wb') as f:
					file = urllib.urlopen(sys.argv[1] + cur + href).read()
					f.write(file)
					f.close()

def get_files(cd): #go through and download all files recursivly call for files
	file = urllib.urlopen(cd).read()
	parser = my_parser()
	parser.feed(file)


if len(sys.argv) > 1:
	if sys.argv[1][-1] != '/':
		sys.argv[1] = sys.argv[1] + '/'
	if sys.argv[2]:
		if os.path.exists(sys.argv[2]):
			print("Directory Exists")
		else:
			print("Making dir " + sys.argv[2])
			os.makedirs(sys.argv[2])
		os.chdir(sys.argv[2])
	get_files(sys.argv[1])
else:
	print("Usage: python getter.py [Required: URL] [Optional: Destination Folder default current]")
