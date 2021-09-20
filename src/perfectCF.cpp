#include <iostream>
#include <vector>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "utils.h"
#include <math.h>
#include <signal.h>
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <getopt.h>
#include <iomanip>
#include <map>
#include "CF.hpp"
#include <algorithm>
#include <math.h>       /* log2 */

bool start=false;
int currentfile=0;
int trace_type=0;
int64_t max_loop=10;
int hash_type=0;
int fingerprint=8;
int universe_bits=32;
bool exaustive=false; 
bool false_positive=false; 

bool pcap_flag =true;
int offset[3]={14,18,0};

char *mybasename(char *path)
{
    char *s = strrchr(path, '/');
    if (!s)
        return strdup(path);
    
    char *s2 = strdup(s + 1);
    s = strchr(s2, '.');
    *s='\0';
    return strdup(s2);
}

int next_power_of_two(int n) {
    int i = 0;
    for (--n; n > 0; n >>= 1) {
        i++;
    }
    return 1 << i;
}



void PrintUsage( struct command_option* long_options);
struct option* long_options;
int cf_rows =1024; // number of elements for each table
unsigned long seed; // value for initialization of random function
bool quiet=false;
bool multiplepcap=false;

int num_queries=0; // number of queries for lookup estimation
int verbose; // set verbose level

// Init function. It is used to initialize the hash tables structures

void init(int argc, char **argv) {
    verbose=0;
    seed=time(NULL); // value for initialization of random function
    //print code version
    char str[512];
    sprintf(str,"%s",MD5);
    //print command line
    printf("Compiled @: %s \n",COMPILE_TIME);
#ifdef __SSE4_2__
    printf("With crc hash from Intel SSE4.2 \n");
#else
    printf("With software crc hash function \n");
#endif

    // Check for switches
    int long_index =0;
    int opt= 0;
    bool set_fingerpriint=false;

    static struct command_option long_command_options[] = {
	//{"option", optional_argument , flag, 'o'} //flag specifies how results are returned for a long option. 
	{"loops",               required_argument,  0,  'L', "number of trials"},
	{"fingerprint",         required_argument,  0,  'f', "force the number of fingerprint bits. default f= u-b"   },
	{"universe",            required_argument,  0,  'u', "number of bits u of the universe U" },
	{"quiet",               no_argument,        0,  'Q', "quiet"  },
	{"seed",                required_argument,  0,  's', "Initial random seed"  },
	{"size",                required_argument,  0,  'm', "number of rows in a table" },
	{"bsize",               required_argument,  0,  'M', "set number of bits b to address a row in the table" },
	{"hash",                required_argument,  0,  'H', "type of hash to use. 0: CRC 1: MurMur 2: XXhash 3: Cityhash 4: CRC24"},
	{"exaustive",           no_argument,        0,  'e', "do exaustive check querying all the elements in U"  },
	{"false_positive",      no_argument,        0,  'F', "evaluate false positive rate with a subset of elements"  },
	{"verbose",             no_argument,        0,  'v', "verbose"  },
	{"help",                no_argument,        0,  'h', "print help "  },
	{0,                     0,                  0,   0 , ""  }
    };
    long_options = convert_options(long_command_options);

    while ((opt = getopt_long_only(argc, argv,"", long_options, &long_index )) != -1) {
	switch (opt) {
	    case 'L':
		max_loop = atoi(optarg); // number of loops
		break;
	    case 'f':
		set_fingerpriint=true;
		fingerprint = atoi(optarg); // number of fingerprint bits
		break;
	    case 'u':
		universe_bits = atoi(optarg); // number of bits for U
		break;
	    case 'Q':
		quiet=true; 
		break;
	    case 'm':
		cf_rows = atoi(optarg); // number of rows
		break;
	    case 'M':
		cf_rows = 1<<atoi(optarg); // number of rows
		break;
	    case 's':
		seed = atoi(optarg); // seed for debug
		break;
	    case 'H':
		hash_type= atoi(optarg); // hash type to use
		break;
	    case 'F':
		printf("\nEvaluate False positive rate\n");
		false_positive=true;
		break;
	    case 'e':
		printf("\nExaustive check enabled\n");
		exaustive=true;
		break;
	    case 'v':
		printf("\nVerbose enabled\n");
		verbose += 1;
		break;
	    case 'h':
		PrintUsage(long_command_options);
		exit(1);
		break;
	    default :
		printf("Illegal option\n");
		PrintUsage(long_command_options);
		exit(1);
		break;
	}
    }

    printf("With command line: ");
    char **currentArgv = argv;
    for (int i = 0; i < optind; i ++) {
	printf("%s ", *currentArgv); 
	currentArgv++; 
    }
    printf("\n");

    if (!set_fingerpriint)
	    fingerprint = universe_bits-log2(cf_rows); 

    printf("\n ------------------ \n");
    printf("With seed: %lu \n",seed);

    free(long_options);
} //end init


