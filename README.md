# QSComp - QuickSort Compression

QSComp is a novel text file compression utility that explores the effectiveness of sorting the data at certain points, and combines other compression algorithms to achieve efficient compression ratios. The project makes use of various compression techniques including Burrows-Wheeler Transform (BWT), Run-Length Encoding (RLE), dictionary-based compression, and a custom QuickSort transformation.

NOTE: As of now, this compression utility outputs statistics on the various tests (see below), and doesn't yet output thecompressed version of the input file.

## Features

- **Multiple Compression Algorithms**:
  - Burrows-Wheeler Transform (BWT)
  - Run-Length Encoding (RLE)
  - Dictionary-based compression
  - QuickSort transformation
  - Incremental Adaptive Compression (IAC)

- **Flexible Processing Pipeline**: Combine different algorithms to achieve optimal compression for various types of input data

## Building the Project

To build the project, simply run:

    make

This will compile the source files and create the executable `qscomp`.

## Usage

./qscomp <file>

Where `<file>` is the path to the text file you want to compress.

### Example

bash
./qscomp data/sample.txt

## Implementation Details

The project consists of several key components:

- **BWT (Burrows-Wheeler Transform)**: Rearranges character strings to improve compressibility
- **RLE (Run-Length Encoding)**: Compresses sequences of repeated characters
- **Dictionary Compression**: Replaces common patterns with shorter codes
- **QuickSort Transform**: A novel approach to data transformation for compression
- **IAC (Incremental Adaptive Compression)**: Adapts to the data characteristics during compression
- **Prep**: Handles data preparation and preprocessing

## Testing Modes

The project includes several test modes that can be enabled in the main.cpp file:

1. **test1**: BWT followed by QuickSort Transform
2. **test2**: Alternative compression pipeline
3. **test3**: Dictionary-based compression with configurable word length
4. **test4**: Enhanced dictionary compression
5. **test5**: RLE-focused compression
6. **test6**: Combined algorithm approach
7. **test7**: Experimental compression technique

## Performance

The compression ratio varies depending on the input data characteristics. Best results are typically achieved with text files containing repetitive patterns.

## Requirements

- C++17 compatible compiler
- BSD-compatible system (can be modified for other platforms)

## Author

José Antonio Lacsam (2021)

## License

MIT License
