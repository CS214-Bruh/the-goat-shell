Project III: My Shell
By: Emily Cao(EC1042) & Ivan Zheng (IZ60)

Compilation & Execution:
- in order to compile our code you can simply type "make"
- then "./mysh" with no other arguments to enter interactive mode, or with arguments for batch mode

Design Notes:
- we have a command struct that stores in information about each command
- our code is split into many functions which we will shortly describe here
    - free_struct
    - read_input
    - search
    - handle_wildcards
    - slash_check
    - arg_add
    - find_path
    - run_command
    - parse_line
- structure of main: 

Testing:
- in terms of testing, we have script file test.sh in script-files, which holds a variety of 
    tests regarding different commands for batch mode
- we have files receive.txt (for outputs to direct to), ./a.out (gcc -Wall of out.c), and test.c (for cat testing)
