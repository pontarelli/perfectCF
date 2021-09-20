#ifndef UTIL_H
#define UTIL_H
#include <getopt.h>
#include <cstdint> // include this header for uint64_t
#include <cstddef>
#include <stdint.h>
#include <stdlib.h>


#include <cstring>
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <unistd.h>
#include "city.h"
#ifdef __SSE4_2__
#include "citycrc.h"


extern int verbose;
#define verprintf(...) if (verbose > 0)  {printf("In file %s, function %s(), line %d: ",__FILE__,__FUNCTION__,__LINE__); printf(__VA_ARGS__);}
#define vverprintf(...) if (verbose > 1)  {printf("In file %s, function %s(), line %d: ",__FILE__,__FUNCTION__,__LINE__); printf(__VA_ARGS__);}
void print_command_line(int argc, char* argv[]);
void print_hostname();

int rot(int64_t key, int i);
int hashg(int64_t key, int i, int s);
void simtime(time_t* starttime_ptr);

#ifndef Max
#define Max(x,y) ((x)>=(y)?(x):(y))
#endif

#ifndef Min
#define Min(x,y) ((x)<=(y)?(x):(y))
#endif


//double genrand();
//void sgenrand(unsigned long seed); /* seed should not be 0 */
//int hash(int64_t item, int j,int max );
//int hash(int64_t item1, int64_t item2, int j,int max );
int64_t RSHash(int64_t key);
int64_t JSHash(int64_t key);


struct five_tuple
{
    uint32_t sip;
    uint32_t dip;
    uint16_t proto;
    uint16_t sp;
    uint16_t dp;

    // equality comparison. doesn't modify object. therefore const.
    bool operator==(const five_tuple& a) const
    {
        return (sip == a.sip && dip == a.dip && proto == a.proto && sp==a.sp && dp==a.dp);
    }
};

int read_IP_from_file(char* filename, int trace_type, five_tuple* ft);

struct command_option
{
#if defined (__STDC__) && __STDC__
  const char *name;
#else
  char *name;
#endif
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
  const char *help_sentence;
};


struct option* convert_options(struct command_option* long_command_options);

#endif

#endif
