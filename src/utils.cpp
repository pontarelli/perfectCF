#include "utils.h"
#include <iostream>
#include <smmintrin.h>
#include <stdint.h>


void print_hostname() {
    char hostname[1024];
    hostname[1023] = '\0';
#ifndef _WIN32
    gethostname(hostname, 1023);
#endif
    printf("[%s]: ", hostname);
}


void print_command_line(int argc, char* argv[]) {
	//print command line
	printf("command executed with command line: ");
	char **currentArgv = argv;
	for (int i = 0; i < argc; i ++) {
		printf("%s ", *currentArgv); /* %s instead of %c and drop [i]. */
		currentArgv++; /* Next arg. */
	}
	printf("\n");
}

int rot(int64_t key, int i)
{
    return (key << i)| (key) >>(64-i);
}

inline uint64 CityHash64WithSeed(int64_t key, uint64_t seed)
{
 return CityHash64WithSeed((const char *)&key,8,seed);
}


// calculate computing time
void simtime(time_t* starttime_ptr) {
    time_t endtime = time(NULL);
    struct tm * timeinfo=localtime(starttime_ptr);
    double second = difftime(endtime,*starttime_ptr);
    printf("simulation started @: %s \n", asctime(timeinfo));
    timeinfo=localtime(&endtime);
    printf("simulation ended   @: %s \n",asctime(timeinfo));
    printf("simulation time: %f sec \n",second);
}

struct option* convert_options(struct command_option* long_command_options)
 {
	 int num_options=0;
         while (long_command_options[num_options].name !=NULL ) num_options++;
	 struct option* result= ( struct option* ) malloc(num_options*sizeof(option));
	 int i=0;
	 while (long_command_options[i].name !=NULL ) {
                result[i].name = long_command_options[i].name;
                result[i].has_arg = long_command_options[i].has_arg;
                result[i].flag = long_command_options[i].flag;
                result[i].val = long_command_options[i].val;
		i++;
	 }
	 return result;
}
