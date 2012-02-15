/*
 * Original Author: Marty Ulrich (ulrichm)
 * File: main.cpp
 * Created: 2012 February 2, 10:50 by ulrichm
 * Last Modified: 2012 February 13, 23:30 by ulrichm
 * 
 * This file contains functions that use
 * multiple processes to sort a set of words.
 */ 

#define _POSIX_SOURCE

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sstream>
#include <algorithm>
#include <ctype.h>

#define MAX_WORD_LENGTH 30 // longest english word length

typedef int fd_t; // file descriptor type

//Takes in a vector pointer and reuturns it filled with words
int parse_words(std::vector< std::vector<std::string> > &temp_word_vec,
				int num_of_sub_lists);
//Sends the words in temp_word_vec to the sort program
int write_to_sort(fd_t pipes_to_sort[][2], 
				  std::vector< std::vector<std::string> > temp_word_vec);
//Reads in the sorted words
std::vector< std::vector<std::string> > read_from_sort(fd_t pipes_to_parent[][2],
													   int num_of_pipes);
//Prints the words, omitting duplicates
void print_sorted_subvectors(std::vector< std::vector<std::string> > &sorted_subsets_vec);

//Makes pipes
int make_pipes(fd_t pipes_to_parent[][2], fd_t pipes_to_sort[][2], int num_of_proc);

/*
 * ----OVERVIEW----
 * Main function.  Takes in a command line argument that specifies how 
 * many processes to use to sort the input stream of words.  Assumes
 * word list is redirected to stdin.
 * 
 * ----PARAMETERS----
 * Standard C++ "main" parameters.
 *
 * ----NOTABLE FUNCTION VARIABLES----
 * int num_of_proc: number of processes to use to sort words.
 * fd_t pipes_to_parent[][]: holds pipes for info flow to main process from sort program.
 * fd_t pipes_to_sort[][]: holds pipes for info flow to sort processes.
 * vector< vector<std::string> > word_vec: vector to store parsed words to be sorted.
 * vector< vector<std::string> > sorted_vecs_of_words: 
 * stores sorted words returned from sort.
 */

int main (int argc, char * argv[]) {		
	int num_of_proc = atoi(argv[1]); // num of proccesses to use
	fd_t pipes_to_parent[num_of_proc][2];
	fd_t pipes_to_sort[num_of_proc][2];
	std::vector< std::vector<std::string> > word_vec(num_of_proc, 
													 std::vector<std::string>());
	std::vector< std::vector<std::string> > sorted_vecs_of_words;
	
	if(num_of_proc < 1) {
		std::cerr << "Cmd line argument must be a positive integer value.  Exiting...\n";
		return -1;
	}
	
	if(parse_words(word_vec, num_of_proc) < 0) {
		return -1;
	}
	
	if(make_pipes(pipes_to_parent, pipes_to_sort, num_of_proc) < 0) {
		return -1;
	}
	
	if(write_to_sort(pipes_to_sort, word_vec) < 0) {
		return -1;
	}
	
	sorted_vecs_of_words = read_from_sort(pipes_to_parent, num_of_proc);
	if(sorted_vecs_of_words.empty()) {
		return -1;
	}
	
	//wait for children to finish.
	while (wait(NULL) != -1) {
		continue;
	}
	if (errno != ECHILD) {
		std::cerr << "wait error: " << strerror(errno) << "\n";
		return -1;
	}
	
	for(int j = 0; j < num_of_proc; j++) {// close all pipes
		close(pipes_to_sort[j][0]);
		close(pipes_to_sort[j][1]);
		close(pipes_to_parent[j][0]);
		close(pipes_to_parent[j][1]);
	}
	
	print_sorted_subvectors(sorted_vecs_of_words);
				
	return 0;
}


/*
 * ----OVERVIEW----
 * Parses words from stdin into temp_word_vec, returns 0 upon completion.
 * 
 * ----PARAMETERS----
 * temp_word_vec: 2D vector to store sets of parsed words.
 * num_of_sub_lists: number of sets of subvectors to have.
 *
 * ----NOTABLE FUNCTION VARIABLES----
 * word[]: a char array to hold single word read.
 * proc_index: index of word arr for child process.
 * word_str: string to put in temp_word_vec.
 */


