#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <iomanip>      // std::setprecision
#include <algorithm>
#include <functional>
#include <time.h>
#include <stdlib.h>
#include <random> // http://en.cppreference.com/w/cpp/numeric/random

int verbose=0;
#define verprintf(...) {if (verbose > 1) {printf("In file %s, function %s(), line %d: ",__FILE__,__FUNCTION__,__LINE__);} if (verbose > 0) {printf(__VA_ARGS__);}}


using namespace std;

// this class define an entry in the LPMtable
class prefix_type {
	public:
		bool ismarker; //define the entry as a Marker
		bool isprefix; //define the entry as a Prefix
};


int prefix_mask(int ip, int len) {
	if (len==0) return 0;
	int32_t mask = ((1 << (32 - len))-1) ^ 0xFFFFFFFF;
	return (mask & ip);
}


vector< map<int,prefix_type> > LPMtable (33);
vector< map<int,prefix_type> > initial_LPMtable (33);
vector< map<int,prefix_type> > expanded_LPMtable (33);

vector<int> prefix_ar;
vector<int> opcount (33,0);
vector<int> pmcount (33,0);
vector<int> pcount (33,0);

int prefix_count=0;
int num_tables=0;
int max_level=-1;
int max_cap=-1;
int abs_max_cap=-1;
int seed=1;
int cache_size=1024;

// stats for search 
int num_access=0;
int num_access_cache=0;
int num_access_jump=0;

int num_search_std=0;
int num_access_std=0;
int num_search_bmp=0;
int num_access_bmp=0;
int num_search_pm=0;
int num_access_pm=0;



int max_loop=1000;
int max_trial=1;
int tested_tree=-1;
int original_tot_p=0;
int original_24_p=0;
int tot_p=0;



char* filename=NULL; //database filename

bool expand16=true;
int expansion_limit=16;
bool expand24=true;
bool expand32=true;


void PrintUsage() {
   printf("usage:\n");

   printf(" -L expansion limit: set the left tree of the expansion. Root is always /24\n");
   printf(" -f  filename: db to explore\n");
   printf(" -h print usage\n");
   printf(" -v verbose enabled\n");
}

void init(int argc, char* argv[])
{
        seed=time(NULL);
	// Check for switches
        printf("\nParsing options: \n");
	while (argc > 1 && argv[1][0] == '-'){
		argc--;
		argv++;
		int flag=0; //if flag 1 there is an argument after the switch
		int c = 0;
                while ((c = *++argv[0])){
                    switch (c) {
			case 'f':
			    flag=1;
			    filename=strdup(argv[1]); // file to convert
			    argc--;
			    break;
                        case 'L':
                            flag=1;
                            expansion_limit=atoi(argv[1]);
                            argc--;
                            break;
                        case 'v':
                            printf("Verbose enabled\n");
                            verbose += 1;
                            break;
                        case 'h':
                            PrintUsage();
                            exit(1);
                            break;
                        default :
                            printf("Illegal option %c\n",c);
                            PrintUsage();
                            exit(1);
                            break;
                    }
                }
        argv= argv + flag;
    }
}


