# Directory-Analyzer

A program that analyzes a directory and provides statistics such as the largest file, number of files and directories, most common words in text files, largest images, and vacant directories. It efficiently traverses the directory tree and processes file contents using system calls and C++ standard library functions. Watch a GIF of me interacting with it below!

[insert gif]

## Features
- **Largest File Detection**: Identifies and reports the largest file in the directory, including its path and size.
- **Total File and Directory Count**: Computes the number of files and directories recursively within the given directory.
- **Total File Size Calculation**: Aggregates the size of all files in the directory.
- **Most Common Words** in `.txt` Files: Words are defined as sequences of at least 5 alphabetic, case-insensitive characters. Sorted by frequency in descending order (ties broken alphabetically).
- **Largest Images Detection**: Uses `identify` to detect image dimensions, and returns the top `N` largest images by pixel count, sorted in decending order (ties broken alphabetically).
- **Vacant Directory Identification**: A vacant directory contains no files, even recursively; reports only top-level vacant directories (subdirectories of already vacant directories are excluded). Returned in alphabetical ordewr.
- **Efficient System Calls**: Minimizes the number of `stat()`, `opendir()`, `readdir()`, `fopen()`, and `popen()` system calls for optimized file traversal and processing.

## Usage / Limitations
### Running the Directory Analyzer:
- `N` specifies how many of the most common words and largest images to return.
- `<directory_name>`
- Example usage (see the `usage()` function inside of `main.cpp` for more help): `./analyzeDir 5 ./test11` will return the 5 most common words and 5 largest images in directory `test11`, along with other stats.

### Limitations:
- Assumes that none of the file names nor directory names contain spaces or quotations.
- Assumes each file path contains less than 4096 characters.
- If multiple words have the same number of occurrences, or multiple images have the same number of pixels, they are returned in alphabetical order.
- Top-level vacant directories are returned in alphabetical order.
- Considers all files as potential images, regardless of their extension.
- Considers only files with the `.txt` extension when calculating the most common words.

## If you want to startup the project on your local machine:
1. Download the code as a ZIP or clone the repository:

  `git clone https://github.com/yourusername/Directory-Analyzer.git`
3. 


