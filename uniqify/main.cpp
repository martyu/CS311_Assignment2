
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

int parse_words(std::vector<std::string> *temp_words_vec);
vector<string>* merge_sort(vector<string> *temp_words_vec);

int main (int argc, char * argv[])
{	
//	int num_of_proc = atoi(argv[1]);
	
	std::vector<std::string> words_vector;
	parse_words(&words_vector);
	merge_sort(&words_vector);
	
	/*
	for (vector<string>::iterator i = right_half_words.begin(); i != right_half_words.end(); ++i)
	std::cout << *i;	
	*/
	
	return 0;
}


int parse_words(std::vector<std::string> *temp_word_vec)
{
	char temp_word[MAX_WORD_LENGTH];
	
	while(std::fgets(temp_word, MAX_WORD_LENGTH, stdin))
	{
		for(int i = 0; i < strlen(temp_word); i++)
			temp_word[i] = tolower(temp_word[i]);
		
		temp_word_vec->push_back(temp_word);
	}
	
	int errChk = ferror(stdin);
	if(errChk != 0){
		perror("There was an error");
		return errChk;
	}
	
	return 0;
}


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



