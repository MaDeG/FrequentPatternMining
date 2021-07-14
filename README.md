# Frequent Pattern Mining
This project has been made for the High Performance Computing course (2020/2021) in Università Ca' Foscari of Venice, and focuses on the parallel and sequential C++ implementation of an algorithm to compute Frequent Patterns.

### Structure
The project is organised as follows:

```
FrequentPatternMining
│   README.md 				This file
│   CMakeLists.txt    	
│
└───src                                 Folder containing the project C++ sources
│   │   *.h *.cpp
│   │   ...
└───docs                                Folder containing the Project Report and related documents
│   │   Report.md
│   │   ...
│   
└───datasets				Part of the datasets used to test the code
│   │   ...
│
└───bin                                 Where the compiled binary can be found
	│   FrequentPatternMining
```

### Compile the code
In order to compile the code on a Linux system it is required to have:

 - `cmake` tool version 1.32 or greater.
 - `make` tool recent version.
 - `boost` libraries version 1.40 or greater.
 - `openmp` libraries version 3.0 or greater.

Satisfied the above requirements it will be possible to compile the code using the following commands from the project folder:
```
cmake CMakeLists.txt
make
```
### Run the code

The project can be executed running the command:

```
bin/FrequentPatternMining
```

### Usage

From the help option:

```
./bin/FrequentPatternMining --help
Allowed options:
  -h [ --help ]                Print program usage
  -s [ --supportFraction ] arg Set minimum supportFraction fraction in 
                               percentage for an itemset to be considered 
                               frequent (e.g. 65%), must be a value between 0 
                               excluded and 100 included
  -i [ --input ] arg           Input file where new-line separated transactions
                               will be read
  -t [ --threads ] arg (=0)    Number of threads to use, use 0 to use as many 
                               as the amount of cores, 1 for sequential 
                               execution, or a custom number
  -o [ --skipOutputFrequent ]  Disables the output of the Frequent Itemsets 
                               once computed, only their count will be printed,
                               used during performance evaluation
  -d [ --debug ]
```

### Expected input format
The input files are expected to be composed by list of numbers, separated by a whitespace and by a newline (\n) between lists.

```
1 2 3 4 5
6 7 8
9
1 2 3
...
```

### Problem solved
Given:
- A set a set of transactions `T`
- Over a set of items `L`
- A minumum support fraction `minsup` defined as the fraction of transactions that contain an itemset and expressed in percentage

Output all itemsets (defined as a collection of one or more items) with items in `L` having minimum support value greater than `minsup`

### Execution example

For instance the following command:

```
./bin/FrequentPatternMining --input datasets/chess.dat -s 50 -t 8
```

Computes all the itemsets that have a minimum support greater than 50% and hence all the possible combination of items in `L` that appear in at least half of the input transactions `T`.
