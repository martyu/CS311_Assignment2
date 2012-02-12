#define _POSIX_SOURCE

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fstream>


#define MAX_WORD_LENGTH 30 // longest english word length

using namespace std; // TAKE THIS OUT!!!

typedef int fd_t; // file descriptor type

int parse_words(vector< vector<string> > &temp_word_vec, int num_of_sub_lists);
int write_to_sort(fd_t pipes_to_sort[][2], vector< vector<string> > temp_word_vec);
int read_from_sort(fd_t pipes_to_parent[][2], int num_of_pipes);

int main (int argc, char * argv[])
{
	int num_of_proc = atoi(argv[1]);
	fd_t pipes_to_parent[num_of_proc][2];
	fd_t pipes_to_sort[num_of_proc][2];
	vector< vector<string> > word_vec(num_of_proc, vector<string>());
	
	parse_words(word_vec, num_of_proc);
	
	for(int j = 0; j < num_of_proc; j++)
	{
		vector<string> theVec = word_vec[j];
		cout << "vector number " << j << ": ";
		for (vector<string>::iterator i = theVec.begin(); i != theVec.end(); i++)
			std::cout << *i << " ";
		cout << "\n";
	}
	
	for(int i = 0; i < num_of_proc; i++)
	{
		if (pipe(pipes_to_sort[i]) == -1)
		{
			cout << "Pipe error1: " << strerror(errno) << "\n";
			return -1;
		}
		if (pipe(pipes_to_parent[i]) == -1)
		{
			cout << "Pipe error2: " << strerror(errno) << "\n";
			return -1;
		}
		
		cout << "sort pipes " << i << ": \t\t" << pipes_to_sort[i][0] << "\t" << pipes_to_sort[i][1] << "\n";
		cout << "parent pipes " << i << ": \t" << pipes_to_parent[i][0] << "\t" << pipes_to_parent[i][1] << "\n";
		
		switch (fork())
		{
			case -1:
				cout << "fork error: " << strerror(errno) << "\n";
				return -1;
				break;
				
			case 0: //child process
				close(pipes_to_parent[i][0]);
				close(pipes_to_sort[i][1]);
				if (pipes_to_parent[i][1] != STDOUT_FILENO)
				{
//					cout << "point1\n";
					if (dup2(pipes_to_parent[i][1], STDOUT_FILENO) == -1)
					{
						cout << "dup2 error 1: " << strerror(errno) << "\n";
						return -1;
					}
//					cout << "point2\n";
					close(pipes_to_parent[i][1]);
				}
				if (pipes_to_sort[i][0] != STDIN_FILENO)
				{
//					cout << "point3\n";
					if (dup2(pipes_to_sort[i][0], STDIN_FILENO) == -1)
					{
						cout << "Pipe error 2: " << strerror(errno) << "\n";
						return -1;
					}
//					cout << "point4\n";
					close(pipes_to_sort[i][0]);
				}
//				cout << "point5\n";
				execl("bin/sort", "sort");
				cout << strerror(errno) << "\n";
				return -1;
		
			default:	//parent process
//				cout << "point6\n";
				close(pipes_to_sort[i][0]);
				close(pipes_to_parent[i][1]);
				break;
		}		
	}
	while (wait(NULL) != -1)
		continue;
	if (errno != ECHILD)
	{
		cout << "point7\n";
		cout << "wait error: " << strerror(errno) << "\n";
		return -1;
	}
	write_to_sort(pipes_to_sort, word_vec);

	read_from_sort(pipes_to_parent, num_of_proc);
		
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


int write_to_sort(fd_t pipes_to_sort[][2], vector< vector<string> > temp_word_vec)
{
	cout << "point8\n";
	FILE *pipe_write_to_sort[temp_word_vec.size()];
	int num_words_read = 0;
	int total_words = 0;
	cout << "point9\n";
	
	for(int i = 0; i < temp_word_vec.size(); i++)
	{
		for(int j = 0; j < temp_word_vec[i].size(); j++)
		{
			total_words++;
		}
	}
		
	for(int i = 0; i < temp_word_vec.size(); i++)
	{
		if((pipe_write_to_sort[i] = fdopen(pipes_to_sort[i][1], "w")) == NULL)
		{
			cout << "error fdopen: " << strerror(errno) << "\n";
		}
		
		cout << "pipe write to sort: " << pipe_write_to_sort[i] << "\n";

	}
	
	
	cout << "point22\n";

	for(int i = 0; num_words_read < total_words; i++)
	{
		cout << "point23\n";
		cout << "temp word vec size = " << temp_word_vec.size() << "\n";
		for (int j = 0; j < temp_word_vec.size() && num_words_read < total_words; j++)
		{
			cout << "num words read = " << num_words_read << "\ntotal words = " << total_words << "\n";
			cout << "point24\n";
			cout << "temp_word_vec[" << j << "][" << i << "] = " << temp_word_vec[j][i].c_str() << "\n";
			cout << "pipe_write_to_sort[" << j << "] = " << pipe_write_to_sort[j] << "\n";
			fputs(temp_word_vec[j][i].c_str(), pipe_write_to_sort[j]);
			cout << "point25\n";
			num_words_read++;
			cout << "point26\n";
//			fflush(pipe_write_to_sort[j]);
			cout << "point27\n";
		}
	}
		
	return 0;
}


int read_from_sort(fd_t pipes_to_parent[][2], int num_of_pipes)
{
	cout << "point10\n";
	vector< vector<string> > sorted_word_vecs;
	FILE *pipe_read_from_sort[num_of_pipes];
	char word_read[MAX_WORD_LENGTH];
	string word;
	cout << "point11\n";

	for (int i = 0; i < num_of_pipes; i++)
	{

		pipe_read_from_sort[i] = fdopen(pipes_to_parent[i][0], "r");
		
	}
	cout << "point12\n";

	for(int i = 0; i < num_of_pipes; i++)
	{
		cout << "point13\n";
		while (fgets(word_read, MAX_WORD_LENGTH, pipe_read_from_sort[i]) != NULL);
		{
			cout << "point18\n";
			word = word_read;
			cout << "point19\n";
			sorted_word_vecs[i].push_back(word);
			cout << "point20\n";
		}
		cout << "point14\n";
	}
	cout << "point15\n";

	for (int i = 0; i < sorted_word_vecs.size(); i++)
	{
		cout << "point16\n";
		for(int j = 0; j < sorted_word_vecs[i].size(); j++)
		{
			cout << "point17\n";
			cout << sorted_word_vecs[i][j] << "\n";
		}
	}
		 
	
	return 0;
}