int parse_words(std::vector< std::vector<std::string> > &temp_word_vec,
				int num_of_sub_lists) {
	char word[MAX_WORD_LENGTH+1]; // +1 makes room for a newline character
	int proc_index = 0;
	std::string word_str;
	
	while(fscanf (stdin, "%s", &word) != EOF) {
		for(int i = 0; i < MAX_WORD_LENGTH; i++) {
			
			word[i] = tolower(word[i]);
			
			if(word[i] == '\0') {
				if(i == 0) {
					word_str.clear();
					break;
				}
				word_str.push_back('\n');
				word_str.push_back('\0');
				break;
			}
			if(isalpha(word[i])) {
			   word_str.push_back(word[i]);
			}
		}
		
		if(proc_index > num_of_sub_lists || proc_index < 0) {
			proc_index = 0;
		}
		else if(word_str.size() != 0) {
			temp_word_vec[proc_index].push_back(word_str);
			++proc_index;
			if(proc_index == num_of_sub_lists)
				proc_index = 0;
			word_str.clear();
		}
	}
	
	return 0;
}


/*
 * ----OVERVIEW----
 * Sets up pipes to write and read data to and from sort programs.
 * 
 * ----PARAMETERS----
 * pipes_to_parent[][]: 2D array of pipes to read words from sort programs.
 * pipes_to_sort[][]: 2D array of pipes to write words to sort programs.
 *
 * ----NOTABLE FUNCTION VARIABLES----
 * 
 */


int make_pipes(fd_t pipes_to_parent[][2], fd_t pipes_to_sort[][2], int num_of_proc) {
	
	for(int i = 0; i < num_of_proc; i++) {
		if (pipe(pipes_to_sort[i]) == -1) {
			std::cerr << "Pipe error1: " << strerror(errno) << "\n";
			return -1;
		}
		if (pipe(pipes_to_parent[i]) == -1) {
			std::cerr << "Pipe error2: " << strerror(errno) << "\n";
			return -1;
		}
		
		switch (fork()) {
			case -1:
				std::cerr << "fork error: " << strerror(errno) << "\n";
				return -1;
				break;
				
			case 0: //child process
				close(pipes_to_parent[i][0]);
				close(pipes_to_sort[i][1]);
				std::cout.flush();
				
				if (pipes_to_parent[i][1] != STDOUT_FILENO) {
					if (dup2(pipes_to_parent[i][1], STDOUT_FILENO) == -1) {
						return -1;
					}
				}
				
				if (pipes_to_sort[i][0] != STDIN_FILENO) {
					if (dup2(pipes_to_sort[i][0], STDIN_FILENO) == -1) {
						std::cerr << "Pipe error 2: " << strerror(errno) << "\n";
						return -1;
					}
				}
				
				execl("/bin/sort", "sort", (char *) NULL);
				std::cerr << "sort error: " << strerror(errno) << "\n";
				return -1;
				
			default:{ //parent process
				close(pipes_to_parent[i][1]);
				close(pipes_to_sort[i][0]);
				
				break;
			}
		}
	}
	
	return 0;
}


/*
 * ----OVERVIEW----
 * Sends words to sort programs to be sorted.
 * 
 * ----PARAMETERS----
 * pipes_to_sort[][]: 2D array of pipes to feed words to sort program.
 * temp_word_vec: words to be sent to the sort program.
 *
 * ----NOTABLE FUNCTION VARIABLES----
 * pipe_write_to_sort[]: open file stream pointers to write words to sort programs.
 * 
 */


int write_to_sort(fd_t pipes_to_sort[][2],
				  std::vector< std::vector<std::string> > temp_word_vec) {
	FILE *pipe_write_to_sort[temp_word_vec.size()];
	int num_words_wrote = 0;
	int total_words = 0;
	
	for(int i = 0; i < temp_word_vec.size(); i++) {
		for(int j = 0; j < temp_word_vec[i].size(); j++) {
			total_words++;
		}
	}
	
	for(int i = 0; i < temp_word_vec.size(); i++) {
		if((pipe_write_to_sort[i] = fdopen(pipes_to_sort[i][1], "w")) == NULL) {
			std::cerr << "error fdopen: " << strerror(errno) << "\n";
			return -1;
		}
	}
	
	
	for(int i = 0; num_words_wrote < total_words; i++) {
		for (int j = 0; j < temp_word_vec.size() && num_words_wrote < total_words; j++) {
			std::cout.flush();
			if((fputs(temp_word_vec[j][i].c_str(), pipe_write_to_sort[j]) == EOF)) {
				std::cerr << "error fputs: " << strerror(errno) << "\n";
				return -1;
			}
			num_words_wrote++;
		}
		
	}
	
	for(int i = 0; i < temp_word_vec.size(); i++) {
		if(fclose(pipe_write_to_sort[i]) == EOF) {
			std::cerr << "error closing pipe: " << strerror(errno) << "\n";
			return -1;
		}
	}
	
	return 0;
}


