
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <string.h>

#define MAX_WORD_LENGTH 30 // longest english word length

void parse_words(std::vector<std::string> *temp_words_vec);

int main (int argc, char * argv[])
{	
//	int num_of_proc = atoi(argv[1]);
		
	std::vector<std::string> words_vector;

	parse_words(&words_vector);
	
	for (std::vector<std::string>::iterator i = words_vector.begin();
		 i != words_vector.end();
		 ++i)
	{
		std::cout << *i << "\n";
	}
	
	return 0;
}


void parse_words(std::vector<std::string> *temp_word_vec)
{
	char temp_word[MAX_WORD_LENGTH];
	
	while(std::fgets(temp_word, MAX_WORD_LENGTH, stdin))
	{
		for(int i = 0; i < strlen(temp_word); i++)
			temp_word[i] = tolower(temp_word[i]);
				
		word_vec->push_back(temp_word);
	}
}