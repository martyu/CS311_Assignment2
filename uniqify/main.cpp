
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

int parse_words(vector< vector<string> > &temp_word_vec, int num_of_sub_lists);
vector<string>* merge_sort(vector<string> *temp_words_vec);
int write_to_sort(fd_t pipes_to_sort[][2], int num_of_pipes);
int read_from_sort(fd_t pipes_to_parent[][2], int num_of_pipes);

int main (int argc, char * argv[])
{
	
	int num_of_proc = atoi(argv[1]);
	fd_t pipes_to_parent[num_of_proc][2];
	fd_t pipes_to_sort[num_of_proc][2];
	vector< vector<string> > word_vec(num_of_proc, vector<string>());
	
	parse_words(word_vec, num_of_proc);
	
	for(int j = 0; j < 3; j++)
	{
		vector<string> theVec = word_vec[j];
		cout << "vector number " << j;
		for (vector<string>::iterator i = theVec.begin(); i != theVec.end(); ++i)
			std::cout << *i << " ";
		cout << "\n";
	}

	
	for(int i = 0; i < num_of_proc; i++)
	{
		if (pipe(pipes_to_sort[i]) == -1)
		{
			return -1;
		}
		switch (fork())
		{
			case -1:
				return -1;
				break;
				
			case 0: //child process
				if (pipes_to_parent[i][0] != STDOUT_FILENO)
				{
					if (dup2(pipes_to_parent[i][0], STDOUT_FILENO) == -1)
					{
						return -1;
					}
					close(pipes_to_parent[i][0]);
				}
				if (pipes_to_sort[i][1] != STDIN_FILENO)
				{
					if (dup2(pipes_to_sort[i][1], STDIN_FILENO) == -1)
					{
						return -1;
					}
					close(pipes_to_sort[i][1]);
				}
				execl("bin/sort", "sort");
				break;
		
			default:	//parent process
				close(pipes_to_sort[i][1]);
				close(pipes_to_parent[i][0]);
				write_to_sort(pipes_to_sort, num_of_proc);
				while (wait(NULL) != -1)
					continue;
				if (errno != ECHILD)
				{
					return -1;
				}
				break;
		}
				
	}
	
		
	return 0;
}



int parse_words(vector< vector<string> > &temp_word_vec, int num_of_sub_lists)
{
	char word[15];//CHANGE TO STRING!#$%#$%#@$%@#$%
	int proc_index = 0; //index of word arr for child process "proc_index"
	
	while (fscanf (stdin, "%s", &word) != EOF)
	{
		temp_word_vec[proc_index++].push_back(word);
		if(proc_index == num_of_sub_lists)
			proc_index = 0;
	}
	
	return 0;
}


int write_to_sort(fd_t pipes_to_sort[][2], int num_of_pipes)
{
	FILE *pipe_write_to_sort[num_of_pipes][2];
	
	for(int i = 0; i < num_of_pipes; i++)
	{
		pipe_write_to_sort[i][0] = fdopen(pipes_to_sort[i][0], "w");
	}
	while (true)
	{
		numRead = read(pfd[0], buf, BUF_SIZE);
		if (numRead == -1)
			errExit("read");
		if (numRead == 0)
		break; /* End-of-file */ if (write(STDOUT_FILENO, buf, numRead) != numRead)
			fatal("child - partial/failed write");)
	}
	
	
	return 0;
}


