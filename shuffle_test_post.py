from itertools import permutations
import matplotlib.pyplot as plt
import sys

args = sys.argv
if len(args) != 2:
    print(f"{args[0]} takes one argument")
    sys.exit()

infile = args[1]
# Create all possibilities
string = "123"
outfile = infile.split('.')[0]

perm = permutations(string)
perm_list = [''.join(p) for p in perm]

count = {x: 0 for x in perm_list}

with open(infile, 'r') as file:
    for line in file:
        line = line.strip()
        count[line] += 1

labels = list(count.keys())
val = list(count.values())

# Plotting
plt.figure(figsize=(10, 6))
plt.bar(labels, val)
plt.xlabel('Permutations')
plt.ylabel('Count')
plt.title('Count of Permutations')
plt.xticks([])
plt.xlim(-0.5, len(labels)-0.5)
plt.tight_layout()
plt.savefig(outfile + '.png')