/*
 * ----OVERVIEW----
 * Reads back sorted words from sort programs.
 * 
 * ----PARAMETERS----
 * pipes_to_parent[][]: pipes to get words back from sort programs.
 * num_of_pipes: the number of processes running sort program.
 *
 * ----NOTABLE FUNCTION VARIABLES----
 * sorted_word_vecs: vector of vector of sorted words from each process.
 * pipe_read_from_sort[]: open file streams to read back words from sorts.
 * word: holds word read from sort.
 */


std::vector< std::vector<std::string> > read_from_sort(fd_t pipes_to_parent[][2],
													   int num_of_pipes) {
	
	std::vector< std::vector<std::string> > sorted_word_vecs;
	FILE *pipe_read_from_sort[num_of_pipes];
	char word_read[MAX_WORD_LENGTH];
	std::string word;
	
	for (int i = 0; i < num_of_pipes; i++) {
		pipe_read_from_sort[i] = fdopen(pipes_to_parent[i][0], "r");
		if(pipe_read_from_sort[i] == NULL) {
			std::cerr << "fdopen error: " << strerror(errno) << "\n";
			return std::vector< std::vector< std::string > >();
		}
	}
	
	for(int i = 0; i < num_of_pipes; i++) {
		fflush(NULL);
		
		sorted_word_vecs.push_back(std::vector<std::string>());
		
		while (fgets(word_read, MAX_WORD_LENGTH, pipe_read_from_sort[i]) != NULL) {
			word = word_read;
			sorted_word_vecs[i].push_back(word);
		}
		if(fclose(pipe_read_from_sort[i]) == EOF) {
			std::cerr << "fclose error: " << strerror(errno) << "\n";
			return std::vector< std::vector< std::string > >();
		}
	}
	
	return sorted_word_vecs;
}


/*
 * ----OVERVIEW----
 * Goes through 2D vector and prints words in sorted order, omitting duplicates.
 * 
 * ----PARAMETERS----
 * sorted_subsets_vec: 2D vector of sorted subsets of words.
 *
 * ----NOTABLE FUNCTION VARIABLES----
 * last_words_in_subvecs: holds the end element in each subvector.
 * min_word: the lowest compared word in each iteration of last_words_in_subvecs.
 * last_word_printed: keeps the last word printed to compare with next for duplicate.
 * min_word_iterator: stores location in subvec of word being considered.
 * 
 */


void print_sorted_subvectors(std::vector< std::vector<std::string> > &sorted_subsets_vec)
{
	std::vector<std::string> last_words_in_subvecs;
	std::string min_word;
	std::string last_word_printed;
	int min_word_index;
	int unique_num_of_words = 0;
	int dup_num_of_words = 0;
	int total_num_of_words = 0;
	std::vector<std::string>::iterator min_word_iterator;
	
	//reverse order to make removing duplicate words faster
	for (int i = 0; i < sorted_subsets_vec.size(); i++) {
		reverse(sorted_subsets_vec[i].begin(), sorted_subsets_vec[i].end());
	}
	
	// go thru last elements of each subvector
	// this is the starting version of first_words vector
	for(int j = 0; j < sorted_subsets_vec.size(); j++) {		
		// add word to "first words" vector
		last_words_in_subvecs.push_back(sorted_subsets_vec[j].back());
	}
	
	last_word_printed = *min_element(last_words_in_subvecs.begin(),
									 last_words_in_subvecs.end());
	
	while(!last_words_in_subvecs.empty()) {
		min_word_iterator = min_element(last_words_in_subvecs.begin(),
										last_words_in_subvecs.end());
		min_word = *min_word_iterator;
		min_word_index = (int)distance(last_words_in_subvecs.begin(), min_word_iterator);			
				
		if(min_word != last_word_printed || total_num_of_words == 0) {
			std::cerr << min_word;
			last_word_printed = min_word;
			unique_num_of_words++;
		}
		else {
			dup_num_of_words++;
		}
		total_num_of_words++;
		
		last_words_in_subvecs.erase(min_word_iterator);
		if(!sorted_subsets_vec[min_word_index].empty()) {
			sorted_subsets_vec[min_word_index].pop_back();
			
			if(!sorted_subsets_vec[min_word_index].empty()) {
				last_words_in_subvecs.insert(min_word_iterator,
											 sorted_subsets_vec[min_word_index].back());
		
			}
		}
	}
	
	std::cout << "Unique words read: " << unique_num_of_words << 
				 "\nDuplicate words read: " << dup_num_of_words << 
				 "\nTotal words read: " << total_num_of_words << "\n";
}
