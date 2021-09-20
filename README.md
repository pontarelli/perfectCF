# Perfect Cuckoo filters

This is the repository for the simulator used in the paper "Perfect Cuckoo filters", ACM CoNext 2021

# Getting Started

The simulator has been developed on Ubuntu 20.04. Other distributions or versions may need different steps.

# Building

Run the following command to build everything:

```
$ make
```

# Running
To execute the simulator run 

```
$ ./perfectCF
```

The executable tests the correctness of perfect Cuckoo Filters loading the filter up to 95% and checking the occurence of fale positives against the entire universe of possible keys. When a bijective function is used (CRC or MurMur hash) the PCF has no false positives.


# Usage
The simulator options can be retrieved running:

```
$ ./perfectCF --help
```
