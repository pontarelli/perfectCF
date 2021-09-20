#include "CF.hpp"
#include <iostream>
#include "utils.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <functional>
#include <cstring>
#include <smmintrin.h>
#include "xxhash.h"
#include "murmur3.h"
#include "crc.h"

template <typename T>  
uint64 CityHash(T key, uint64_t seed) 
{
    char* k= (char*) malloc(sizeof(T));
    k = (char*) memcpy(k,&key,sizeof(T));
    uint64 r= CityHash64WithSeed(k,sizeof(T),seed);
    free(k);
    return r;
}

template <typename T>  
uint64 XXHash(T key, uint64_t seed) 
{
    char* k= (char*) malloc(sizeof(T));
    k = (char*) memcpy(k,&key,sizeof(T));

    XXH64_hash_t r=XXH64(k,sizeof(T),seed);
    free(k);
    return (uint64) r;
}

template <typename T>  
uint32_t CRC24Hash(T key) 
{
    uint8_t* k= (uint8_t*) malloc(sizeof(T));
    k = (uint8_t*) memcpy(k,&key,sizeof(T));

    uint32_t r=crc24(k,sizeof(T));
    free(k);
    return (uint32_t) r;
}

	template <typename T>  
uint32_t CRC32Hash(T key, uint32_t seed) 
{
    char* k= (char*) malloc(sizeof(T));
    k = (char*) memcpy(k,&key,sizeof(T));

    uint32_t r=crc32(k,sizeof(T),seed);
    free(k);
    return (uint32_t) r;
}

template <typename T>  
uint64 MurMurHash(T key, uint64_t seed) 
{
    char* k= (char*) malloc(sizeof(T));
    k = (char*) memcpy(k,&key,sizeof(T));
    uint32_t r;
    MurmurHash3_x86_32(k,sizeof(T),seed,&r);
    free(k);
    return r;
}

template <typename T>  
uint32_t NOHash(T key) 
{
    uint8_t* k= (uint8_t*) malloc(sizeof(T));
    k = (uint8_t*) memcpy(k,&key,sizeof(T));

    uint32_t r=k[1]+256*(k[2]+256*k[0]); 
    free(k);
    return (uint32_t) r;
}

template <typename key_type> 
int CF<key_type>::myhash(key_type key, int i, int s) {
    uint64_t   val=0;

    if (hash_type==0)
	    val = CRC32Hash<key_type>(key,444);
    if (hash_type==1)
	    val = MurMurHash<key_type>(key,2137);
    if (hash_type==2)
	    val = XXHash<key_type>(key,2137);
    if (hash_type==3)
	    val = CityHash<key_type>(key,2137);
    if (hash_type==4)
	    val = CRC24Hash<key_type>(key);
    if (hash_type==5)
	    val = NOHash<key_type>(key);
    
    if (i==1) { //fingerprint
    	return (val %s);
    }
    if (i==2) { //pointer
	val = (val / fp_size);     
    	return (val %s);
    }
    if (i==3) { //pointer 2
    	val = XXHash<key_type>(key,2314);
        return (val % s);
    }
    return -1;
}

/*
 * Constructor
 */


extern int verbose; // define the debug level

template <typename key_type>
CF<key_type>::CF(int M,int f, int slot, int hash_type_param)
{
  cf_size=M;
  fp_size=(1<<f);
  num_slots=slot;
  hash_type=hash_type_param;
  if (cf_size>0) {
      keys = new key_type*[M];
      table = new int*[M];
      table_bit = new int*[M];
      for (int i = 0;  i < M;  i++) {
	      keys[i]= new key_type[num_slots]; 
	      table[i]= new int[num_slots]; 
	      table_bit[i]=new int[num_slots]; 
      }
      clear();
  }
}

/*
 * Distructor
 */
template <typename key_type>
CF<key_type>::~CF()
{
  if (cf_size>0) {
	for (int i = 0;  i < cf_size;  i++) {delete[] table[i]; delete[] table_bit[i];}
	delete[] table;
	delete[] table_bit;
  }
}

//DUMP
template <typename key_type>
void CF<key_type>::dump()
{
  if (cf_size>0) {
	for(int i=0; i<cf_size; i++) {
	    for(int ii=0; ii<4; ii++) {
		printf("table[%d][%d] = %d %d\n", i,ii, table[i][ii], table_bit[i][ii]);
	}
	}
  }
}

/*
 * Clear
 */

