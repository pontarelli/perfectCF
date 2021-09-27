# Perfect Cuckoo filters

This is the repository for the simulator used in the paper "Perfect Cuckoo filters", ACM CoNext 2021
The repository also contains an executable to provide statistics for an LPM table composed by three nodes (as described in section 4.2 of the paper)


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

To collect the stats for the LPM databases run 

```
$ ./LPMstat -L N -f database.txt
```

The executable provides the statistics with a tree composed by a tree with root /24, right node /32, and left node /N 

# Usage
The simulator options can be retrieved running:

```
$ ./perfectCF --help
```
