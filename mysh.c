/**
* mysh.c
*/

#include<stdlib.h>
#include <string.h>
#include<unistd.h>
#include<stdbool.h>
#include<stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <glob.h>
#include <ctype.h>

// DEFINITIONS
#define BUFFER_SIZE 1024
#define PATH_LEN 4096
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
     // Malloc a variable arraylist
     char** argv;
     // The input files will use the file descriptor in order to set an input, output.
     // Typically, this should be STDIN_FILENO for input and STDOUT_FILENO
     // Unless there are `<` and `>`
     int argc;
     int input_file;
     int output_file;
 }command_t;

  /**
  * free command struct
 */
void free_struct(command_t* command) {

    //iterator for argv
    int i = 0;

    printf("argv at %i: %s\n", i, command->argv[i]);
    //check to see if the path is also the first argument
    if (strcmp(command->path, command->argv[i]) == 0) {
        printf("the path is the same as the first arg\n");
        free(command->path);
        i++;
    }

    //free each argument
    for (i; i < command->argc; i++) {
        printf("argv at %i: %s\n", i, command->argv[i]);
        free(command->argv[i]);
    }

    //free the list
    free(command->argv);

    //free the struct
    free(command); 

}


 /**
  * Read Line of input
  *
  * @param buf : Pointer to a character buffer that I can write into.
  * @param fd: File descriptor to read from.
  * - For anything with pipings, pass in name_of_pipe[0] into this.
  */
