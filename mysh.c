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
     // Malloc another struct, populate it, and put address here.
//     bool pipe_exists;
//     struct command_struct* piped_command;
 }command_t;

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
     static char* search_params[] = {"/user/local/bin/", "/usr/bin/", "/bin/"};
     if (DEBUG) printf("long: %s, mid: %s, small: %s\n", search_params[0], search_params[1], search_params[2]);

     for(int i = 0; i < 3; i++) {
         if (access(search_params[i], F_OK) == 0) {
             //found
             printf("Found at path: %s\n", search_params[i]);
             return search_params[i];
         }
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
char** handle_wildcards(char* pathname, char** argv, int* argc) {
    char **matching;
    glob_t gstruct;
    int r; 

    r = glob(pathname, GLOB_ERR , NULL, &gstruct);
    
    //error checking
    if( r!=0 ){
        if( r==GLOB_NOMATCH ) {
            //just return the pathname
            argv[*argc] = pathname;
            *argc++;
        } else {
            fprintf(stderr,"Some kinda glob error\n");
            return EXIT_FAILURE;
        }
        
    }
    
    //increment argc
    *argc += gstruct.gl_pathc;
    
    matching = gstruct.gl_pathv;
    while(*matching) {
        argv[*argc] = *matching;
        *argc++;
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
    //initialize struct
    command_t *holder = malloc(sizeof(command_t));

    //array to hold the argv_found
    //update the size later on
    char* argv_found[50];
    int row = 0;

    //search for the first word to determine what to do
    char first_word[20];
    int k = 0;
    while (!isspace(line[k])) {
        k++;
    }

    //copy in the first word
    strncpy(first_word, line, k);
    first_word[k] = '\0';
    if (DEBUG) { printf("the first token: %s\n", first_word);}

    if (slash_check(first_word)) {
        //the first token can be considered as a path
       
        holder->path = first_word;
        printf("HHIHIHIH\n");
    } else if (strcmp(first_word,"cd") == 0 || strcmp(first_word, "pwd") == 0 || strcmp(first_word,"which") == 0 ) {
        //it's a built-in command
        //STDIN_FILENO for input and STDOUT_FILENO
        
        holder->path = first_word;
        printf("hi im in here\n");
        holder->input_file = STDIN_FILENO;
        holder->output_file = STDOUT_FILENO;
    } else {
        //it's a bare name of a program or shell command
        char* str_tmp = malloc(PATH_LEN);
        strcpy(str_tmp, search(first_word));

        printf("what's in temp: %s, what's in first word: %s\n", str_tmp, first_word);
        if (strcmp(str_tmp, "fail") ==0) {
            //what to do here?
            //printf("Named program not found\n");
            //figrue out what to do here
            printf("added as path & first argument\n");
            holder->path = first_word;
            argv_found[row] = first_word;
            row++;
            holder->argc += 1;
        } else {
            strcat(str_tmp, first_word);
//            strcpy(holder->path, str_tmp);
            holder->path = str_tmp;
            printf("%s\n", holder->path);
            printf("uh hi?\n");
        }
//        free(str_tmp);
    }
    printf("the path: %s, k: %i\n", holder->path, k);

    //hold whether or not we've found <,>, |
    bool found = false;

    //1 if output >, 0 if input <
    bool output = 0;
    //0 if not needed
    bool pipe_output = 0;

    //vars to hold whether in word or not and if there's wildcard
    bool in_word = false;
    bool wild_found = false;

    int pos = 1;

    //hold the file descriptors
    int fd_o;
    int fd_i;

    k++;
    //hold argc
    int argc_found = 0;
    while (line[k]) {
        printf("the current char: %c, current pos: %i, current index: %i\n", line[k], pos, k);
        if (isspace(line[k]) && in_word && found == false ) {
            //new argument for argv
            //copy word from string into new var
            char temp[15];
            if (DEBUG) {printf("k: %i, pos: %i\n", k, pos);}
            strncpy(temp, &line[k - (pos - 1)], pos);
            temp[pos] = '\0';

            printf("the argument list has added: %s\n", temp);

            if (wild_found == true) {
                //call the wildcard expansion thing
                handle_wildcards(temp, argv_found, &argc_found);
            } else {
                argv_found[row] = temp;
                row++; 
            }
        
            wild_found = false;
            in_word = false;
            argc_found++;
            pos = 0;
        } else if (isspace(line[k]) && in_word && found == true) {
            //save this as our output/input file for redirection
            //copy word from string into new var
            char temp[15];
            printf("the index: %i, the pos: %i\n", k, pos);
            strncpy(temp, &line[k - (pos - 1)], pos);
            temp[pos] = '\0';
            in_word = false;
            printf("the file has added: %s\n", temp);
            
            if (output == 1) {
                //this file is for output redirection
                fd_o = open(temp, O_RDWR);
                holder->output_file = fd_o;
                printf("output: %i\n", fd_o);
            } else if (output == 0) {
                //this file is for input redirection
                fd_i = open(temp, O_RDWR);
                holder->output_file = fd_i;
                printf("input: %i\n", fd_i);
            }
            printf("this file is considered: %i\n", output);
            
            //continue adding to arg list
            in_word = false;
            found = false;
            pos = 0;
        } else if (isspace(line[k]) && in_word && pipe_output) {
            //set the piping output
            //copy the word in
            char temp[15];
            strncpy(temp, &line[k - (pos - 1)], pos);
            temp[pos] = '\0';
            in_word = false;

            fd_o = open(temp, O_RDWR);
            holder->output_file = fd_o;
            printf("file %s has been added as piping output\n", temp);
            
            command_t* other = malloc(sizeof(command_t));
            other->output_file = STDOUT_FILENO;
            fd_i = open(holder->path, O_RDWR);
            other->input_file = fd_i;
            other->path = temp;
            other->argc = 1;

            pos = 0;
            in_word = false;
            pipe_output = false;
        } else if (line[k] == '>' || line[k] == '<') {
            found = true;
            //set input & output stuff
            if (line[k] == '>') {
                output = 1;
            } else {
                output = 0;
            }
            in_word = false;
            pos = 0;
        } else if (line[k] == '|') {
            //call the search
            found = true;
            in_word = false;
            pipe_output = true;
            //set the piping input using the last item in the argv list 

            holder->input_file = STDIN_FILENO;
            pos = 0;
        } else {
            //add to the current word we are building on
            if (line[k] == '*') {
                wild_found = true;
            }
            pos++;
            in_word = true;
        }
        k++;
    }

    k--;
    //check if still in word and then add to appropriate place
    if (in_word) {
        printf("the pos: %i, the index: %i\n", pos, k);
        if (found == false ) {
            //new argument for argv
            //copy word from string into new var
            char temp[15];
            printf("k: %i, pos: %i\n", k, pos);
            strncpy(temp, &line[k - (pos - 2)], pos);
            temp[pos] = '\0';
            printf("the argument list has added: %s\n", temp);

            if (wild_found == true) {
                //call the wildcard expansion thing
                handle_wildcards(temp, argv_found, argc_found);
            } else {
                argv_found[row] = temp;
                row++; 
            }
            argc_found++;
            wild_found = false;
            in_word = false;
            pos = 0;
        } else if (found == true) {
            //save this as our output/input file for redirection
            //copy word from string into new var
            char temp[15];
            strncpy(temp, &line[k - (pos - 2)], pos);
            temp[pos] = '\0';
            in_word = false;
            printf("the file has added: %s\n", temp);
            
            if (output == 1) {
                //this file is for output redirection
                fd_o = open(temp, O_RDWR);
                holder->output_file = fd_o;
            } else if (output == 0) {
                //this file is for input redirection
                fd_i = open(temp, O_RDWR);
                holder->output_file = fd_i;
            }
            
            //continue adding to arg list
            found = false;
            pos = 1;
        } else if (pipe_output) {
            //set the piping output
            //if argv is empty, use the path name
            if (argc_found < 1) {
                fd_i = open(holder->path, O_RDWR);
            } else {
                fd_i = open((holder->argv)[argc_found - 1], O_RDWR);
            }
            holder->input_file = fd_i;
            pipe_output = false;
        }
    }

    holder->argc = argc_found;
    holder->argv = argv_found;

    return holder;
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
        if (comm->argc > 1) {
            // There are more than 1 arguments in cd
            perror("cd expects only one command.\n");
            return EXIT_FAILURE;
        } else {
            chdir(comm->argv[0]);
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
        }
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
            run_command(use);
            getcwd(directory, PATH_LEN);
            if(DEBUG) printf("%s %s \n", use->path, use->argv[0]);
            if(DEBUG) printf("New Path is: %s\n", directory);
            free(use->path);
            free(use);
            free(directory);
            
        }
        free(buf);
    }



    // @todo parse through all the args in a command line and add them to the struct.
}