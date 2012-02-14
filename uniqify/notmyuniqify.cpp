/*
 * Riley Hickman
 * CS 311 - HW 2
 * uniqify.cpp: 
 * 	Reads: from stdin then pipes argv[1] times to sort and then prints a 
 * 	list of unique words and count of unique words from stdin, delimited 
 *	by non alphabetic characters
 */

#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>


#ifndef BUF_SIZE
#define BUF_SIZE 100
#endif

//prototypes

int make_babies(int num, int fd_in[][2], int fd_out[][2],
			 FILE *strm_sort[], FILE * strm_supress[]);
void close_all_pipes(int num_sorts, int fd_in[][2], int fd_out[][2]);
void close_all_pipes(int nu_sorts, int fd[][2]);
int feed_babies(int num, FILE *strm_in[], int fd_in[][2]);
int eat_babies(int num, FILE *strm_out[], int fd_out[][2]);
int parse(char buf[]);
int find_first(int num, char buf[][BUF_SIZE]);
void copy_word(char from[], char to[]);

/*
 * Main:
 * 	performs program description
 */


int main (int argc, char *argv[]){
		
		//check input
	if(argc != 2 || argv[1] == "--help"){
		std::cout << "./uniqify (int)num_sort <input" ; 
	}
	int num_sorts = 0;
	if(isalpha(*argv[1]) == 0){
		if(isalnum(*argv[1])){
    			num_sorts = atoi(argv[1]);
    		}
	}else{
      		std::cout << "\"" << argv[1] << "\" is not a valid number\n";
      		return -1;
    	}
	if (num_sorts <= 0){
		std::cout << "\"" << argv[1] << "\" Needs to be > 0\n";
		return -1;
	} 

	int fd_in[num_sorts][2];
	int fd_out[num_sorts][2];
	FILE *strm_sort[num_sorts];
	FILE *strm_sup[num_sorts];
	
		// job make babies
	if( make_babies(num_sorts, fd_in, fd_out, strm_sort, strm_sup) == -1){
		std::cout << "Error make_babies\n";
		return -1;
	}

		// parse to babies
	if( feed_babies(num_sorts, strm_sort, fd_in) == -1){
		std::cout << "Error feed_babies\n";
		return -1;
	}
		// get sorted words from babies then suppress
	if( eat_babies(num_sorts, strm_sup, fd_out) == -1){
		std::cout << "Error eat_babies\n";
		return -1;
	}

	//std::cout << "made it to the end!!!\n";
	return 0;

}


/*
 * make_babies:
 *	creates all pipes(in and out), streams (in and out), and children sorts
 */
int make_babies(int num, int fd_in[][2], int fd_out[][2],
			 FILE *strm_sort[], FILE * strm_sup[]){

	//create all my pipes
	int j;
	for(j = 0 ; j < num; j++){
		if( (pipe(fd_in[j]) || pipe(fd_out[j])) == -1){
			std::cout << "Error pipes\n";
			return -1; 
		} 
	}

	//make children sorts
	for(j = 0; j< num; j++){
	
		switch(fork()){

			case -1:
				std::cout << "There was a forking error\n";		
			case 0:	
				close(fd_in[j][1]);
				close(fd_out[j][0]);
				std::cout.flush();

				dup2(fd_in[j][0], 0);
				dup2(fd_out[j][1], 1);
				close_all_pipes(num, fd_in, fd_out);	
				
				//execl("/usr/bin/sort", "sort", "-o" , "sort_out.txt", (char *)NULL);
				execl("/bin/sort", "sort", (char *)NULL);
				std::cout << "exec didnt work\n";//should not make it here
			 	_exit(3);	
			default:	
				close(fd_in[j][0]);
				close(fd_out[j][1]);
				continue;
		}
	}
	//create streams
	for(j = 0; j < num; j++){

		strm_sort[j] = fdopen(fd_in[j][1], "a");
		strm_sup[j] = fdopen(fd_out[j][0], "r");

	}
}

/*
 * close_all_pipes:
 * 	closes all pipes from fd
 */

void close_all_pipes(int num_sorts, int fd[][2]){
	int i;
	for(i = 0; i < num_sorts; i++){
		close(fd[i][1]);
		close(fd[i][0]);
	}

	return;


}


/*
 * close_all_pipes:
 * 	closes all pipes from fd
 */

void close_all_pipes(int num_sorts, int fd_in[][2], int fd_out[][2]){
	int i;
	for(i = 0; i < num_sorts; i++){
		close(fd_in[i][1]);
		close(fd_in[i][0]);
		close(fd_out[i][1]);
		close(fd_out[i][0]);
	}

	return;
}
/*
 * feed_babies:
 *	parses words from stdin to streams round robin then closes in streams
 */


