#######################################################################
## STRIPE PROFILE #####################################################

import matplotlib.pyplot as plt
import numpy as np

labels = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512]

filename = "/mnt/results/stripe_profile/{}kb_seq"
data_list = []

for label in labels:
    file = open(filename.format(label), 'r')
    data = []
    for line in file:
        data.append(int(line))
    data = np.array(data)
    data.shape = (-1, 1)
    data_list.append(data)

labels_new = map(lambda l: "{}kb_seq".format(l), labels)

fig, ax = plt.subplots()
bp = ax.boxplot(data_list, notch=1, sym='+', patch_artist=True)
for element in ['boxes', 'whiskers', 'fliers', 'means', 'medians', 'caps']:
    plt.setp(bp[element], color='black')

# CHANGE THIS COLOR
# Some possible colors are:
# plum
# peachpuff
# lightsteelblue
# palegreen
for patch in bp['boxes']:
    patch.set(facecolor="palegreen")

ax.set_xticklabels(labels_new, fontsize=16, fontweight='medium', fontname='serif', rotation=45)
for tick in ax.yaxis.get_major_ticks():
    tick.label.set_fontsize(16)
    tick.label.set_fontweight('medium')
    tick.label.set_fontname('serif')

ax.set_xlabel("File name", fontsize=16, fontweight='semibold', fontname='serif')
ax.set_ylabel("Read Latency ($\mu$s)", fontsize=16, fontweight='semibold', fontname='serif')
fig.set_size_inches(9, 4.5)
# fig.subplots_adjust(left=0.11, bottom=0.31, right=0.98, top=0.98)

# Change ylim if needed
# plt.ylim(200, 1500) # Samsung
plt.show()

#######################################################################
## LOCATION PROFILE ###################################################

import matplotlib.pyplot as plt
import numpy as np

labels = range(32)

# CHANGE THIS BASED ON THE CHUNK SIZE YOU ARE VISUALIZING FOR
filename = "/mnt/results/loc_profile/8kb.txt"
data_list = []

f = open(filename, 'r')
for line in f:
    data_list.append(float(line))

labels = []
for i in range(32):
    labels.append("{}kb".format(4*i))

plt.plot(range(32), data_list, 'bo-')
plt.set_xticklabels(labels)

plt.xlabel("Offset Group")
plt.ylabel("Avg. Latency (us)")
plt.show()

