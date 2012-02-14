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
#include <algorithm>
#include <list>
#include <ctype.h>

#define MAX_WORD_LENGTH 30 // longest english word length

using namespace std;

typedef int fd_t; // file descriptor type

int parse_words(vector< vector<string> > &temp_word_vec, int num_of_sub_lists);
int write_to_sort(fd_t pipes_to_sort[][2], vector< vector<string> > temp_word_vec);
vector< vector<string> > read_from_sort(fd_t pipes_to_parent[][2], int num_of_pipes);
void print_sorted_subvectors(vector< vector<string> > &sorted_subsets_vec);
void print_vector(vector<string> &vec_to_print);

int main (int argc, char * argv[])
{
	int num_of_proc = atoi(argv[1]);
	fd_t pipes_to_parent[num_of_proc][2];
	fd_t pipes_to_sort[num_of_proc][2];
	vector< vector<string> > word_vec(num_of_proc, vector<string>());
	vector< vector<string> > sorted_vecs_of_words;
	
	parse_words(word_vec, num_of_proc);
	
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
		
		switch (fork())
		{
			case -1:
			{
				cout << "fork error: " << strerror(errno) << "\n";
				return -1;
				break;
			}
				
			case 0: //child process
			{
				close(pipes_to_parent[i][0]);
				close(pipes_to_sort[i][1]);
				cout.flush();
				
				if (pipes_to_parent[i][1] != STDOUT_FILENO)
				{
					if (dup2(pipes_to_parent[i][1], STDOUT_FILENO) == -1)
					{
						return -1;
					}
				}
				
				if (pipes_to_sort[i][0] != STDIN_FILENO)
				{
					if (dup2(pipes_to_sort[i][0], STDIN_FILENO) == -1)
					{
						cerr << "Pipe error 2: " << strerror(errno) << "\n";
						return -1;
					}
				}
				
				execl("/bin/sort", "sort", (char *) NULL);
				cerr << "sort error: " << strerror(errno) << "\n";
				return -1;
			}
				
			default:	//parent process
			{
				
				close(pipes_to_parent[i][1]);
				close(pipes_to_sort[i][0]);
				
				break;
			}
		}		
	}
	
	write_to_sort(pipes_to_sort, word_vec);
	
	sorted_vecs_of_words = read_from_sort(pipes_to_parent, num_of_proc);
	
	while (wait(NULL) != -1)
	{
		continue;
	}
	if (errno != ECHILD)
	{
		cout << "wait error: " << strerror(errno) << "\n";
		return -1;
	}
	
	for(int j = 0; j < num_of_proc; j++) // close pipes
	{
		close(pipes_to_sort[j][0]);
		close(pipes_to_sort[j][1]);
		close(pipes_to_parent[j][0]);
		close(pipes_to_parent[j][1]);
	}
	
	print_sorted_subvectors(sorted_vecs_of_words);
	
	cout << "success, exiting...\n";
	
	return 0;
}


int parse_words(vector< vector<string> > &temp_word_vec, int num_of_sub_lists)
{
	char word[MAX_WORD_LENGTH+1]; // +1 makes room for a newline character
	int proc_index = 0; //index of word arr for child process "proc_index"
	string word_str;
	
	while(fscanf (stdin, "%s", &word) != EOF)
	{
		for(int i = 0; i < MAX_WORD_LENGTH; i++)
		{
			word[i] = tolower(word[i]);
			if(word[i] == '\0')
			{
				word_str.push_back('\n');
				word_str.push_back('\0');
				break;
			}
			if(isalpha(word[i]))
			{
			   word_str.push_back(word[i]);
			}
		}
		
		temp_word_vec[proc_index++].push_back(word_str);
		if(proc_index == num_of_sub_lists)
			proc_index = 0;
		word_str.clear();
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
			cout.flush();
			if((fputs(temp_word_vec[j][i].c_str(), pipe_write_to_sort[j]) == EOF))
			{
				return -1;
			}
			num_words_wrote++;
		}
		
	}
	
	for(int i = 0; i < temp_word_vec.size(); i++)
	{
		if(fclose(pipe_write_to_sort[i]) == EOF)
		{
			cerr << "error closing pipe: " << strerror(errno) << "\n";
		}
	}
	
	return 0;
}


vector< vector<string> > read_from_sort(fd_t pipes_to_parent[][2], int num_of_pipes)
{
	
	vector< vector<string> > sorted_word_vecs;
	FILE *pipe_read_from_sort[num_of_pipes];
	char word_read[MAX_WORD_LENGTH];
	string word;
	
	for (int i = 0; i < num_of_pipes; i++)
	{
		pipe_read_from_sort[i] = fdopen(pipes_to_parent[i][0], "r");
	}
	
	for(int i = 0; i < num_of_pipes; i++)
	{
		fflush(NULL);
		
		sorted_word_vecs.push_back(vector<string>());
		
		while (fgets(word_read, MAX_WORD_LENGTH, pipe_read_from_sort[i]) != NULL)
		{
			word = word_read;
			sorted_word_vecs[i].push_back(word);
			
		}		
		fclose(pipe_read_from_sort[i]);
		
	}
	
	return sorted_word_vecs;
}


void print_sorted_subvectors(vector< vector<string> > &sorted_subsets_vec)
{	
	vector<string> last_words_in_subvecs; //A list would be better for this, since insertion time is constant.  Change if I have time before deadline.
	vector<string> words_printed;
	string min_word;
	string possible_word;
	int min_word_index;
	int unique_num_of_words = 0;
	int dup_num_of_words = 0;
	int total_num_of_words = 0;
	vector<string>::iterator min_word_iterator;
	
	//reverse order to make removing duplicate words faster
	for (int i = 0; i < sorted_subsets_vec.size(); i++)
	{
		reverse(sorted_subsets_vec[i].begin(), sorted_subsets_vec[i].end());
	}
	
	// go thru last elements of each subvector
	// this is the starting version of first_words vector
	for(int j = 0; j < sorted_subsets_vec.size(); j++)
	{
		possible_word = sorted_subsets_vec[j].back();
		
		// add word to "first words" vector
		last_words_in_subvecs.push_back(possible_word);
	}
	
	while(!last_words_in_subvecs.empty())
	{
		min_word_iterator = min_element(last_words_in_subvecs.begin(), last_words_in_subvecs.end());
		min_word = *min_word_iterator;
		min_word_index = (int)distance(last_words_in_subvecs.begin(), min_word_iterator);			
				
		if(!binary_search(words_printed.begin(), words_printed.end(), min_word))
		{
			cout << min_word;
			unique_num_of_words++;
			words_printed.push_back(min_word);
		}
		else
		{
			dup_num_of_words++;
		}
		total_num_of_words++;
		
		last_words_in_subvecs.erase(min_word_iterator);
		if(!sorted_subsets_vec[min_word_index].empty())
		{
			sorted_subsets_vec[min_word_index].pop_back();
			if(!sorted_subsets_vec[min_word_index].empty())
				last_words_in_subvecs.insert(min_word_iterator, sorted_subsets_vec[min_word_index].back());
		}
	}
	
	cout << "Unique words read: " << unique_num_of_words << "\nDuplicate words read: " << 
	dup_num_of_words << "\nTotal words read: " << total_num_of_words << "\n";
}
