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
	for dataset in {"uniform3", "uniform5", "uniform2"}:
		for fanout in {"50", "100", "250", "1000"}:

			w.write("=================================== {0} {1} ===================================\n".format(dataset, fanout))

			for tree in {"rtree", "rplustree", "nirtree"}:
				for run in {"a", "b", "c", "d", "e"}:
					print("Processing datafile {0}.{1}.{2}.{3}.out".format(tree, dataset, fanout, run))

					with open("{0}/{1}.{2}.{3}.{4}.out".format(dataPath, tree, dataset, fanout, run)) as f:
						for line in f:
							if re.match("^Avg time to insert", line):
								insert = insert + line.split(": ")[1].strip()[:-1] + "\t"
							elif re.match("^Avg time to search", line):
								search = search + line.split(": ")[1].strip()[:-1] + "\t"
							elif re.match("^Avg time to range search", line):
								rsearch = rsearch + line.split(": ")[1].strip()[:-1] + "\t"
							elif re.match("^Avg time to delete", line):
								delete = delete + line.split(": ")[1].strip()[:-1] + "\t"
							if re.match("^Memory Usage", line):
								mem = mem + line.split(": ")[1].strip()[:-1] + "\t"
						# end for
					# end with
				# end for

				w.write(tree)
				w.write('\n')
				w.write(insert[:-1])
				w.write('\n')
				w.write(search[:-1])
				w.write('\n')
				w.write(rsearch[:-1])
				w.write('\n')
				w.write(delete[:-1])
				w.write('\n')
				w.write(mem[:-1])
				w.write("\n\n")
				w.flush()

				insert = ""
				search = ""
				rsearch = ""
				delete = ""
				mem = ""

			# end for
		# end for
	# end for
# end with

