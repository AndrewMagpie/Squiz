1. Building assumptions:
    - make is on the path
    - gcc  is on the path
    - gcc  is c++20 compatible

1. Tested on:
    - Ubuntu 22.04.1 LTS (inside WSL)

1. Build//Clean/Rebuild/Run
    - make
    - make clean
    - make rebuild
    - make run
    - make run RUN_OPTIONS="-f 'his head a flag'"

1. Running on its own:
    - ./bin/squiz  -f "his head a flag"
    - ./bin/squiz  -help

1. Running Tests:
    - make tests

1. Functionality assumptions:
    - Searches are case insensitive.
    - Searches ignore non-alphas.

1. TODO:
    - Unicode ?
    - threads ?
    - Indexfile is just a text file.  Converting it to a binary file will improve load time.
    - More settings to narrow the search.
    - More Tests
    - Validation of search file.
    - Better word matches.  Splash should match with Splashed. :(

1. Details:
    1. The program parses the command line for settings which determines how the program is run.  
These are the options available:  

                -h   Displays this help text.
                -q   Quiet mode. No console output.
                -v   Verbose mode. Verbose messages will display on the console.
                -n   Noisy mode. Noisy messages will display on the console.
                -i   Recreate the index file.
                -np  No prompt text for the search text will be displayed.
                -rc  The return code is set to the number of matches.
                -s   Specify the search file. Defaults to: squiz-coding-challenge-lepanto.txt
                -f   Specify the text to search for

    1. The program then tries to find/create/load an index file based on the search file.  
        It contains a last modified time stamp of the search file. If the time stamp is older than that of the search file a new index file is created.  
        The index file is created to speed up searches. Instead of enumerating the lines of entire search file every time the program is run, it uses the index file.  
        The index file contains only the unique words and the lines they appear in.  
        So searching for a word can quickly look up the lines its on, instead of searching the whole file.  
        Line offsets (the start of each line) is alway stored in the index file.  
    1. The program then either uses the search text passed on the command line or prompts the user for the search text.  
        The search text is broken up into words, each word is searched for in the index data.  
        For each word, the index data returns the lines that it had appeared on. Once all words have been searched for it then determines which lines have the most words.  
        The greatest number of words on a line is considered a match.  
    1. The program then displays the matches on the console.  