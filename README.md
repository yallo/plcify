# plcify
A Command line utility to execute packet loss concealment algorithm on wavs 

# Usage 
```
./plcify [input_path] [output_path]
```
Note that the input file must be a 8khz 16 bit PCM wav file. A file in an identical format will be created at the output path.

# Installation
Install the libraries included in the `3rdparty` directoy.
(./configure && make && make install)

spandsp's `configure` should be added the `--enable-builtin-tiff` flag to use the included version of tiff.


