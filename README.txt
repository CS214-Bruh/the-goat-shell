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
    - read_input:
        - Read_input is a helper function designed to help the main function to parse through the input.
        - It was made to modularize the code more, in case it was needed later.
    - search: searches for program name in 3 locations & returns appropriate path
    - handle_wildcards: utilizes glob to find all instances of wild_cards
    - slash_check: checks for slashes, if found, treated as path
    - arg_add: adds argument to argv list of command struct
    - find_path: checks for built-in, with slash, or utilizes search to determine path for first token
    - run_command:
        - Run_command takes care of multiple different commands.
            - Built-in commands:
                - cd -> takes in one param and changes directory to it, printing an error if one occurs.
                - pwd -> prints out the present working directory.
                - which -> displays the path to the program.
            - Non-built in commands:
                - Uses the find_path() function in order to find the path to the command, and run it with execv().
                - Starts a child process which is able to read and write from the specified in/out files
                - Parent waits for child to die and returns a exit status.
    - parse_line: iterates through command string, tokenizes, and assigns variables to account for 
        running different command modes
- structure of main:
    - The main function is in charge of a few separate things:
        - Batch vs Interactive mode: Differentiate between the two modes and start the required one.
            - This is done through the use of isatty(), to check whether STDIN_FILENO is associated with the program.
        - Maintaining a buffer to read the input line by line, whether it is in batch mode or interactive.
            - This read buffer will parse the first letter for the 3 keywords below, and truncate them to pass to parse_line().
        - Then statements: by holding on to the latest return status we can use it to decide whether to run the then statement.
        - Else statements: same goes for the else.
        - Exit statement: upon receiving exit, immediately terminate the loop.

Testing:
- in terms of testing, we have script file test.sh in script-files, which holds a variety of 
    tests regarding different commands for batch mode
- we have files receive.txt (for outputs to direct to), ./a.out (gcc -Wall of out.c), and test.c (for cat testing)

Some Limitations We've Encountered:
- We have noticed that there are a few memory leaks upon exiting the program using "exit".
    - Unfortunately, even after trying many different things, we were unable to fix it before the deadline.
