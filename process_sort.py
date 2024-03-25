import sys
import os
from dataclasses import dataclass
import matplotlib.pyplot as plt
import numpy as np

@dataclass
class Stats:
    name: str
    pattern: int
    size: list[int]
    time: list[int]
    cmps: list[int]

def plt_wrap(xticks, yticks, plot_name, file_name):
    ax = plt.gca()
    ax.set_xticks(xticks)
    ax.set_yticks(yticks)
    plt.xticks(rotation=30)
    plt.title(plot_name)
    plt.legend()
    plt.savefig(file_name)
    plt.close()

# Parse log file and return result lists for each type
# Returns a dictionary with type name as key and Stats
def process_file(file_name):
    type = {}
    try:
        with open(file_name, "r") as file:
            for line in file:
                t, pattern, size, time, cmps = line.split()
                if t not in type:
                    type[t] = Stats(t,pattern,[],[],[])
                type[t].size.append(size)
                type[t].time.append(time)
                type[t].cmps.append(cmps)
    except FileNotFoundError:
        print(f"Error: File '{file_name}' not found.")
    except Exception as e:
        print(f"An error occurred while processing '{file_name}': {e}")
    return type
pattern_dict = {1 : 'All random',
                2 : 'Descending',
                3 : 'Ascending',
                4 : 'Ascend_3rand',
                5 : 'Ascend_10_rand',
                6 : 'Ascend_rand1%',
                7 : 'Duplicates', 
                8 : 'All_Equal'}

def __plot(x, y, x_name, y_name, label, plot_name, file_name):
    plt.figure(figsize=(10,6))
    xticks = np.arange(0, 12000, 1000)
    y_max = np.max(y)
    step_size = 100
    while y_max / step_size > 10:
        step_size *= 10
    yticks = np.arange(0, y_max + step_size, step_size)
    plt.scatter(x, y, label=label, marker='+', s=15, alpha=0.8)
    plt.xlabel(x_name)
    plt.ylabel(y_name)
    plt_wrap(xticks, yticks, plot_name, file_name)

def __plot_mixed(x, data, y_name, data_name, plot_name, file_name):
    # Draw mixed figure
    plt.figure(figsize=(10,6))
    max = -1
    for d,n in zip(data,data_name):
        plt.scatter(x, d, label=n, marker='+', s=15, alpha=0.8)
        if np.max(d) > max:
            max = np.max(d)
    xticks = np.arange(0, 12000, 1000)
    y_max = max
    step_size = 100
    while y_max / step_size > 10:
        step_size *= 10
    yticks = np.arange(0, y_max + step_size, step_size)
    # Set labels and title
    plt.xlabel('Size')
    plt.ylabel(y_name)
    plt_wrap(xticks, yticks, plot_name, file_name)

def __plot_aggregate(data, plot_name, file_name):
    plt.figure(figsize=(10,6))
    max = -1
    for size, cmps, _, pattern in data:
        plt.scatter(size, cmps, label=pattern_dict[int(pattern)], marker='+', s=15, alpha=1)
        temp = np.max(cmps)
        if temp > max:
            max = temp
    xticks = np.arange(0, 12000, 1000)
    y_max = max
    step_size = 100
    while y_max / step_size > 10:
        step_size *= 10
    yticks = np.arange(0, y_max + step_size, step_size)
    # Set labels and title
    plt.xlabel('Size')
    plt.ylabel('Comparisons')
    plt_wrap(xticks, yticks, plot_name, file_name)

def plot(result):
    # Iterate through all sorting
    size = 0
    nameL = []
    timeL = []
    cmpsL = [] 
    dataL_dict = {'Tim':[], 'Merge':[]} 
    for d in result:
        data=result[d]
        plt.figure()
        data = result[d]
        pattern = data.pattern
        # Convert to numpy array
        size = np.array(data.size).astype(int)
        time = np.array(data.time).astype(int)
        cmps = np.array(data.cmps).astype(int)
        timeL.append(time)
        cmpsL.append(cmps)
        nameL.append(data.name)
        dataL_dict[d].append((size, cmps, time, pattern))

        __plot(size, time, "Size", "Time", d, \
               f'{data.name}sort {pattern_dict[int(pattern)]} -- Time', \
               prefix + '/../plots/' + f'{data.name}_{pattern}_time.png')
        __plot(size, cmps, "Size", "Comparisons", d, \
               f'{data.name}sort {pattern_dict[int(pattern)]} -- Comparisons', \
               prefix + '/../plots/' + f'{data.name}_{pattern}_cmps.png')
    __plot_mixed(size, timeL, 'Size', nameL, \
                f'Mixed {pattern_dict[int(pattern)]} -- Time', \
                prefix + '/../plots/' + f'Mixed_{pattern}_time.png')
    __plot_mixed(size, cmpsL, 'Size', nameL, \
                f'Mixed {pattern_dict[int(pattern)]} -- Comparisons', \
                prefix + '/../plots/' + f'Mixed_{pattern}_cmps.png')
    return (dataL_dict['Tim'], dataL_dict['Merge'])
    

if __name__ == "__main__":
    if(len(sys.argv) < 2):
        print(f"Usage: python {sys.argv[0]} input_file1 input_file2 ...")
        sys.exit()
    input_files = sys.argv[1:]
    TimAll = []
    MergeAll = []
    for i in input_files:
        prefix = os.path.dirname(i)
        result = process_file(i)
        print(f'Plotting {i}...')
        a,b = plot(result)
        TimAll += a
        MergeAll += b

    print('Plotting aggregate ...')
    __plot_aggregate(TimAll, \
                     f'Timsort all patterns -- Comparisons', \
                     prefix + '/../plots/' + f'Timsort_all_cmps.png')
    __plot_aggregate(MergeAll, \
                     f'Mergesort all patterns -- Comparisons', \
                     prefix + '/../plots/' + f'Mergesort_all_cmps.png')




