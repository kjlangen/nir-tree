import os
import io
import re

dataPath = "/home/kjlangen/nir-tree/data"

insert = ""
search = ""
rsearch = ""
delete = ""
mem = ""

with open("{0}/analysis.out".format(dataPath), "w") as w:
	for dataFileNumber in range(0, 64):
		print("dataFileNumber {0}".format(dataFileNumber))
	
		with open("{0}/rplus.result.{1}.out".format(dataPath, dataFileNumber)) as f:
			first = True
			lineNumber = 0
			for line in f:
				if lineNumber < 10:
					w.write(line)
					lineNumber = lineNumber + 1
					continue
				elif re.match("^BRANCH", line):
					continue
				elif re.match("^SIZE [0-9]+$", line) and first:
					mem = line.split()[1].strip()
					first = False
				elif re.match("^Avg time to insert", line):
					insert = line.split(':')[1].strip()
				elif re.match("^Avg time to search", line):
					search = line.split(':')[1].strip()
				elif re.match("^Avg time to range search", line):
					rsearch = line.split(':')[1].strip()
				elif re.match("^Avg time to delete", line):
					delete = line.split(':')[1].strip()

			w.write("d = {0}\n".format(int(dataFileNumber / 16)))
			w.write(insert)
			w.write('\n')
			w.write(search)
			w.write('\n')
			w.write(rsearch)
			w.write('\n')
			w.write(delete)
			w.write('\n')
			w.write(mem)
			w.write("\n\n")
			w.flush()

		with open("{0}/nirplus.result.{1}.out".format(dataPath, dataFileNumber)) as f:
			first = True
			lineNumber = 0
			for line in f:
				if re.match("^BRANCH", line):
					continue
				elif re.match("^SIZE [0-9]+$", line) and first:
					mem = line.split()[1].strip()
					first = False
				elif re.match("^Avg time to insert", line):
					insert = line.split(':')[1].strip()
				elif re.match("^Avg time to search", line):
					search = line.split(':')[1].strip()
				elif re.match("^Avg time to range search", line):
					rsearch = line.split(':')[1].strip()
				elif re.match("^Avg time to delete", line):
					delete = line.split(':')[1].strip()

			w.write("d = {0}\n".format(int(dataFileNumber / 16)))
			w.write(insert)
			w.write('\n')
			w.write(search)
			w.write('\n')
			w.write(rsearch)
			w.write('\n')
			w.write(delete)
			w.write('\n')
			w.write(mem)
			w.write("\n")
			w.flush()

