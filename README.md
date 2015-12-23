# plcify
A Command line utility to execute packet loss concealment algorithm on wavs 

# Usage 
```
./plcify [input_path] [output_path]
```
Note that the input file must be a 8khz 16 bit PCM wav file. A file in an identical format will be created at the output path.

# Installation
## Mac OSX
### Installing libsndfile
- `cd` into the `3rdparty/libsndfile-1.0.26` folder.
- Run `./configure && make && sudo make install`

### Installing spandsp
- `cd` into `3rdparty/spandsp-0.0.6`
- Run `./configure --enable-builtin-tiff && make && sudo make install`

## Linux
### Installing libsndfile
- `cd` into the `3rdparty/libsndfile-1.0.26` folder.
- Run `./configure && make && sudo make install`

### Installing spandsp
- `cd` into `3rdparty/spandsp-0.0-1.6`
- Run `./configure && make && sudo make install`