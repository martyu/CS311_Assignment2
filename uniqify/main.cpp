#define _POSIX_SOURCE

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sstream>

#define MAX_WORD_LENGTH 30 // longest english word length

using namespace std;

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
	
//	cerr << "point 1\n";
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
//		cerr << "point 2\n";

		switch (fork())
		{
			case -1:
			{
				cout << "fork error: " << strerror(errno) << "\n";
				return -1;
				break;
			}
//			cerr << "point 3\n";

			case 0: //child process
			{
				close(pipes_to_parent[i][0]);
				close(pipes_to_sort[i][1]);
				cout.flush();
//				cerr << "point 4\n";

				if (pipes_to_parent[i][1] != STDOUT_FILENO)
				{
					if (dup2(pipes_to_parent[i][1], STDOUT_FILENO) == -1)
					{
						return -1;
					}
				}
				
//				cerr << "point 5\n";

				if (pipes_to_sort[i][0] != STDIN_FILENO)
				{
					if (dup2(pipes_to_sort[i][0], STDIN_FILENO) == -1)
					{
						cerr << "Pipe error 2: " << strerror(errno) << "\n";
						return -1;
					}
				}
				
//				cerr << "point 6\n";
				
/*				for(int j = 0; j < num_of_proc; j++) // close pipes
				{
					close(pipes_to_sort[j][0]);
					close(pipes_to_sort[j][1]);
					close(pipes_to_parent[j][0]);
					close(pipes_to_parent[j][1]);
				}
*/
				ostringstream oss;
				oss << "output" << i << ".txt";
				string filename = oss.str();
								
				execl("/bin/sort", "sort", (char *) NULL);
				cerr << "sort error: " << strerror(errno) << "\n";
				return -1;
			}
		
				default:	//parent process
			{
				cerr << "point 7\n";

				close(pipes_to_parent[i][1]);
				close(pipes_to_sort[i][0]);
				cerr << "point 8\n";

				break;
			}
		}		
	}
	
	cerr << "point 9\n";

	write_to_sort(pipes_to_sort, word_vec);
	
	cerr << "point 10\n";

	read_from_sort(pipes_to_parent, num_of_proc);
	
	cout << "waiting for children\n";
	
	while (wait(NULL) != -1)
	{
		cerr << "waiting...\n";
		continue;
	}
	if (errno != ECHILD)
	{
		cout << "point7\n";
		cout << "wait error: " << strerror(errno) << "\n";
		return -1;
	}

		
	return 0;
}


int parse_words(vector< vector<string> > &temp_word_vec, int num_of_sub_lists)
{
	char word[MAX_WORD_LENGTH+1]; // +1 makes room for a newline character
	int proc_index = 0; //index of word arr for child process "proc_index"
	int index;
	
	while (fscanf (stdin, "%s", &word) != EOF)
	{
		index = 0;
		while(word[index] != '\0')
		{
			index++;
		}
		word[index] = '\n';
		word[index+1] = '\0';
		temp_word_vec[proc_index++].push_back(word);
		if(proc_index == num_of_sub_lists)
			proc_index = 0;
	}
	
	return 0;
}


int write_to_sort(fd_t pipes_to_sort[][2], vector< vector<string> > temp_word_vec)
{
	FILE *pipe_write_to_sort[temp_word_vec.size()];
	int num_words_wrote = 0;
	int total_words = 0;
	
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
	}

	
	for(int i = 0; num_words_wrote < total_words; i++)
	{
		for (int j = 0; j < temp_word_vec.size() && num_words_wrote < total_words; j++)
		{
			cerr << "temp_word_vec[" << j << "][" << i << "] = " << temp_word_vec[j][i].c_str();
			cout.flush();
			if((fputs(temp_word_vec[j][i].c_str(), pipe_write_to_sort[j]) == EOF))
			{
				return -1;
			}
			num_words_wrote++;
		}
		cout << "number of words wrote to pipe" << i << ": " << num_words_wrote << "\n";

	}
	
	for(int i = 0; i < temp_word_vec.size(); i++)
	{
		cerr << "closing pipe " << pipe_write_to_sort[i] << "\n";
		if(fclose(pipe_write_to_sort[i]) == EOF)
		{
			cerr << "error closing pipe: " << strerror(errno) << "\n";
		}
	}
		
	return 0;
}


int read_from_sort(fd_t pipes_to_parent[][2], int num_of_pipes)
{
	cerr << "point 11\n";

	vector< vector<char*> > sorted_word_vecs;
	FILE *pipe_read_from_sort[num_of_pipes];
	char word_read[MAX_WORD_LENGTH];
//	string word;

	cerr << "point 12\n";
	
	for (int i = 0; i < num_of_pipes; i++)
	{
		cout << "pipes_to_parent[" << i << "][0] = " << pipes_to_parent[i][0] << "\n";
		pipe_read_from_sort[i] = fdopen(pipes_to_parent[i][0], "r");
	}

	cerr << "point 13\n";
	
	for(int i = 0; i < num_of_pipes; i++)
	{
		fflush(NULL);
		cerr << "point 14\n";
		
		cerr << "pipe_read_from_sort[" << i << "] = " << pipe_read_from_sort[i] << "\n";
		
		while (fgets(word_read, MAX_WORD_LENGTH, pipe_read_from_sort[i]) != NULL)
		{
			cerr << "inside while\n";
//			word = word_read;
			
			cerr << "inside while2\n";
		
			cerr << "\n\nword read is " << word_read << "\n\n";
//			sorted_word_vecs[i].push_back(word_read);

			cerr << "inside while3\n";
		}
		cerr << "error is: " << strerror(errno) << "\n";
		
		cerr << "point 15\n";

	}
	
	for(int i = 0; i < sorted_word_vecs.size(); i++)
	{
		for(int j = 0; j < sorted_word_vecs[i].size(); j++)
		{
			cout << "word at [" << i << "][" << j << "] = " << sorted_word_vecs[i][j] << "\n";
		}
	}
	
	cerr << "point 16\n";

	return 0;
}







