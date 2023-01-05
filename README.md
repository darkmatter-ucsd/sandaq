# sandaq
A C++ based data processor for the DAW firmware on the CAEN V1725 digitizer.
`sandaw` is the processor which takes the raw binary files from the output of the V1725 digitizer and makes `peaks` and `events` (events are yet to be implemented). `sandawpy` is the interface with python that loads the processed `peaks` and `events` into numpy structured arrays.

## Installation
Requirements:
```
ROOT
C++17
cmake
```

To install, go to the `sandaw` directory, then type:
```
mkdir build
cd build
cmake ..
make
```

## Running `sandaw`

To run sandaw, do:

`./process -c [CONFIG FILE] -f [DATA FILE FROM V1725 OUTPUT] -p [SAVE DIRECTORY] -r [RUN ID] -h [HITS OUTPUT ROOT FILE] -v [DEBUG]`

Flags `-c` and `-f` are required. An example config file can be found in `sandaw/config`. If the save directory (`-p`) is specified, so too must the run-ID. The processed files will be saved in the save directory with the name `{peaks or events}_{run-ID}.bin`. Hits are basically just a decoded and restructured version of the raw binary files. They can be saved into a ROOT file by specifying the `-h` flag.

### Example
From the `build` directory, run:

`./process -f ../example_data/rawdata/long_tail_20221114T184450.bin -c ../config/config.ini -p ../example_data/
processed/ -r 1234`