int read_input(char** buf_ptr, int fd) {
    int total_length = 0, len = 0, rd;
    char* buf = *buf_ptr;
    char c;
    bool first_word = true;
     while((rd = read(fd, &c, sizeof(char))) >= 0) {
         if(rd < 0) perror("Read error.");
         else if(rd == 0) {
             break;
         }
         else {
            if(DEBUG) printf("%c Read: %i\n", c, rd);
             if(c == '\n') {
                 break;
             } else {
                 buf[total_length] = c;
                 total_length++;
                 if(c == ' ') first_word = false;
                 if(first_word) len++;
             }
         }
     }
     return len;
}


 /**
  * Search functionality
  *
  * We need a way to search the paths
  */

 char* search(char *program) {
     // Taking in a program name, search through all the required locations for the program.
     // Return where mysh would use as a path.
     //we will only search the directories /usr/local/bin, /usr/bin, and /bin
     static char* search_params[] = {"/usr/local/bin/", "/usr/bin/", "/bin/"};
     if (DEBUG) printf("long: %s, mid: %s, small: %s\n", search_params[0], search_params[1], search_params[2]);

     for(int i = 0; i < 3; i++) {
        char* search_concat = malloc(PATH_LEN);
        strcpy(search_concat, search_params[i]);
        strcat(search_concat, program);
         if (access(search_params[i], F_OK) == 0) {
             //found
             printf("Found at path: %s\n", search_concat);
             return search_concat;
         }
         free(search_concat);
     }
     return "fail"; 
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
char** handle_wildcards(command_t* command, char * pathname, char** argv, int* argc) {
    char **matching;
    glob_t gstruct;
    int r; 

    r = glob(pathname, GLOB_ERR , NULL, &gstruct);
    //error checking
    if( r!=0 ){
        if( r==GLOB_NOMATCH ) {
            //just return the pathname
            *argc++;
            argv = realloc(argv, (*argc) * sizeof(char *));
            argv[*argc - 1] = pathname;
            
            return argv;
        } else {
            fprintf(stderr,"Some kinda glob error\n");
            return EXIT_FAILURE;
        }
        
    }
    
    printf("current argc before found: %i\n", *argc);
    //increment argc
    int k = *argc - 1;
    *argc += gstruct.gl_pathc;
    printf("argc after found: %i\n", *argc);
    argv = realloc(argv, (*argc) * sizeof(char *));


    matching = gstruct.gl_pathv;
    while(*matching) {
        argv[k] = *matching;
        k++;
        matching++;
    }
    return argv;
}

/**
 * check for slash in first token
*/

bool slash_check(char* token) {
    int i = 0;
    while(isgraph(token[i])) {
        if (token[i] == '/') {
            return true;
        }
        i++;
    }
    return false;
}

/**
 * function to add arguments to the argv array
*/
char **arg_add(command_t *command, char **argv, int *argc, char *arg)
{
    (*argc)++;
    argv = realloc(argv, (*argc) * sizeof(char *));

    if (argv == NULL) {
        perror("Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    // Add the new element
    argv[(*argc) - 1] = arg;
    printf("what was added: %s\n", argv[(*argc) - 1]);

    return argv;
}

/**
 * helper function to find the path of the first token
 * 
*/
void find_path(command_t *command, char* first_word) {
    if (slash_check(first_word)) {
        //the first token can be considered as a path
       
        command->path = first_word;
    } else if (strcmp(first_word,"cd") == 0 || strcmp(first_word, "pwd") == 0 || strcmp(first_word,"which") == 0 ) {
        //it's a built-in command
        //STDIN_FILENO for input and STDOUT_FILENO
        
        command->path = first_word;
        command->argv = arg_add(command, command->argv, &command->argc, first_word);
        command->input_file = STDIN_FILENO;
        command->output_file = STDOUT_FILENO;
    } else {
        //it's a bare name of a program or shell command
        char* temp = search(first_word);
        
        if (DEBUG) {printf("what's in temp: %s, what's in first word: %s\n", temp, first_word);}
        if (strcmp(temp, "fail") ==0) {
            //add to front of argument list & path
            command->path = first_word;
            command->argv = arg_add(command, command->argv, &command->argc, first_word);
        } else {
            first_word = temp;
            command->path = first_word;
            command->argv = arg_add(command, command->argv, &command->argc, temp);
        }
    }
}

/**
 * Parse each line of commands
 * 
 * possible commands: 
 * - <,>, | (max 2), conditionals (then, else)
 * - cd, pwd, which, exit
 * - cp, mv, cat (don't have to implement ourselves
 * 
 *
 * - add the precedence checking for pipes and redirection 
 *      - maybe make the initial command for the pipe, then wait until the line is finished to
 * - possibly make it more flexible
 * - set errors for file open failure?
 * 
*/

command_t* parse_line(char* line) {
    printf("%s\n", line);
    //initialize struct
    command_t *holder = malloc(sizeof(command_t));
    command_t *piped_comm;
    holder->argv = NULL;
    holder->argc = 0;

    //array to hold the argv_found
    //update the size later on
    //char* argv_found[50];
    //int row = 0;

    //search for the first word to determine what to do
    char* first_word = malloc(sizeof(char) * 1024);
    int k = 0;
    while (!isspace(line[k])) {
        k++;
    }

    //copy in the first word
    strncpy(first_word, line, k);
    first_word[k] = '\0';
    if (DEBUG) { printf("the first token: %s\n", first_word);}

    find_path(holder, first_word);
    //free(first_word);
    if (DEBUG) {printf("the path: %s, k: %i\n", holder->path, k);}

    //hold whether or not we've found <,>, |
    bool found = false;

    //1 if output >, 0 if input <
    bool output = 0;
    //0 if no pipe present, 1 if piping present & adding to args
    bool pipe_output = 0;
    bool piped_output_first_word = false;

    //vars to hold whether in word or not and if there's wildcard
    bool in_word = false;
    bool wild_found = false;

    //holds where in the word we're at
    int pos = 1;

    //hold the file descriptors
    int fd_o, fd_i;

    int pipes[2];
    pipe(pipes);


    //increment index past space
    k++;
    while (k < strlen(line)) {
        int word_size = 0;
        // If the line isnt a space, keep going till we find one

        char* read_word = malloc(PATH_LEN);
        while(k < strlen(line) && !isspace(line[k])) {
            if(DEBUG) printf("read letter: %c, new length %d\n", line[k], word_size);
            read_word[word_size] = line[k];
            word_size++;
            k++;        // Add one to the char line
        }
        read_word[word_size] = '\0';
        if(DEBUG) printf("This is the word read: %s of length: %d with pipe_found = %d\n", read_word, word_size, pipe_output);

        bool found_pipe = false;
        if(strcmp(read_word, "|") == 0) {
            found_pipe = true;
            printf("found pipe\n");
        }
        else if(pipe_output) {
            // Code for if after here, we are doing piped output
            if(!piped_output_first_word) {
                // First word has not been entered into 2nd struct
                find_path(piped_comm, read_word);
                printf("found path: %s\n", piped_comm->path);
                piped_output_first_word = true;
            }
        }
        if(found_pipe) {
            pipe_output = found_pipe;
            piped_comm = malloc(sizeof(command_t));
            piped_comm->argv = malloc(sizeof(char*));
        }
        free(read_word);
//        printf("%c\n", line[k]);
//        printf("%c\n", line[k]);
        k++;
//        printf("%c\n", line[k]);
    }
}


/**
 * Actually run the commands...
 *
 * @param command_t* command - A filled out struct with the command, args, and any pipes
 *
 */
 int run_command(command_t *comm) {
    // Handle pwd, cd, which
    if(comm->path[0] == '\0') return EXIT_SUCCESS;
    if(strcmp(comm->path, "cd") == 0) {
        // Change directory command found
        if (comm->argc > 2) {
            // There are more than 1 arguments in cd
            perror("cd expects only one command.\n");
            return EXIT_FAILURE;
        } else {
            chdir(comm->argv[1]);
        }
    } else if (strcmp(comm->path, "pwd") == 0) {
        // Return present working directory
        char* directory = malloc(PATH_LEN);
        if(getcwd(directory, PATH_LEN) == NULL) {
            perror("Unable to switch directories, are you sure the path is correct?\n");
            free(directory);
            return EXIT_FAILURE;
        } else {
            write(comm->output_file, directory, strlen(directory));
            write(comm->output_file, '\n', 1);
            free(directory);
            return EXIT_SUCCESS;
        }
    } else if (strcmp(comm->path, "which") == 0) {
        if(comm->argc > 1) {
            perror("which given wrong number of arguments.\n");
            return EXIT_FAILURE;
        } else {
            char *path = search(comm->argv[0]);
            if(path[0] == '\0') {
                perror("Issue with searching...");
            } else {
                write(comm->output_file, path, strlen(path));
            }
        }
    } else {
        // This is for everything else...
//        int pipe_fd[2];
//        pipe(pipe_fd);
        pid_t pid = fork();
        if(pid == -1) {
            perror("Error when forking process.\n");
            return EXIT_FAILURE;
        } else if (pid == 0) {
            // In the new child process.
            dup2 (comm->output_file, STDOUT_FILENO);
            execv(comm->path, comm->argv);
            // Handle the piping
//            if(comm->pipe_exists) {
//                int pipe_fd_n[2];
//                pipe(pipe_fd_n);
//
//            }

        } else {
            // parent process
            int st;
            if (waitpid(pid, &st, 0) == -1) {
                perror("Error with child process");
                return EXIT_FAILURE;
            }
        }
        return EXIT_FAILURE;
    }
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

    // Check whether or not to use batch mode
    printf("Number of args: %d\n", argc);
    bool use_batch = false;

    // If the arguments are greater than 1, check if any of it is terminal
    if(argc > 1) {
        for(int i = 1; i < argc; i++) {
            int fd = open(argv[i], O_RDONLY);
            if(!isatty(fd)) use_batch = true;
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
    }

    // If no specified input file
    if(!isatty(STDIN_FILENO)) {
        use_batch = true;
//        int fd = fopen();
//        dup2(fd, STDIN_FILENO);
//        close(fd);
    }

    // Interactive mode
    if(!use_batch) printf("Welcome to the shell! Running in interactive mode.\n");
    // Batch mode -> Loop through and retrieve all
    if(use_batch) {
        if(DEBUG) printf("Batch mode instead.\n");
    }


    bool keep_running = true;
    // I dont know how big this buffer size should be.
    char c;
    int rd, prev_exit_status;
    while(keep_running) {
        char* buf = calloc(BUFFER_SIZE, sizeof(char));

        // Init messages
        if(!use_batch) write(STDOUT_FILENO, "mysh> ", 6);

        // Read the line
        //parse_line(buf);
//            printf("Command is: %s\n", buf);
        int len = read_input(&buf, STDIN_FILENO);
        char comm[len+1];
        for(int i= 0; i < len; i++) {
//            printf("%c vs %c\n", comm[i], buf[i]);
            comm[i] = buf[i];
        }
        comm[len] = '\0';
//        write(STDOUT_FILENO, comm, len);
        if(strcmp(comm, "exit") == 0) {
            write(STDOUT_FILENO, "Exit signal received... Goodbye!\n", 33);
            keep_running = false;
        } else if(strcmp(comm, "then") == 0) {
            // Received a "then" statement as the first token
            // Check the exit status of the last command
            if(DEBUG) printf("Then statement found!\n");
            if(DEBUG) printf("%s \n", buf+len+1);
            if(prev_exit_status == EXIT_SUCCESS) {
                // Run code for exit success
                // Need to get everything past the "then" statement
                // That will be passed to the argument parser
            }
        } else if(strcmp(comm, "else") == 0) {
            // received an else statement
            if(DEBUG) printf("Else Statement found!\n");
            if(DEBUG) printf("%s \n", buf+len+1);
            if(prev_exit_status == EXIT_FAILURE) {
                // Run code for exit failure
            }
        } else {
            write(STDOUT_FILENO, buf, strlen(buf));
            command_t* use = parse_line(buf);
            char* directory = malloc(PATH_LEN);
            getcwd(directory, PATH_LEN);
            if(DEBUG) printf("Old Path is: %s \n", directory);
//            run_command(use);
            getcwd(directory, PATH_LEN);
//            if(DEBUG) printf("%s %s \n", use->path, use->argv[1]);
//            if(DEBUG) printf("New Path is: %s\n", directory);
            
            free(directory);
//            free_struct(use);
            
        }
        free(buf);
    }



    // @todo parse through all the args in a command line and add them to the struct.
}