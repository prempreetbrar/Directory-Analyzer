#include "analyzeDir.h"

// OS-Specific Includes for error-handling
#ifdef __linux__
    #include <error.h>
#elif defined(__APPLE__) || defined(__MACH__)
    #include <errno.h>
    #include <cstdio>
    #include <cstdlib>
#elif defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <iostream>
#endif

// C Standard Libraries
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// C++ Standard Libraries
#include <unordered_map>
#include <algorithm>
#include <sstream>  
#include <optional>

// define strings as a C string so that we don't need to invoke .c_str when passing it into
// a C system call
constexpr char CURRENT_DIRECTORY[] = ".";
constexpr char PREVIOUS_DIRECTORY[] = "..";
constexpr char PATH_SEPARATOR = '/';
constexpr char NO_PATH[] = "";

constexpr int SYSCALL_SUCCESS = 0;
constexpr int SYSCALL_FAILED = -1;

constexpr int DEFAULT_LARGEST_SIZE = -1;
constexpr int MIN_WORD_SIZE = 5;

// FILE HELPERS
// ===================================================================================================================

/**
 * @brief Prints an error message and exits the program.
 * @param message The error message to display.
 */
void print_error(const std::string &message) {
  #ifdef __linux__
      error(SYSCALL_FAILED, errno, "%s", message.c_str());
  #elif defined(__APPLE__) || defined(__MACH__)
      perror(message.c_str());
      exit(EXIT_FAILURE);
  #elif defined(_WIN32) || defined(_WIN64)
      std::cerr << message << ": Error code " << GetLastError() << std::endl;
      exit(EXIT_FAILURE);
  #endif
}

/**
 * @brief Opens a directory for reading.
 * @param dir_name The path of the directory.
 * @return Pointer to the opened directory (DIR*).
 */
DIR *open_directory(const std::string &dir_name) {
  DIR *dir = opendir(dir_name.c_str());
  if (!dir) print_error("could not opendir " + dir_name);
  return dir;
}

/**
 * @brief Opens a file for reading.
 * @param file_name The path of the file.
 * @return Pointer to the opened directory (FILE*).
 */
FILE *open_file(const std::string &file_name) {
  FILE *file = fopen(file_name.c_str(), "r");
  if (!file) print_error("could not open file " + file_name);
  return file;
}

// STRING PARSERS
// ===================================================================================================================
/**
 * @brief Checks if a given path is a directory.
 * @param path The file/directory path.
 * @return True if the path is a directory, false otherwise.
 */
static bool is_dir(const std::string & path)
{
    struct stat buff;
    if (SYSCALL_SUCCESS != stat(path.c_str(), &buff)) { return false; }
    return S_ISDIR(buff.st_mode);
}

/**
 * @brief Checks if a given path is a regular file.
 * @param path The file/directory path.
 * @return True if the path is a file, false otherwise.
 */
static bool is_file(const std::string & path)
{
    struct stat buff;
    if (SYSCALL_SUCCESS != stat(path.c_str(), &buff)) { return false; }
    return S_ISREG(buff.st_mode);
}

/**
 * @brief Checks if a string ends with a suffix.
 * @param str The string to check.
 * @param suffix The suffix to compare.
 * @return True if the string ends with the suffix, false otherwise.
 */
static bool ends_with(const std::string & str, const std::string & suffix)
{
    if (str.size() < suffix.size()) {
        return false;
    } else {
        return 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }
}

/**
 * @brief Removes a leading ./ from a path.
 * @param path The file/directory path.
 * @return The file/directory path without a "./" prefix.
 */
static std::string clean_path(const std::string &path) {
  if (path.rfind(std::string(CURRENT_DIRECTORY) + PATH_SEPARATOR, 0) == 0) {
    return path.substr(2);
  }
  return path;
}

/**
 * @brief Converts a string to lowercase.
 * @param str The input string.
 * @return The lowercase version of the string.
 */
static std::string lowercase(const std::string &str) {
  std::string lower_str = str;
  std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
  return lower_str;
}

// GLOBALS
// maps each directory to its parent directory. Helps with determining which directories are vacant.
// ALGO: Check if n_files of current directory == 0 and n_files of parent directory != 0.
// ===================================================================================================================
// These hashtables are populated when doing the tree traversal.
std::unordered_map<std::string, std::string> parent_map;
std::unordered_map<std::string, int> n_files_map;
std::unordered_map<std::string, int> most_common_words_map;