int feed_babies(int num, FILE *strm_sort[],int fd_in[][2]){

	//parse one word and put to baby stream
	char buf[BUF_SIZE];
	int i, pos;
	i = 0;
	while(parse(buf) != 1){
		std::cout.flush();	
		pos = i % num;
		if(fputs(&buf[0], strm_sort[pos]) < 0){
			std::cout << "fputs error\n";
			return -1;
		}
		//std::cout << "Word " << i << " successful\n";
		i++;

	}	
	
	for(i = 0; i < num; i++){
		//std::cout << "Closing stream " << i << "\n";
		if(fclose(strm_sort[i]) == -1){
			std::cout << "Error closing stream\n";
			return -1;
		}
	}
	
	close_all_pipes(num, fd_in);
	return 0;
}

/*
 * parse:
 * 	puts the next word delimited by non alpha to buf
 */

int parse(char buf[]){ 	
	char vampire[BUF_SIZE]; //temporary char[]
	
	// first scan removes delimiters then second one gets what we want into vamp
	 
	if(fscanf(stdin, "%*[^a-zA-Z]"), fscanf(stdin, "%[a-zA-Z]s", &vampire[0]) < 0 ){	
        	return 1; // end has been reached
	}

    	//std::cout << vampire[0] << vampire[1] << vampire[2] << vampire[3] << " " << pos << " In parse\n";

    	int slayer;
    	int e;
    	e  = 0;
	
    	for (slayer = 0; slayer < BUF_SIZE; slayer++){

        	if (vampire[slayer] == '\0'){

            		break;

        	}else if(isalpha(vampire[slayer])){ //defensive check for alpha char

            		vampire[slayer] = tolower(vampire[slayer]);

            		buf[e] = vampire[slayer];//haha funny! atleast I thought so....
            		e++;
        	}
    	}
	buf[e++] = '\n';//for sorts and fputs deleimiter
	buf[e] = '\0';//to signify the end of the string
    	

    	return 0;  //success
}

/*
 * eat_babies:
 * 	waits for all children processes to term
 * 	gets words from out streams and prints uniqiue words
 * 	in alpha order
 */

int eat_babies(int num, FILE *strm_sup[], int fd_out[][2]){
	int body_count;
	for(body_count = 0; body_count < num; body_count++){
		wait(NULL);//wait for all children to terminate
	}
	char buf[num][BUF_SIZE];
	int i;
	int count;
	count = 0;
	
	//store the first word from each stream to buf
	for(i = 0; i < num; i++){
		if(fgets(buf[i], BUF_SIZE, strm_sup[i]) == (char *) NULL ){
			buf[i][0] = '\0';	
		}
		//std::cout << buf[i];
		//std::cout.flush();
	} 
	int pos;
	char prev[BUF_SIZE];
	//std::cout << "first pos  " << find_first(num, buf) << "\n";
	// finish loop when all streams are empty
	int j = 0;
	while( ( pos = find_first(num, buf) ) != -1){
		//std::cout << "Pass " << j++ << "\n";
		if(count == 0){
			copy_word(buf[pos], prev);
			std::cout << prev;
			std::cout.flush();
			if(fgets(buf[pos], BUF_SIZE, strm_sup[pos]) == (char *) NULL ){
				buf[pos][0] = '\0';
			} 
			count++;
			continue;
		}		
		if(strcmp(prev, buf[pos]) == 0){ //next is identical to prev thus ignore
			if(fgets(buf[pos], BUF_SIZE, strm_sup[pos]) == (char *) NULL ){

				buf[pos][0] = '\0';	
			}
		}else { //must be the next one 
			
			copy_word(buf[pos], prev);
			std::cout << prev;
			std::cout.flush();
			if(fgets(buf[pos], BUF_SIZE, strm_sup[pos]) == (char *) NULL ){
				buf[pos][0] = '\0';
			}
			count++;
		}	
	
	}
	std::cout << "There were " << count << " unique words\n";
		
	std::cout.flush();
			
	for(i = 0; i < num; i++){
		if(fclose(strm_sup[i]) == -1){
			std::cout << "Error closing stream\n";
			return -1;
		}
	}
	close_all_pipes(num, fd_out);
	return 0;
}


/*
 * find_first:
 *	will find the first determined alphabetically from buf
 *	if all entrys are NULL'ed then returns -1 otherwise
 *	 returns pos of first
 */

int find_first(int num, char buf[][BUF_SIZE]){
	int pos = -1;
	int j;
	int comp;
	int first = 1;	

	for(j = 0; j < num; j++){
		//if every word from every stream is '\0' then all streams are done
		if( buf[j][0] == '\0'){
			continue;
		}
		if(first == 1){
		// gets the first word from stream available
			pos = j;
			first = 0;
			continue;
		}
		comp = strcmp(buf[pos], buf[j]);
		if(comp <= 0){//first is less than buf or equal
			continue;
		}else{//buf[j] is less than first
			pos = j;

		}

	}
	return pos;
		
}

/*
 * copy_word:
 *	copys from (from) to  (to) until '0' is reached, transfers that as well
 */


void copy_word(char from[], char to[]){

	int i;
	for(i = 0; i < BUF_SIZE; i++){

		if(from[i] == '\0'){
			to[i] = '\0';
			break;
		}else {
			to[i] = from[i];
		}
	}
	return;
}



