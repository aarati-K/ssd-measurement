import matplotlib.pyplot as plt
import numpy as np

labels = ['4kb' ,'8kb', '16kb', '32kb', '64kb', '128kb', '256kb', '1mb']
# labels_upper = []
# for label in labels:
# 	labels_upper.append(label.upper())

filename = "/mnt/hdd/record/{}_seq_read.txt"
data_list_old = []

for label in labels:
	file = open(filename.format(label), 'r')
	data = []
	for line in file:
		data.append(float(line))

	data = np.array(data)
	data.shape = (-1, 1)
	data_list_old.append(data)

# fig, ax = plt.subplots()
# ax.boxplot(data_list_old, notch=1, sym='+', labels=labels_upper)
# ax.set_xlabel('Chunk Size', fontsize=14)
# ax.set_ylabel('Bandwidth MB/s', fontsize=14)
# fig.show()


filename = "/mnt/hdd/record/{}_seq_read_1.txt"
data_list = []
box_colors = ['peachpuff', 'plum']

for label in labels:
	file = open(filename.format(label), 'r')
	data = []
	for line in file:
		data.append(float(line))

	data = np.array(data)
	data.shape = (-1, 1)
	data_list.append(data)

# Interleave the data
data_list_combined = []
i = 0;
labels_used = []
for label in labels:
	data_list_combined.append(data_list_old[i])
	labels_used.append("{}-seq".format(label.upper()))
	data_list_combined.append(data_list[i])
	labels_used.append("{}-rand".format(label.upper()))
	# Increment
	i+=1

fig, ax = plt.subplots()
bp = ax.boxplot(data_list_combined, notch=1, sym='+', patch_artist=True)

for element in ['boxes', 'whiskers', 'fliers', 'means', 'medians', 'caps']:
	plt.setp(bp[element], color='black')

i = 0
for patch in bp['boxes']:
	patch.set(facecolor=box_colors[i%2])
	i += 1

ax.set_xticklabels(labels_used, fontsize=10, rotation=55)
ax.set_xlabel("File name", fontsize=14)
ax.set_ylabel("Sequential Read Bandwidth MB/s", fontsize=14)
fig.set_size_inches(12, 6)
fig.subplots_adjust(left=0.08, bottom=0.20, right=0.98, top=0.98)
fig.show()

##################################################

import matplotlib.pyplot as plt
import numpy as np

# filename = "/mnt/hdd/record/latency_at_offsets/{}.txt"
filename = "/mnt/hdd/record/garbage_collection/after/{}.txt"
labels = range(16)
data_list = []

for label in labels:
	file = open(filename.format(label), 'r')
	data = []
	for line in file:
		data.append(float(line))

	data = np.array(data)
	data.shape = (-1, 1)
	data_list.append(data)

fig, ax = plt.subplots()
bp = ax.boxplot(data_list, notch=1, sym='+', patch_artist=True)

for element in ['boxes', 'whiskers', 'fliers', 'means', 'medians', 'caps']:
	plt.setp(bp[element], color='black')

for patch in bp['boxes']:
	patch.set(facecolor="peachpuff")

labels_used = [0]
for label in labels[1:]:
	labels_used.append(str(label*4) + "KB")

ax.set_xticklabels(labels_used, fontsize=10, rotation=55)
ax.set_xlabel("Offset in a 64KB block", fontsize=14)
ax.set_ylabel("Latency ($\mu$s)", fontsize=14)
ax.tick_params(axis='both', which='major', pad=20)
# plt.ylim(120, 550)
plt.ylim(120, 400)
# fig.set_size_inches(12, 6)
fig.set_size_inches(12, 4.5)
fig.subplots_adjust(left=0.08, bottom=0.20, right=0.98, top=0.98)
fig.show()

##################################################

import matplotlib.pyplot as plt
import numpy as np

filename = "/mnt/hdd/record/latency_at_offsets_32/{}.txt"
labels = range(16)
data_list = []

for label in labels:
	file = open(filename.format(label), 'r')
	data = []
	for line in file:
		data.append(float(line))

	data = np.array(data)
	data.shape = (-1, 1)
	data_list.append(data)

fig, ax = plt.subplots()
bp = ax.boxplot(data_list, notch=1, sym='+', patch_artist=True)

for element in ['boxes', 'whiskers', 'fliers', 'means', 'medians', 'caps']:
	plt.setp(bp[element], color='black')

for patch in bp['boxes']:
	patch.set(facecolor="peachpuff")

labels_used = [0]
for label in labels[1:]:
	labels_used.append(str(label*4) + "KB")

ax.set_xticklabels(labels_used, fontsize=10, rotation=55)
ax.set_xlabel("Offset in a 64KB block", fontsize=14)
ax.set_ylabel("Latency ($\mu$s)", fontsize=14)
ax.tick_params(axis='both', which='major', pad=20)
plt.ylim(0, 300)
fig.set_size_inches(12, 6)
fig.subplots_adjust(left=0.08, bottom=0.20, right=0.98, top=0.98)
fig.show()