template <typename key_type>
void CF<key_type>::clear()
{
	num_item=0; num_access=0;
	victim_fingerprint=-9;
	victim_pointer=-1;
  if (cf_size>0) {
	for(int i=0; i<cf_size; i++) {
	    for(int ii=0; ii<4; ii++) {
		    table[i][ii]=-1;
		    table_bit[i][ii]=-10;
	    }
        }
  }
}

/*
 * Insert
 */
template <typename key_type>
bool CF<key_type>::insert(key_type key)
{
  if (cf_size==0) return true;
  if (query(key)) return true;
  int fingerprint=myhash(key,1,fp_size);
  int p=myhash(key,2,cf_size);
  p=p % cf_size;
  return insert2(key,p,fingerprint);
}

template <typename key_type>
bool CF<key_type>::insert2(key_type key,int p,int fingerprint)
{
  int t;
  int newf=-1;
  int new_bit=0;
  int old_bit=0;
  key_type new_key;
  int j=0;
  int jj=0;
  if (cf_size==0) return true;

  for (t = 1;  t <= 1000;  t++) {
      for (jj = 0;  jj < num_slots;  jj++) {
	      p=p % cf_size;
	      verprintf("i2: item in table[%d][%d] for p=%d and f=%u\n",p,jj,p,fingerprint);
	      if (table[p][jj] == -1){
		      keys[p][jj]=key;
		      table[p][jj]=fingerprint;
		      table_bit[p][jj]=new_bit;

		      num_item++;
		      verprintf("inserted in table[%d][%d] f=%u\n",p,jj,fingerprint);
		      return true;
	      }
	      int p1=p^myhash(fingerprint,3,1<<30);
	      p1=p1 % cf_size;
	      verprintf("i2: item in table[%d][%d] for p1=%d and f=%u\n",p1,jj,p1,fingerprint);
	      if (table[p1][jj] == -1) {
		      keys[p1][jj]=key;
		      table[p1][jj]=fingerprint;
		      table_bit[p1][jj]=(1+new_bit)%2;

		      num_item++;
		      verprintf("inserted in table[%d][%d] f=%u\n",p1,jj,fingerprint);
		      return true;
	      }
      } // all place are full
      j = rand() % 2;
      jj = rand() % num_slots;
      p=p^(j*myhash(fingerprint,3,1<<30));
      p=p % cf_size;
      new_key=keys[p][jj];
      newf = table[p][jj];
      old_bit = table_bit[p][jj];

      keys[p][jj]=key;
      table[p][jj]=fingerprint;
      table_bit[p][jj]=(j +new_bit) %2;

      verprintf("inserted %u in table[%d][%d] f=%u bit=%d\n",key,p,jj,fingerprint,j);
      verprintf("moving %u with p=%d and bit %d\n",new_key,p,new_bit);
      fingerprint = newf; // find new home for cuckoo victim
      key=new_key;
      new_bit=old_bit;
  }
  victim_pointer=p;
  victim_fingerprint=fingerprint;
  return false; // insertion failed
}

template <typename key_type>
bool CF<key_type>::direct_insert(key_type key)
{
  if (cf_size==0) return true;
    if (query(key)) {
	printf ("item already here!!!\n");
	return true;
    }
    int fingerprint=myhash(key,1,fp_size);
    int p=myhash(key,2,cf_size);
    p=p % cf_size;
    int jj=0;
    for (jj = 0;  jj < num_slots;  jj++) {
            verprintf("di: item in table[%d][%d] for p=%d and f=%u\n",p,jj,p,fingerprint);
            if (table[p][jj] == -1){
                table[p][jj]=fingerprint;
                num_item++;
                verprintf("inserted in table[%d][%d] f=%u\n",p,jj,fingerprint);
                return true;
            }
            int p1=p^myhash(fingerprint,3,1<<30);
            p1=p1 % cf_size;
            verprintf("i2: item in table[%d][%d] for p1=%d and f=%u\n",p1,jj,p1,fingerprint);
            if (table[p1][jj] == -1) {
                table[p1][jj]=fingerprint;
                num_item++;
                verprintf("inserted in table[%d][%d] f=%u\n",p1,jj,fingerprint);
                return true;
	    }
    } // all place are full
    int j = rand() % 2;
    jj = rand() % num_slots;
    p=p^(j*myhash(fingerprint,3,1<<30));
    p=p % cf_size;
    table[p][jj]=fingerprint;
    verprintf("inserted in table[%d][%d] f=%u\n",p,jj,fingerprint);
    return true;
}

