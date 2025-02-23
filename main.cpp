#include "analyzeDir.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

constexpr int MAX_OPEN_FILES = 256;
constexpr int EXPECTED_ARG_COUNT = 3; // Expecting program name, N, and directory name
constexpr int ARG_N_INDEX = 1;
constexpr int ARG_DIR_INDEX = 2;
constexpr int SYSCALL_SUCCESS = 0;
constexpr int PROGRAM_FAILED = -1;

/**
 * @brief Prints usage instructions and exits the program.
 * 
 * @param pname The program name.
 * @param exit_code The exit status code.
 */
void usage(const std::string & pname, int exit_code)
{
    printf("Usage: %s N directory_name\n", pname.c_str());
    exit(exit_code);
}

int main(int argc, char ** argv)
{
    // set the max number of open file descriptors to avoid hitting system limits
    {
        struct rlimit rlim;
        rlim.rlim_cur = MAX_OPEN_FILES;
        rlim.rlim_max = MAX_OPEN_FILES;
        int res = setrlimit(RLIMIT_NOFILE, &rlim);
        assert(res == SYSCALL_SUCCESS);
    }

    // convert the first argument (N) to an integer and analyze the directory
    if (argc != EXPECTED_ARG_COUNT || chdir(argv[ARG_DIR_INDEX])) { usage(argv[0], PROGRAM_FAILED); }
    Results res = analyzeDir(std::stoi(argv[1]));
    
    // print the results in a formatted output
    printf("--------------------------------------------------------------\n");
    printf("Largest file:      \"%s\"\n", res.largest_file_path.c_str());
    printf("Largest file size: %ld\n", res.largest_file_size);
    printf("Number of files:   %ld\n", res.n_files);
    printf("Number of dirs:    %ld\n", res.n_dirs);
    printf("Total file size:   %ld\n", res.all_files_size);

    // descending order, follwoed by alphabetical
    printf("Most common words from .txt files:\n");
    for (auto & w : res.most_common_words) {
        printf(" - \"%s\" x %d\n", w.first.c_str(), w.second);
    }
    printf("Vacant directories:\n");
    for (auto & d : res.vacant_dirs) { printf(" - \"%s\"\n", d.c_str()); }
    
    // descending order by size, followed by alphabetical
    printf("Largest images:\n");
    for (auto & ii : res.largest_images) {
        printf(" - \"%s\" %ldx%ld\n", ii.path.c_str(), ii.width, ii.height);
    }
    printf("--------------------------------------------------------------\n");
    return 0;
}
