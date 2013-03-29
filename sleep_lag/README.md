# Sleep lag test

This toy problem is how we are debugging odd lag spikes when sleeping on EC2 virtual machines.

Build:

```
make
```

Run:

```
./lag > lag.csv
```

Plot:

```
./plot.py lag.csv
```

The plotting needs matplotlib.

You can alter the number of iterations by editing the `N` variable in `lag.cc`.
