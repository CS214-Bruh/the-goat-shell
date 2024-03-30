/**
* mysh.c
*/

#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifndef DEBUG
#define DEBUG 0
#endif

/**
 * Create a struct for storing each line of commands
 *
 * The struct will store:
 * - The path to the executable
 * - The list of argument strings
 * - The files to use for StdIn and StdOut
 */
 typedef struct command_struct {
     char* path;
     char** argv;
     // The input files will use the file descriptor in order to set an input, output.
     // Typically, this should be STDIN_FILENO for input and STDOUT_FILENO
     // Unless there are `<` and `>`
     int input_file;
     int output_file;
 }command_t;

/**
 * Handle all the different possible commands the shell can receive
 *
 * There are two types of commands possible:
 *
 * @b In-built commands:
 * - These commands are the `cd`, `pwd`, and `which` commands.
 * @b Executables in Directories:
 * - There are 3 different locations to search:
 *      - `/usr/local/bin`, `/usr/bin`, and `/bin`
 *      - Using the `access()` function, you can determine whether the file exists
 */

int handle_builtin(command_t* command) {
    // @todo fill this in
    return EXIT_SUCCESS;
}

int handle_non_builtin(command_t command) {
    // @todo fill this in
    return EXIT_FAILURE;
}

/**
 * @b Processes
 *
 * - The shell can use multiple different processes
 * - How are we handling this?
 */

/**
* Handle Filename wildcards
 *
 * - These wildcards are ones that include an `*`.
 * - When receiving this, execute the commands on all of the following files
*/
int handle_wildcards(char** pathname) {
    // @todo fill this in
    return EXIT_SUCCESS;
}

/**
 * In the main function, we need to be able to distinguish the two modes to run mysh.
 *
 * @Modes Interactive Mode
 *      - Interactive mode accepts input from StdIn, outputting start and end messages.
 * @Modes Batch Mode
 *      - Batch mode runs cmd line arguments as if it was an executable.
 *
 */

int main(int argc, char** argv) {
    // @todo Differentiate between the two modes
//    command_t new_comm = malloc(sizeof(command_t));

    // I dont know how big this buffer size should be.
    char* buf[];
    // Check whether or not to use batch mode
    bool use_batch = false;
    if(argc > 1) {
        for(int i = 1; i < argc; i++) {
            int fd = open(argv[i], O_RDONLY);
            if(!isatty(fd)) use_batch = true;
        }
    }
    if(!use_batch) {
        // Interactive mode
        bool keep_running = true;
        while(keep_running) {
            read(STDIN_FILENO, )
            if()
        }
        if(DEBUG) printf("The input is associated with the terminal.\n");
    } else {
        // Batch mode -> Loop through and retrieve all
        if(DEBUG) printf("Input is not associated with the terminal.\n");
    }

    // @todo parse through all the args in a command line and add them to the struct.
}