######################################################

import numpy as np
import matplotlib.pyplot as plt

dirname = "/mnt/hdd/record/updating_file/"
filenames = ["overwritten_1_latency.txt", "overwritten_2_latency.txt", "overwritten_4_latency.txt", "skipped_latency.txt"]
data_list = []
for name in filenames:
	file = open("{}{}".format(dirname, name), "r")
	data = []
	for line in file:
		data.append(int(line))
	data = np.array(data)
	data.shape = (-1, 1)
	data_list.append(data)

fig, ax = plt.subplots()
colors = ['lightsteelblue', 'salmon', 'plum', 'peachpuff']
bp = ax.boxplot(data_list, notch=1, sym='+', patch_artist=True)

for element in ['boxes', 'whiskers', 'fliers', 'means', 'medians', 'caps']:
	plt.setp(bp[element], color='black')

i = 0
for patch in bp['boxes']:
	patch.set(facecolor=colors[i])
	i+=1

# labels = ['64KB', '32*2KB', '16*4KB', 'original']
labels = ['(a)', '(b)', '(c)', '(d)']
ax.set_xticklabels(labels, fontsize=10)
# ax.set_xlabel("Manner of writing", fontsize=14)
ax.set_ylabel("Latency ($\mu$s)", fontsize=14)
ax.tick_params(axis='both', which='major', pad=20)
plt.ylim(100, 550)
fig.set_size_inches(7.5, 3.5)
fig.subplots_adjust(left=0.15, bottom=0.15, right=0.98, top=0.98)

fig.text(0.50, 0.92, "Single write of 64KB", backgroundcolor=colors[0], weight='roman', color='black', size='large')
fig.text(0.75, 0.92, "Two writes of 32KB", backgroundcolor=colors[1], weight='roman', color='black', size='large')
fig.text(0.50, 0.82, "Four writes of 16KB", backgroundcolor=colors[2], weight='roman', color='black', size='large')
fig.text(0.75, 0.82, "Not overwritten", backgroundcolor=colors[3], weight='roman', color='black', size='large')

fig.show()

######################################################

import numpy as np
import matplotlib.pyplot as plt

filename = "/mnt/hdd/record/garbage_collection/latency_16kb_128_write.txt"
file = open(filename, 'r')
data = []
for line in file:
	data.append(int(line))

fig, ax = plt.subplots()
ax.set_xlabel("Time $\longrightarrow$", fontsize=18)
ax.set_ylabel("Latency", fontsize=18)
plt.plot(data)
fig.show()

######################################################

import numpy as np
import matplotlib.pyplot as plt

node_size = '32kb'

write_latency_bad = []
read_latency_bad = []

write_latency_good = []
read_latency_good = []

filename = "/mnt/hdd/record/sqlite_cpy/latency_{}_bad_traditional.txt".format(node_size)
file = open(filename, 'r')
i = 1
for line in file:
	if i%2 == 1:
		write_latency_bad.append(float(line)/1000000)
	else:
		read_latency_bad.append(float(line)/1000000)
	i += 1
file.close()

filename = "/mnt/hdd/record/sqlite_cpy/latency_{}_good_traditional.txt".format(node_size)
file = open(filename, 'r')
i = 1
for line in file:
	if i%2 == 1:
		write_latency_good.append(float(line)/1000000)
	else:
		read_latency_good.append(float(line)/1000000)
	i+= 1
file.close()

read_throughput_good = []
read_throughput_good_cumulative = []

read_throughput_bad = []
read_throughput_bad_cumulative = []

num_inserts = 50000
num_inserts_cumulative = 0
read_latency_cumulative = 0
for lat in read_latency_good:
	read_latency_cumulative += lat
	num_inserts_cumulative += num_inserts
	read_throughput_good.append(num_inserts/lat)
	read_throughput_good_cumulative.append(num_inserts_cumulative/read_latency_cumulative)

num_inserts_cumulative = 0
read_latency_cumulative = 0
for lat in read_latency_bad:
	read_latency_cumulative += lat
	num_inserts_cumulative += num_inserts
	read_throughput_bad.append(num_inserts/lat)
	read_throughput_bad_cumulative.append(num_inserts_cumulative/read_latency_cumulative)

x_axis = range(1, 101)
plt.plot(x_axis, read_throughput_good, 'b+', markersize=4)
plt.plot(x_axis, read_throughput_good_cumulative, 'dodgerblue', linewidth=2)

plt.plot(x_axis, read_throughput_bad, 'g+', markersize=4)
plt.plot(x_axis, read_throughput_bad_cumulative, 'mediumseagreen', linewidth=2)

plt.show()