// STRUCTS & COMPARATORS
// ===================================================================================================================
struct DirStats {
  std::string largest_file_path = NO_PATH;
  long largest_file_size = DEFAULT_LARGEST_SIZE;
  long n_files = 0;
  // set to 1 to count the current directory itself
  long n_dirs = 1;
  long all_files_size = 0;
  std::vector<ImageInfo> largest_images;
};

/**
 * @brief Comparator for sorting images by size and alphabetically by path.
 */
struct ImageInfoComparator {
  inline bool operator() (const ImageInfo &image1, const ImageInfo& image2) {
    long pixels1 = image1.width * image1.height;
    long pixels2 = image2.width * image2.height;

    // first sort by greater pixel count; if equal, sort alphabetically
    if (pixels1 != pixels2) {
      return pixels1 > pixels2;
    }
    return image1.path < image2.path;
  }
};

/**
 * @brief Comparator for sorting words by frequency and alphabetically.
 */
struct WordFrequencyComparator {
  inline bool operator() (
    const std::pair<std::string, int> &word1, 
    const std::pair<std::string, int> &word2) 
    {
      // first sort by greater word count; if equal, sort alphabetically
      if (word1.second != word2.second) {
        return word1.second > word2.second;
      }
      return word1.first < word2.first;
    }
};

// HELPERS
// ===================================================================================================================
/**
 * @brief Obtains image info from a file (if the file is an image).
 * @param file_path The path to the potential image.
 * @return ImageInfo containing the image's width and height, or null if the file is not an image.
 */
static std::optional<ImageInfo> get_image_info(const std::string &file_path) {
  // 2> /dev/null redirects errors to /dev/null, effectively ignoring them
  std::string command = "identify -format '%w %h' " + file_path + " 2> /dev/null";
  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) print_error("could not call identify via popen on %s" + file_path);

  char buffer[PATH_MAX];
  long width = 0;
  long height = 0;

  if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
    std::istringstream iss(buffer);
    iss >> width >> height;
  }

  int status = pclose(pipe);
  if (status == SYSCALL_SUCCESS && width > 0 && height > 0) {
    return ImageInfo{clean_path(file_path), width, height};
  }

  return std::nullopt;
}

/**
 * @brief Records the occurrences of words in the provided file.
 * @param file_path The path to the file.
 */
static void count_words_in_file(const std::string &file_path) {
  FILE *file = open_file(file_path);
  std::string next_word;
  
  while (true) {
    int c = fgetc(file);
    if (c == EOF) break;
    c = tolower(c);
    
    if (isalpha(c)) {
      next_word.push_back(c);
    }
    // there's a space, or some sort of non-character. At this point the word stops and we'll "reset" it, 
    // but we still need to do an additional check to ensure it's the appropriate length
    else {
      if (next_word.length() >= MIN_WORD_SIZE) {
        most_common_words_map[next_word]++;
      }
      next_word.clear();
    }
  }

  // edge case: last word in file (file might not end in a non-alphabetic character)
  if (next_word.length() >= MIN_WORD_SIZE) {
    most_common_words_map[next_word]++;
  }  

  fclose(file);
}

/**
 * @brief Scans directories and their parents to determine which are top-level vacant.
 * @param
 * @return A vector containing all top level vacant directories.
 */
static std::vector<std::string> get_top_level_vacant_dirs() {
  std::vector<std::string> top_level_vacant_dirs; 
  // ensures the highest level can be identified as top-level, by making its parent have a file count > 0.
  // its parent might not actually have a file count = 1, but this is just for the purposes of the algorithm.
  n_files_map[NO_PATH] = 1;

  for (const auto& [dir_path, n_files] : n_files_map) {
    if (n_files == 0 && n_files_map[parent_map[dir_path]] > 0) {
      top_level_vacant_dirs.push_back(clean_path(dir_path));
    }
  }

  // sort it in alphabetical order to make it easier to compare outputs with the Python file
  std::sort(top_level_vacant_dirs.begin(), top_level_vacant_dirs.end());
  return top_level_vacant_dirs;
}

// DIRECTORY TRAVERSAL
// ===================================================================================================================
/**
 * @brief Records rudimentary statistics about the provided directory.
 * @param dir_path The path to the current directory.
 * @param parent_dir_path The path to the parent directory.
 * @return A DirStats struct containing rudimentary statistics.
 */