// this function runs the simulation
void run() {

    int64_t tot_i=0;
    time_t starttime,endtime;
    starttime= time(NULL);
    int insertion_failure=0;

    // main loop

    CF<int> perfectCF(cf_rows,fingerprint,4,hash_type);
    map<int,int> test_map;
    vector<int> tot_ar;

    perfectCF.clear();    
    test_map.clear();
    tot_ar.clear();

    
    printf("***:PERFECT CF:\n");
    printf("***:size: %d\n",cf_rows);
    printf("***:fingerprint bits: %d\n",fingerprint);
    printf("***:Total size: %d\n",perfectCF.get_size());
    printf("***:Total size (bits): %d\n",8*perfectCF.get_size());
    printf("***:---------------------------\n");
    setbuf(stdout, NULL);

    tot_i = 95*perfectCF.get_size()/100;
    int tot_count_collision=0;
    uint64_t max_test= 1UL<<(universe_bits);
    for (int loop=0; loop<max_loop; loop++) {
	    srand(seed++);

	    perfectCF.clear();    
	    test_map.clear();
	    tot_ar.clear();

            for (int64_t i = 0;  i <tot_i;  i++)
            {
                unsigned int key= (rand()*2^16)+rand();
		if (universe_bits<32) key = key % (1<<universe_bits);
                if (test_map.count(key)>0) {
                    i--;
                    continue;
                }

                test_map[key]=i;
                verprintf("insert key: %u \n",key);
                if ((i%1000)==0) {
                    if (!quiet) fprintf(stderr,"loop: %d item: %lu\r",loop,i);
                }

                // insert in perfect CF
                if(perfectCF.query(key))
                {
                    verprintf(" CF collision (key: %u)\n",key);
                    verprintf("Hash function is not bijective\n");
                    i--;
                    continue;
                }

                if(!perfectCF.insert(key))
                {
                    printf(" CF full (key: %u)\n",key);
                    printf("seed: %lu\n",seed);
		    insertion_failure++;
		    break;
                }
            }
            if (!quiet) fprintf(stderr, "\n");
            printf("End insertion\n");
            printf("---------------------------\n");
            printf("items= %d\n",perfectCF.get_nitem());
            printf("load(%d)= %f \n",loop,perfectCF.get_nitem()/(0.0+perfectCF.get_size()));
	    int count_collision=0;
	    if (exaustive){
		    for (uint64_t test=0; test<max_test; test++) {
			    if (!quiet) fprintf(stderr,"loop: %d item: %lu (0x%08lx)\r",loop,test,test);
			    if (test_map.count(test)>0) {
				    continue;
			    }
			    if(perfectCF.query(test)){
				    verprintf(" CF collision (key: %lu)\n",test);
				    count_collision++;
			    }
			    if ((test%1000)==0) {
				    if (!quiet) fprintf(stderr,"loop: %d item: %lu\r",loop,test);
			    }
		    }
		    printf("\ncollisions for loop %d are %d\n",loop,count_collision);
		    printf("\nFPR for loop %d is %.4f\n",loop,count_collision/(max_test+0.0));
		    tot_count_collision += count_collision;
	    }
	    if (false_positive){
		    printf("\nStart False positive rate evaluation\n");
		    int count_collision=0;
		    for (uint64_t test=0; test<10000; test++) {
			    unsigned int key= (rand()*2^16)+rand();
			    if (test_map.count(key)>0) {
				    test--;
				    continue;
			    }
			    if(perfectCF.query(key)){
				    printf(" CF collision (key: %u)\n",key);
				    count_collision++;
			    }
		    }
		    printf("\ncollisions for loop %d are %d\n",loop,count_collision);
		    printf("\nFPR for loop %d is %.8f\n",loop,count_collision/(10000+0.0));
		    printf("End False positive rate evaluation\n\n");
	    }
    }
    endtime = time(NULL);
    printf("Insertion failures: %d of %ld \n",insertion_failure,max_loop);
    printf("\nFPR is %.8f (%d out of %ld)\n",tot_count_collision/(max_loop*max_test+0.0),tot_count_collision,max_loop*max_test);
    struct tm * timeinfo=localtime(&starttime);
    double second = difftime(endtime,starttime);
    std::cout << "\nsimulation started @: " << asctime(timeinfo) << std::endl;
    timeinfo=localtime(&endtime);
    std::cout << "simulation ended   @: " << asctime(timeinfo) << std::endl;
    std::cout << "simulation time: " << second << " sec" << std::endl;
}




int main(int argc, char **argv) {
    init(argc,argv);
    run();
}

// Function to print the program usage

void PrintUsage( struct command_option* long_options){
    printf("\n");
    int i=0;
    printf("Usage: perfectCF ");
    while (long_options[i].name !=NULL ) {
	if (long_options[i].has_arg == required_argument) printf("[--%s N] ",long_options[i].name);
	else printf("[--%s] ",long_options[i].name);
	i++;
    }
    printf("\n\n");
    i=0;
    while (long_options[i].name !=NULL ) {
	std::string s;
	if (long_options[i].has_arg == required_argument)  
	    s= "[--" +  std::string(long_options[i].name) +  " N]";
	else 
	    s= "[--" +  std::string(long_options[i].name) +  "]";
	std::cout << std::left <<  std::setw(30) << s;
	std::cout << std::left << long_options[i].help_sentence << std::endl;
	i++;
    }
}