template <typename key_type>
bool CF<key_type>::direct_insert(const int p_in, const int f_in)
{
  if (cf_size==0) return true;
    int fingerprint=f_in; 
    int p=p_in % cf_size;
    int jj=0;
    for (jj = 0;  jj < num_slots;  jj++) {
            verprintf("di: item in table[%d][%d] for p=%d and f=%u\n",p,jj,p,fingerprint);
            if (table[p][jj] == -1){
                table[p][jj]=fingerprint;
                num_item++;
                verprintf("inserted in table[%d][%d] f=%u\n",p,jj,fingerprint);
                return true;
            }
            int p1=p^myhash(fingerprint,3,1<<30);
            p1=p1 % cf_size;
            verprintf("i2: item in table[%d][%d] for p1=%d and f=%u\n",p1,jj,p1,fingerprint);
            if (table[p1][jj] == -1) {
                table[p1][jj]=fingerprint;
                num_item++;
                verprintf("inserted in table[%d][%d] f=%u\n",p1,jj,fingerprint);
                return true;
	    }
    } // all place are full

    int j = rand() % 2;
    jj = rand() % num_slots;
    p=p^(j*myhash(fingerprint,3,1<<30));
    p=p % cf_size;
    table[p][jj]=fingerprint;
    verprintf("inserted in table[%d][%d] f=%u\n",p,jj,fingerprint);
    return true;
}

/*
 * Query
 */

template <typename key_type>
bool CF<key_type>::query(const key_type key)
{
  if (cf_size==0) return false;
    int fingerprint=myhash(key,1,fp_size);
    fingerprint= fingerprint % fp_size;
    int p=myhash(key,2,cf_size);
    if ((fingerprint==victim_fingerprint) && (p==victim_pointer)) {
            verprintf("query item %u in victim for p=%d and f=%d \n",key, p,fingerprint);
	    return true;
    }
    for (int j = 0;  j < 2;  j++) {
        p = myhash(key,2,cf_size)^(j*myhash(fingerprint,3,1<<30));
        p = p % cf_size;
        for (int jj = 0;  jj < num_slots;  jj++) {
            fingerprint= fingerprint % fp_size;
            verprintf("query item %u in table[%d][%d] for p=%d and f=%d (0x%08x) with j=%d\n",key, p,jj,p,fingerprint,fingerprint,j);
            verprintf("result is: %d (0x%08x),%d\n",table[p][jj],table[p][jj],table_bit[p][jj]);
            verprintf("key is: %u\t",keys[p][jj]);
            verprintf("p is %d  p2 is %d key f is %d \n",myhash(keys[p][jj],2,cf_size),myhash(keys[p][jj],2,cf_size)^myhash(table[p][jj],3,1<<30),myhash(keys[p][jj],1,fp_size));
            verprintf("victim is p=%d and f=%d \n",victim_pointer,victim_fingerprint);
            num_access++;
            if ((table[p][jj] == fingerprint) && ((table_bit[p][jj]%2)==j)) {
                return true;
            }
        }
    }
    return false;
}
template <typename key_type>
std::pair<int,int> CF<key_type>::fullquery(const key_type key)
{
  if (cf_size==0) return make_pair(-1,-1);
    //std::cout << "<-->" << key.sip << "," << key.dip << "," << key.proto << "," << key.sp << "," << key.dp <<'\n';
    int fingerprint=myhash(key,1,fp_size);
    fingerprint= fingerprint % fp_size;
    int p=myhash(key,2,cf_size);
    if ((fingerprint==victim_fingerprint) && (p==victim_pointer)) return make_pair(-2,-2);
    for (int j = 0;  j < 2;  j++) {
        p = myhash(key,2,cf_size)^(j*myhash(fingerprint,3,1<<30));
        p = p % cf_size;
        for (int jj = 0;  jj < num_slots;  jj++) {
            fingerprint= fingerprint % fp_size;
            verprintf("query item %u in table[%d][%d] for p=%d and f=%d with j=%d\n",key, p,jj,p,fingerprint,j);
            verprintf("result is: %d,%d\n",table[p][jj],table_bit[p][jj]);
            num_access++;
            if ((table[p][jj] == fingerprint) && ((table_bit[p][jj]%2)==j)) {
                return make_pair(p,jj);
            }
        }
    }
    return make_pair(-1,-1);
}

template <typename key_type>
std::pair<int,int> CF<key_type>::get_pf(const key_type key) {
    int fingerprint=myhash(key,1,fp_size);
    fingerprint= fingerprint % fp_size;
    int p=myhash(key,2,cf_size);
    return make_pair(p,fingerprint);
}

//template class CF<five_tuple>;
template class CF<int>;
