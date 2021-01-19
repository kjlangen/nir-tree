import sys
import re

memory = []
splits = 0
shrinks = 0
height = []
size = []
polygonSize = []
branchFactor = []
branchesSearched = []

# Fill the statistics arrays
with open(sys.argv[1], 'r') as debugLog:
	for line in debugLog:
		if re.match("^MEM [0-9]+$", line):
			memory.append(int(line.strip().split(' ')[1]))
		elif re.match("SPLIT", line):
			splits = splits + 1
		elif re.match("SHRINK", line):
			shrinks = shrinks + 1
		elif re.match("^HEIGHT [0-9]+$", line):
			height.append(int(line.strip().split(' ')[1]))
		elif re.match("^SIZE [0-9]+$", line):
			size.append(int(line.strip().split(' ')[1]))
		elif re.match("^PSIZE [0-9]+$", line):
			polygonSize.append(int(line.strip().split(' ')[1]))
		elif re.match("^BRANCH [0-9]+$", line):
			branchFactor.append(int(line.strip().split(' ')[1]))
		elif re.match("^BRANCH SEARCH [0-9]+$", line):
			branchesSearched.append(int(line.strip().split(' ')[2]))


print("================== Statistics Counters ==================")

# Branches Searched
branchSum = 0;
for branchEntry in branchesSearched:
	branchSum = branchSum + branchEntry

branchAvg = branchSum / (max(len(branchesSearched), 1))

print("Average branches searched = {0}".format(branchAvg))

# Memory
memMax = 0
memMin = 2**64

for memEntry in memory:
	if memEntry > memMax:
		memMax = memEntry

	if memEntry < memMin:
		memMin = memEntry

print("Maximum memory usage = {0}b".format(memMax))
print("Minimum memory usage = {0}b".format(memMin))
print("Initial memory usage = {0}b".format(memory[0]))
print("Final memory usage = {0}b".format(memory[len(memory) - 1]))
print("\n")

# Splits
print("Number of splits induced = {0}".format(splits))

# Shrinks
print("Number of shrinks induced = {0}".format(shrinks))
print("\n")

# Height
heightMax = 0
heightMin = 2**64
heightAvg = 0;

for heightEntry in height:
	if heightEntry > heightMax:
		heightMax = heightEntry

	if heightEntry < heightMin:
		heightMin = heightEntry

	heightAvg = heightAvg + heightEntry

heightAvg = heightAvg / (max(len(height), 1))

print("Maximum tree height = {0}".format(heightMax))
print("Minimum tree height = {0}".format(heightMin))
print("Average tree height = {0}".format(heightAvg))
print("\n")

# Size of Tree in Branches
treeSize = size[0]
solitaryBranchesSize = size[1]

print("Tree size = {0}".format(treeSize))
print("Solitary branches size = {0}".format(solitaryBranchesSize))
print("\n")

# Polygon size
polyMax = 0
polyMin = 2**64
polyAvg = 0;

for polyEntry in polygonSize:
	if polyEntry > polyMax:
		polyMax = polyEntry

	if polyEntry < polyMin:
		polyMin = polyEntry

	polyAvg = polyAvg + polyEntry

polyAvg = polyAvg / (max(len(polygonSize), 1))

print("Maximum polygon size = {0}".format(polyMax))
print("Minimum polygon size = {0}".format(polyMin))
print("Average polygon size = {0}".format(polyAvg))
print("\n")

# Branch factor
branchMax = 0
branchMin = 2**64
branchAvg = 0;
branchHist = dict()

for x in range(0, 57):
	branchHist[x] = 0

for branchEntry in branchFactor:
	branchHist[branchEntry] = branchHist[branchEntry] + 1

	if branchEntry > branchMax:
		branchMax = branchEntry

	if branchEntry < branchMin:
		branchMin = branchEntry
	
	branchAvg = branchAvg + branchEntry

branchAvg = branchAvg / (max(len(branchFactor), 1))

print("Maximum branch factor = {0}".format(branchMax))
print("Minimum branch factor = {0}".format(branchMin))
print("Average branch factor = {0}".format(branchAvg))
print("Histogram = {0}".format(branchHist))
