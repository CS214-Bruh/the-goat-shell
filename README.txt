Project III: My Shell
By: Emily Cao(EC1042) & Ivan Zheng (IZ60)

Compilation & Execution:
- in order to compile our code you can simply type "make"
- then "./mysh" with no other arguments to enter interactive mode, or with arguments for batch mode

Design Notes:
- we have a command struct that stores in information about each command
    - path: stores path to Unix command, bare names, built-in commands, pathnames
    - argv: stores the argument list
    - argc: stores the number of arguments in argv
    - input_file: stores the file descriptor for input
    - output_file: stores the file descriptor for output 
- our code is split into many functions which we will shortly describe here
    - free_struct: iterates through and frees all memory associated with command struct
    - read_input
    - search: searches for program name in 3 locations & returns appropriate path
    - handle_wildcards: utilizes glob to find all instances of wild_cards
    - slash_check: checks for slashes, if found, treated as path
    - arg_add: adds argument to argv list of command struct
    - find_path: checks for built-in, with slash, or utilizes search to determine path for first token
    - run_command
    - parse_line: iterates through command string, tokenizes, and assigns variables to account for 
        running different command modes
- structure of main: 

Testing:
- in terms of testing, we have script file test.sh in script-files, which holds a variety of 
    tests regarding different commands for batch mode
- we have files receive.txt (for outputs to direct to), ./a.out (gcc -Wall of out.c), and test.c (for cat testing)
