CS311_Assignment2
=================

## Overview
This program creates a series of pipes to communicate to sub-processes that sort a set of words. It starts by parsing a list of words from stdin.

## Assumptions
The program makes a few assumptions that the user should be aware of. It assumes that the text is being redirected to stdin from the command line at runtime. All input is taken from stdin. It also assumes that the text in the file consists of alphabetic characters. Characters that are symbols (such as , etc.), when fed to the input of the program, has undefined behavior. The program will most likely not delimit these characters correctly, and the char array reading in the words will overflow, causing a crash. This may or may not be fixed in a later release.
It also assumes that there is only one command line argument passed to the program. This ar- gument must be a positive integer value. This will be the number of processes to use to sort the word list.
The list of words being input also must be relational to the number of pipes being used. As the filesize gets bigger, the number of pipes must get bigger. Note that there is a limit to how many processes can be created. Passing too large an integer to the program may result in the program exiting. This number is system-dependent.

## Functions
The program contains one source file called ”main.cpp”. This file contains five functions that each
handle a separate task. They are: parse_words(), write_to_sort(), read_from_sort(), print_sorted_subvectors(), make_pipes(). The main() function calls these functions, and also handles a few small tasks not deemed complex enough to merit their own function. For documentation on how to use these functions, see comment blocks preceding the function in the source code. Below is a discussion on the design implementation choices used for each function.

### main()
The main() function is used to initalize variables that will be passed to other functions, some of which must be used between multiple functions. The pipe arrays are used to write and read to the sort programs, so they are created in the main function and passed to the others. By creating them in main(), it is easier to keep track of them, and allows us to ensure they are all closed at the end of the program’s lifecycle. The main function is also the ”home-base” function for the parent process. The children do not exist in the main function. This keeps main() cleaner looking, and allows a reader to easily see the flow of the program without needing to worry about the ”worker” functions. Every function, sans print sorted subvectors(), returns a value that main() checks to ensure there were no errors. If there were, the sub-function prints the error, and main() terminates the program.

### parse_words()
parse_words() reads in strings one at a time from stdin. It uses fscanf() to get the words. It was a poor design choice to use this method, because it allows for the char array that stores the word to be overflown, causing the program to crash. Using cin or fgets() would have been a better idea, and would have been switched in, had time allowed. If the word does not have a newline character at the end, one is added. It distributes the words that it reads in round robin to subvectors of a vector. The vector holds subvectors of all the words, with as many subvectors as there were processes specified to the program.

### make_pipes()
This function sets up the pipes. It creates child processes in a for loop, which inherit copies of the pipes. It copies over stdout with the write end of one of the pipes for the child processes, so that when they execute the sort program (which by default writes its output to stdout), the output will be written to the pipe to the parent. The parent can then read the output from the sort program. It also copies the read end over stdin, so that the input for the sort program will be whatever is written to the write end of the pipe in the parent process.

### write_to_sort()
This function writes all the words to the child sort programs. It uses the array of pipe file descriptors passed to it to open file streams to the sort programs. It then iterates through the subvectors passed to it that contain the words to sort, and closes the file streams.

### read_from_sort()
This function does the opposite of write_to_sort(). It reads back the words from the sort programs by opening streams using the array of pipe file descriptors sent to it. It then iterates through the subvectors of the vector created in it and adds the words to them. It may have been faster to read one word from each stream at a time. Had time allowed, this would have been tested to see if it was more efficient. When they are all read, it closes the streams and returns.

### print_sorted_subvectors()
Here we iterate through the sorted subvectors and print out the words in sorted order, with regard to words in other subvectors, omitting duplicate words. The method used for parsing the words in the subvectors and printing them was a poor choice. It takes advantage of the fact that the subvectors are already sorted, but not optimally.

It first reverses the order of the subvectors of words. This is so that we can retrieve the first word (alphabetically) of the subvector without having to remove the first element in the vector, which (I think?) would take longer. It then finds the ”lowest” word (that is, the word that comes first in the sorted list) and checks to make sure it wasn’t the last word printed. Since we are taking the lowest of the first words of the subvectors, we are sure to always have the lowest word of the whole vector. Therefore, we don’t need to check each word against every word that has been printed, but only against the last one printed, since they will never (theoretically) print out of order. It is then printed to the screen and replaced with the last word in the subvector it came from.

This design implementation was intended to be a quicker way to mimic a merge-sort. I thought that it would be quicker to code, and be just as efficient. I then realized that the worst case runtime complexity (according to my calculations) was not optimal. Consider a vector of 100 words split into 10 subvectors. If the lowest 10 words in the whole vector are all in the last subvector, the program ￼￼￼￼￼￼￼￼￼would need to go through nine words to get to it every time. This means for every iteration, it needs ￼￼to go through √N words before finding the smallest.  It must do this √N times, giving a total of N iterations just to find the first √N words.  Then once that subvector is exhausted, if the subvector next to it contains the 10 next lowest words, it must first iterate through √(N-1) words.  Thus, in the case that the number of pipes is close to the square root of the number of words, the worst-case runtime complexity of this one function is N^(3/2) . Merge sort has a worst-case complexity of NlogN.  Therefore, it would have been better to use a merge sort in this function. This may be changed in a later release.
