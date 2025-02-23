# Directory-Analyzer

A program that analyzes a directory and provides statistics such as the largest file, number of files and directories, most common words in text files, largest images, and vacant directories. It efficiently traverses the directory tree and processes file contents using system calls and C++ standard library functions. Watch a GIF of me interacting with it below!

![run](https://github.com/user-attachments/assets/f6029110-9bf8-4904-80a2-0189a1564c9d)
&nbsp;

## Features
- **Largest File Detection**: Identifies and reports the largest file in the directory, including its path and size.
- **Total File and Directory Count**: Computes the number of files and directories recursively within the given directory.
- **Total File Size Calculation**: Aggregates the size of all files in the directory.
- **Most Common Words** in `.txt` Files: Words are defined as sequences of at least 5 alphabetic, case-insensitive characters. Sorted by frequency in descending order (ties broken alphabetically).
- **Largest Images Detection**: Uses `identify` to detect image dimensions, and returns the top `N` largest images by pixel count, sorted in decending order (ties broken alphabetically).
- **Vacant Directory Identification**: A vacant directory contains no files, even recursively; reports only top-level vacant directories (subdirectories of already vacant directories are excluded). Returned in alphabetical order.
- **Efficient System Calls**: Minimizes the number of `stat()`, `opendir()`, `readdir()`, `fopen()`, and `popen()` system calls for optimized file traversal and processing.

## Usage / Limitations
### Running the Directory Analyzer:
- `N` specifies how many of the most common words and largest images to return.
- `<directory_name>` is the name of the directory to analyze.
- Example usage (see the `usage()` function inside of `main.cpp` for more help):
  
```./analyzeDir 5 ./test11``` 

will return the 5 most common words and 5 largest images in directory `test11`, along with other stats.

### Limitations:
- Assumes that none of the file names nor directory names contain spaces or quotations.
- Assumes each file path contains less than 4096 characters.
- If multiple words have the same number of occurrences, or multiple images have the same number of pixels, they are returned in alphabetical order.
- Top-level vacant directories are returned in alphabetical order.
- Considers all files as potential images, regardless of their extension.
- Considers only files with the `.txt` extension when calculating the most common words.
- Calls to `identify` via `popen()` introduce some overhead especially with many files, as `libc` calls `fork()` twice for each `popen()` system call.
- To benchmark performance, run the program twice to minimize filesystem caching effects:

```
time ./analyzeDir 10 ./test_directory               # first run
time ./analyzeDir 10 ./test_directory               # second run (more accurate)
```

## If you want to startup the project on your local machine:
1. Download the code as a ZIP and unzip it or clone the repository:

  ```git clone https://github.com/yourusername/Directory-Analyzer.git```

**Clone**:
<br></br>
![clone](https://github.com/user-attachments/assets/53934a56-ff5d-4dcf-b857-b751e96d1aaf)
<br></br>
**Download**:
<br></br>
![download](https://github.com/user-attachments/assets/0d2db9c6-0ae6-48d3-92f8-cbbe6ecebdde)
<br></br>
**Unzip**:
<br></br>
![unzip](https://github.com/user-attachments/assets/867d917b-9074-418a-8221-a1d8f33af79d)
<br></br>

2. Open up a terminal, navigating into the repository:

```cd Downloads/Directory-Analyzer[-main]        # -main will only be in the folder name if you downloaded as a ZIP ```
<br></br>
![terminal](https://github.com/user-attachments/assets/beb85031-a09f-4106-84ae-382902b225e8)
&nbsp;

3. Compile the code:

```make```
<br></br>
![compile](https://github.com/user-attachments/assets/332e883e-bdb4-4667-b87b-1d70434b1749)
&nbsp;

4. Run the code:

```./analyzeDir N <directory_name>```

Example:

```./analyzeDir 5 test11```
<br></br>
![run](https://github.com/user-attachments/assets/5e66b187-26e8-4167-9e87-4e4f770b9896)
&nbsp;

## Cleaning Up:

To remove the compiled binary and object files:

```make clean```
<br></br>
![clean](https://github.com/user-attachments/assets/169c9fb3-e545-4ca0-822b-b38d28612b8f)
&nbsp;

This will delete all `.o` files and the `analyzeDir` executable.
