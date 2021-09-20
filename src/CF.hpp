#include <utility>
#include <cstdint> // include this header for uint64_t
using namespace std;



// https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor
// add in the CF.cpp the template class eg:


template <typename key_type> class CF {
		//int ***table;//  memory
		int **keys;//  memory
		int **table;//  memory
		int **table_bit;//  memory
		int   cf_size; // size of CF memory
		int   fp_size;    // 1<<f
		int   num_item;   	// number of inserted item
		int   num_access;   	// number of inserted item
		int   num_slots;   	// number of slots
		int victim_fingerprint;
		int victim_pointer;
		int hash_type;
		bool insert2(key_type key,int p,int fingerprint);

		public:
		CF(int M,int f, int slot, int hash_type_param);
		//CF(int M,int f);
		virtual ~CF();
		void clear();
		void dump();

		bool direct_insert(const key_type key);
		//bool direct_insert(const int p, const int f);
		bool direct_insert(const int p, const int f);
		bool insert(const key_type key);
		bool query(const key_type key);
		std::pair<int,int> fullquery(const key_type key);
		bool cache_query(const key_type key);
		bool check(const key_type key) {return query(key);}
		pair<int,int> get_pf(const key_type key);
		int get_nslots() {return num_slots;}
		int get_nitem() {return num_item;}
		int get_size() {return  num_slots*cf_size;}
		int get_numaccess() {return num_access;}

		private:
		int myhash(key_type key, int i, int s);
};