int run() {

        //open the db
    FILE *fp = NULL;
    fp = fopen(filename, "r");
    if (!fp) {
	printf("file %s non trovato\n",filename);
	return 1;
    }
    printf("db: %s \n",filename);

    //
    //Here we read the db and populate the initial LPMtable with the prefix
    //

    int a,b,c,d;
    int prefix_len=-1;
    int row=0;
    printf("\nCreate the initial LPM table from file\n");
    while( EOF != fscanf(fp,"%d.%d.%d.%d/%d\n",&a,&b,&c,&d,&prefix_len)) {
        verprintf("[%d] read %d.%d.%d.%d/%d\n",++row,a,b,c,d,prefix_len);
        int32_t ip= (a<<24) + (b<<16) + (c<<8) + d;
        verprintf("ip= %d\n",ip);
        if ((prefix_len==-1) && (b==0) && (c==0) && (d==0)) prefix_len=8;
        if ((prefix_len==-1) && (c==0) && (d==0)) prefix_len=16;
        if ((prefix_len==-1) && (d==0)) prefix_len=24;
        if ((prefix_len==-1) ) prefix_len=32;
        ip =prefix_mask(ip,prefix_len);
        verprintf("ip= %d\n",ip);

        prefix_type prefix;
        prefix.ismarker=false;
        prefix.isprefix=true;
        initial_LPMtable[prefix_len].insert(make_pair(ip,prefix));
        opcount[prefix_len]++;
        pcount[prefix_len]++;
        prefix_len=-1;
        original_tot_p++;
    }
    printf("Done.\n");


    expanded_LPMtable=initial_LPMtable;
    
    std::cout << "\nOriginal Database stats\n";
    for( int i=0; i<32; i++){ 
            std::cout  << std::setprecision(2)  << "table LPM(" << i <<") size:" <<expanded_LPMtable[i].size() << "(" << (100*expanded_LPMtable[i].size()/(original_tot_p+0.0))<< "%)\n";
    	    if (i==24) original_24_p= expanded_LPMtable[i].size(); 
    }

    // Here we expand prefix < 16
    if (expand16){
        printf("\nExpand < %d\n",expansion_limit);
        for( int i=0; i<expansion_limit; i++) {
                //for (auto x: expanded_LPMtable[i]) {
                for (map<int,prefix_type>::iterator it = expanded_LPMtable[i].begin(); it != expanded_LPMtable[i].end(); ++it) {
                    pair<int,prefix_type> x = *it;
                if ((expanded_LPMtable[i+1].count(prefix_mask(x.first,i+1))==0)) { //There is nothing in i+1
                    expanded_LPMtable[i+1].insert(make_pair(prefix_mask(x.first,i+1),x.second));
                }
                if ((expanded_LPMtable[i+1].count(prefix_mask(x.first,i+1)+1)==0)) { //There is nothing in i+1
                    expanded_LPMtable[i+1].insert(make_pair(prefix_mask(x.first,i+1)+(1<<(32-i-1)),x.second));
                }
            }
            expanded_LPMtable[i].clear();
        }
    }
    // Here we expand prefix > 17 to /24
    if (expand24 && (expansion_limit<24)) {
	    printf("\nExpand <24\n");
	    for( int i=expansion_limit+1; i<24; i++) {
		    //for (auto x: expanded_LPMtable[i]) {
		    for (map<int,prefix_type>::iterator it = expanded_LPMtable[i].begin(); it != expanded_LPMtable[i].end(); ++it) {
			    pair<int,prefix_type> x = *it;
			    if ((expanded_LPMtable[i+1].count(prefix_mask(x.first,i+1))==0)) { //There is nothing in i+1
				    expanded_LPMtable[i+1].insert(make_pair(prefix_mask(x.first,i+1),x.second));
			    }
			    if ((expanded_LPMtable[i+1].count(prefix_mask(x.first,i+1)+1)==0)) { //There is nothing in i+1
				    expanded_LPMtable[i+1].insert(make_pair(prefix_mask(x.first,i+1)+(1<<(32-i-1)),x.second));
			    }
		    }
		    expanded_LPMtable[i].clear();
	    }
    }
    // Here we expand prefix > 24 to /32
    if (expand32){
	    printf("\nExpand <32\n");
	    for( int i=25; i<32; i++) {
		    //for (auto x: expanded_LPMtable[i]) {
		    for (map<int,prefix_type>::iterator it = expanded_LPMtable[i].begin(); it != expanded_LPMtable[i].end(); ++it) {
			    pair<int,prefix_type> x = *it;
			    if ((expanded_LPMtable[i+1].count(prefix_mask(x.first,i+1))==0)) { //There is nothing in i+1
				    expanded_LPMtable[i+1].insert(make_pair(prefix_mask(x.first,i+1),x.second));
			    }
			    if ((expanded_LPMtable[i+1].count(prefix_mask(x.first,i+1)+1)==0)) { //There is nothing in i+1
				    expanded_LPMtable[i+1].insert(make_pair(prefix_mask(x.first,i+1)+(1<<(32-i-1)),x.second));
			    }
		    }
		    expanded_LPMtable[i].clear();
	    }
    }
        
    // Here we evict tables with 0 prefix
    num_tables=0;
    for( int i=32; i>=0; i--)
        if (expanded_LPMtable[i].size() ==0) {
            expanded_LPMtable.erase(expanded_LPMtable.begin()+i);
        }
        else {
            num_tables++;
            prefix_ar.insert(prefix_ar.begin(),i);
        }

    // compute the total number of prefix (it is tree independent)
    tot_p=0;
    for( int i=0; i<num_tables; i++) {
        tot_p += expanded_LPMtable[i].size();
        opcount[i]=expanded_LPMtable[i].size();
        pcount[i]=expanded_LPMtable[i].size();
    }
    for( unsigned int i=num_tables; i<opcount.size(); i++) {
        opcount[i]=0;
        pcount[i]=0;
    }

    std::cout << "\nDatabase stats\n";
    for( int i=0; i<num_tables; i++){ 
            std::cout  << std::setprecision(2)  << "table LPM(" << prefix_ar[i] <<") size:" <<expanded_LPMtable[i].size() << "(" << (100*expanded_LPMtable[i].size()/(tot_p+0.0))<< "%)\n";
    }

    std::cout << "Total # of tables is " <<num_tables << '\n';
    std::cout << "Original size table LPM is " <<original_tot_p << '\n';
    std::cout << "Total size table LPM is " <<tot_p << '\n';


    LPMtable=expanded_LPMtable;

    // Here we add markers
    printf("\nCreate Markers\n");
    if (expansion_limit<24) {
	    for (auto & x: LPMtable[2]) {
		    if ((LPMtable[1].count(prefix_mask(x.first,prefix_ar[1]))>0)) { //There is already something
			    verprintf("l: %d val:%d\n",prefix_ar[1],prefix_mask(x.first,prefix_ar[1]));
			    prefix_type y=LPMtable[1].at(prefix_mask(x.first,prefix_ar[1]));
			    y.ismarker=true;
			    LPMtable[1][prefix_mask(x.first,prefix_ar[1])]=y;
		    }
		    else {//Nothing exist: this will be a pure marker
			    prefix_type prefix;
			    prefix.ismarker=true;
			    prefix.isprefix=false;
			    LPMtable[1].insert(make_pair(prefix_mask(x.first,prefix_ar[1]),prefix));
		    }
	    }
    }
    else { 
	    for (auto & x: LPMtable[1]) {
		    if ((LPMtable[0].count(prefix_mask(x.first,prefix_ar[0]))>0)) { //There is already something
			    verprintf("l: %d val:%d\n",prefix_ar[0],prefix_mask(x.first,prefix_ar[0]));
			    prefix_type y=LPMtable[0].at(prefix_mask(x.first,prefix_ar[0]));
			    y.ismarker=true;
			    LPMtable[0][prefix_mask(x.first,prefix_ar[0])]=y;
		    }
		    else {//Nothing exist: this will be a pure marker
			    prefix_type prefix;
			    prefix.ismarker=true;
			    prefix.isprefix=false;
			    LPMtable[0].insert(make_pair(prefix_mask(x.first,prefix_ar[0]),prefix));
		    }
	    }
    }



    int tot_i=0;
    for(int i=0; i<num_tables; i++) tot_i += LPMtable[i].size();


    for( int i=0; i<num_tables; i++) {
        std::cout  << std::setprecision(2)  << "table LPM(" <<prefix_ar[i] <<") \t Table size: " <<LPMtable[i].size() << std::endl;
    }


    std::cout << "\n==========================" << std::endl;
    std::cout << "Summary:" << std::endl;
    std::cout << "Original size /24 table is  " << original_24_p << '\n';
    std::cout << "Original size table LPM is " << original_tot_p << '\n';
    std::cout << "Table size table with markers:" << std::endl; 
    for( int i=0; i<num_tables; i++) {
        std::cout  << std::setprecision(2)  << "table LPM(" <<prefix_ar[i] <<") \t Table size: " <<LPMtable[i].size() << std::endl;
    }
    std::cout << "Total size table LPM (with markers) is " <<tot_i << '\n';
       
    return 0;
}

    int main(int argc, char **argv) {
        init(argc,argv);
        return run();
    }