static DirStats get_dir_stats(const std::string &dir_path, const std::string &parent_dir_path) {
  DirStats dir_stats;
  DIR *dir = open_directory(dir_path);
  parent_map[dir_path] = parent_dir_path;
  n_files_map[dir_path] = 0;
  
  for (dirent *directory_entry = readdir(dir); directory_entry != nullptr; directory_entry = readdir(dir)) {
    std::string entry_name = directory_entry->d_name;
    if (entry_name == CURRENT_DIRECTORY || entry_name == PREVIOUS_DIRECTORY) continue;
    std::string file_or_subdir_path = dir_path + PATH_SEPARATOR + entry_name;
    
    if (is_file(file_or_subdir_path)) {
      dir_stats.n_files++;
      // not related to stats, but we do this here so that we don't have to traverse the whole
      // tree again. For efficiency (ie. we're already traversing, might as well update the hashtables).
      n_files_map[dir_path]++;

      struct stat file_stat;
      
      if (SYSCALL_SUCCESS == stat(file_or_subdir_path.c_str(), &file_stat)) {        
        if (file_stat.st_size > dir_stats.largest_file_size) {
          dir_stats.largest_file_path = clean_path(file_or_subdir_path);
          dir_stats.largest_file_size = file_stat.st_size;
        } 

        dir_stats.all_files_size += file_stat.st_size;
      }

      if (ends_with(lowercase(file_or_subdir_path), ".txt")) {
        count_words_in_file(file_or_subdir_path);
      }

      auto image_info = get_image_info(file_or_subdir_path);
      if (image_info.has_value()) {
        dir_stats.largest_images.push_back(image_info.value());
      }
    }
    else if (is_dir(file_or_subdir_path)) {
      // we don't increment n_dirs since the recursive call will take care of that for us
      // (because we initialize n_dirs = 1, so each recursive call already counts its own dir)
      DirStats subdir_stats = get_dir_stats(file_or_subdir_path, dir_path);

      if (subdir_stats.largest_file_size > dir_stats.largest_file_size) {
        dir_stats.largest_file_path = clean_path(subdir_stats.largest_file_path);
        dir_stats.largest_file_size = subdir_stats.largest_file_size;
      }

      dir_stats.n_files += subdir_stats.n_files;
      // not related to stats, but we do this here so that we don't have to traverse the whole
      // tree again. For efficiency (ie. we're already traversing, might as well update the hashtables).
      n_files_map[dir_path] += subdir_stats.n_files;
      dir_stats.n_dirs += subdir_stats.n_dirs;
      dir_stats.all_files_size += subdir_stats.all_files_size;
      dir_stats.largest_images.insert(dir_stats.largest_images.end(), subdir_stats.largest_images.begin(), subdir_stats.largest_images.end());
    }
  }
  closedir(dir);
  return dir_stats;
}

/**
 * @brief Analyzes a directory and returns statistics about its contents.
 * @param n The number of most common words and largest images to return.
 * @return A `Results` struct containing directory analysis data.
 */
Results analyzeDir(int n)
{
    Results results;
    // we want the stats for our current working directory, and it has no parent (we consider it to
    // be the highest level)
    DirStats dir_stats = get_dir_stats(CURRENT_DIRECTORY, NO_PATH);
    
    // simple stats
    results.largest_file_path = dir_stats.largest_file_path;
    results.largest_file_size = dir_stats.largest_file_size;
    results.n_files = dir_stats.n_files;
    results.n_dirs = dir_stats.n_dirs;
    results.all_files_size = dir_stats.all_files_size;
    
    std::vector<std::pair<std::string, int>> most_common_words(most_common_words_map.begin(), most_common_words_map.end());
    std::sort(most_common_words.begin(), most_common_words.end(), WordFrequencyComparator());
    most_common_words.resize(std::min(static_cast<int>(most_common_words.size()), n));
    results.most_common_words = most_common_words;
    
    std::sort(dir_stats.largest_images.begin(), dir_stats.largest_images.end(), ImageInfoComparator());
    dir_stats.largest_images.resize(std::min(static_cast<int>(dir_stats.largest_images.size()), n));
    results.largest_images = dir_stats.largest_images;

    results.vacant_dirs = get_top_level_vacant_dirs();
    
    return results;
}
