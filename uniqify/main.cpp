
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_WORD_LENGTH 30 // longest english word length

using namespace std; // TAKE THIS OUT!!!

typedef int fd_t; // file descriptor type

int parse_words(vector< vector<string> > &temp_word_vec);
vector<string>* merge_sort(vector<string> *temp_words_vec);
int write_to_pipe(int pipe_file_desc);
int read_from_pipe(int pipe_file_desc);

int main (int argc, char * argv[])
{	
	int num_of_proc = atoi(argv[1]);
	fd_t fd_arr[num_of_proc];
	fd_t parent_to_child_pipe[2];
	fd_t child_to_parent_pipe[2];
	vector< vector<string> > word_vec(num_of_proc, vector<string>());
	
	if(pipe(parent_to_child_pipe) == -1 || pipe(child_to_parent_pipe) == -1)
	{
		cout << stderr << "Pipe failed. Exiting\n";
		return -1;
	}
	
	parse_words(word_vec);
		
	for(int i = 0; i < num_of_proc; i++)
	{
		if((fd_arr[i] = fork()) == -1)
		{
			return -1;
		}
		
		if(fd_arr[i] == 0) //child process
		{
			if((close(parent_to_child_pipe[0])) == -1)
				return -1;
			   
			if((close(child_to_parent_pipe[1])) == -1)
				return -1;
			
			break;
		}
	}
	
	
	
	
	
	
	/*
	std::vector<std::string> words_vector;
	parse_words(&words_vector);
	merge_sort(&words_vector);
	
	for (vector<string>::iterator i = right_half_words.begin(); i != right_half_words.end(); ++i)
	std::cout << *i;	
	*/
	
	return 0;
}



int parse_words(vector< vector<string> > &temp_word_vec)
{
	char word[3];
	int proc_index = 0; //index of word arr for child process "proc_index"
	
	while (fscanf (stdin, "%s", &word) != EOF)
	{
		temp_word_vec[proc_index++].push_back(word);
		if(proc_index == 3)
			proc_index = 0;
	}
	
	return 0;
}

/*
vector<string>* merge_sort(vector<string> *temp_words_vec)
{
	if(temp_words_vec->size() <= 1){
		return temp_words_vec;
	}
	
	vector<string> left_half_words;
	vector<string> right_half_words;
	vector<string>::iterator halfway_iter = temp_words_vec->begin() + temp_words_vec->size()/2;
	left_half_words.assign(temp_words_vec->begin(), halfway_iter);
	right_half_words.assign(halfway_iter, temp_words_vec->end());
	
	pid_t left_child_pid;
	pid_t right_child_pid;
	int parent_child_pipe[2];
	
	if (pipe(parent_child_pipe))
	{
		cout << stderr << "Pipe failed. Exiting\n";
		exit(EXIT_FAILURE);
	}
	
	left_child_pid = fork();
	if(left_child_pid != 0){
		right_child_pid = fork();
	}
		
	if(left_child_pid == -1 || right_child_pid == -1)
	{
		cout << strerror(errno);
		exit(EXIT_FAILURE);
	}
	
	switch (left_child_pid){
		case 0: //left child process
			clo
			merge_sort(left_half_words);
		default:
			break;
	}
	
	return NULL;
}


int write_to_pipe(int pipe_file_desc)
{
	FILE *pipe_in_stream = fdopen(<#int#>, <#const char *#>)
}



*/