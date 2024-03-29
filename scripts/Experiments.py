import os

execPath = "/home/kjlangen/nir-tree/bin/main"
dataPath = "/home/kjlangen/nir-tree/data"
benchmarks = [0, 1]
sizes = [10000000, 100000000, 230000000]
fanouts = [25, 250, 1000]
seeds = [3141, 271828, 138064, 141421]
dataFileNumber = 0

for benchmark in benchmarks:
	for size in sizes:
		for fanout in fanouts:
			for seed in seeds:
				os.system("{0} -t 1 -m {1} -n {2} -a {3} -b {4} -s {5} > {6}/rplus.result.{7}.out".format(execPath, benchmark, size, fanout, fanout * 2, seed, dataPath, dataFileNumber))
				os.system("{0} -t 2 -m {1} -n {2} -a {3} -b {4} -s {5} > {6}/nirplus.result.{7}.out".format(execPath, benchmark, size, fanout, fanout * 2, seed, dataPath, dataFileNumber))
				dataFileNumber = dataFileNumber + 1
