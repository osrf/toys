#!/usr/bin/env python

import sys

from pylab import *


def main():
    if len(sys.argv) == 1:
        print("Usage: make && ./lag > lag.csv && plot.py lag.csv")
        return
    with open(sys.argv[1], 'r') as f:
        content = f.read()
    dts = []
    cpus = []
    mems = []
    for line in content.splitlines()[1:]:
        packed = line.split(',')
        if len(packed) != 3:
            print(len(packed))
            continue
        dt, cpu, mem = packed
        dts.append(float(dt.strip()))
        cpus.append(float(cpu.strip()))
        mems.append(float(mem.strip())/1e6)
    subplot(311)
    plot(dts)
    title('Time deltas between samples @ 1 per 1ms')
    ylabel('time in milliseconds')
    subplot(312)
    plot(cpus)
    title('CPU usage sampled @ 1 per 10ms')
    ylabel('percentage of cpu usage')
    subplot(313)
    plot(mems)
    title('RAM usage sampled @ 1 per 10ms')
    ylabel('RAM used in Mb')
    show()

if __name__ == '__main__':
    main()
