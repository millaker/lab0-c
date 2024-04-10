import numpy as np
import matplotlib.pyplot as plt

num = []
double_sqrt = []
fix_sqrt = []
double_log = []
fix_log = []
double_log10 = []
fix_log10 = []

with open('fix.dat', 'r') as file:
    for line in file:
        fields = line.split()
        num.append(float(fields[0]))
        double_sqrt.append(float(fields[2]))
        fix_sqrt.append(float(fields[3]))
        double_log.append(float(fields[4]))
        fix_log.append(float(fields[5]))
        double_log10.append(float(fields[6]))
        fix_log10.append(float(fields[7]))

num = np.asarray(num, np.float64)
double_sqrt = np.asarray(double_sqrt, np.float64)
fix_sqrt = np.asarray(fix_sqrt, np.float64)

double_log = np.asarray(double_log, np.float64)
fix_log = np.asarray(fix_log, np.float64)

double_log10 = np.asarray(double_log10, np.float64)
fix_log10 = np.asarray(fix_log10, np.float64)

plt.figure()
plt.plot(num, double_sqrt, label='Double')
plt.plot(num, fix_sqrt, label='Fixed')

plt.xlabel("num")
plt.ylabel("sqrt")
plt.legend()
plt.savefig("dvf_sqrt.png")

plt.figure()
plt.plot(num[1:], double_log[1:], label='Double')
plt.plot(num[1:], fix_log[1:], label='Fixed')

plt.xlabel("num")
plt.ylabel("log2")
plt.legend()
plt.savefig("dvf_log.png")

plt.figure()
plt.plot(num[1:], double_log10[1:], label='Double')
plt.plot(num[1:], fix_log10[1:], label='Fixed')

plt.xlabel("num")
plt.ylabel("log10")
plt.legend()
plt.savefig("dvf_log10.png")