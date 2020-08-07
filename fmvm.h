/* Multi-Value Map ADT : via Double Hashing with Linear Probing
   Both key & data are strings (char*)
   Multiple Data with the same key are stored in a linked list
   New data is inserted at the front of the linked list:
   O(1) insertion
   O(1) search
   O(1) deletion
*/

/* Error that can't be ignored */
#define ON_ERROR(STR) fprintf(stderr, STR); exit(EXIT_FAILURE)

struct mvmcell {
   char* key;
   char* data;
   struct mvmcell* next;
};
typedef struct mvmcell mvmcell;

struct htable {
   mvmcell** arr;
   int* numdata;
   int numkeys;
   int capacity;
};
typedef struct htable htable;

struct mvm {
   htable* hash;
};
typedef struct mvm mvm;

mvm* mvm_init(void);
/* Number of key/value pairs stored */
int mvm_size(mvm* m);
/* Insert one key/value pair */
void mvm_insert(mvm* m, char* key, char* data);
/* Store list as a string "[key](value) [key](value) " etc.  */
char* mvm_print(mvm* m);
/* Remove one key/value */
void mvm_delete(mvm* m, char* key);
/* Return the corresponding value for a key */
char* mvm_search(mvm* m, char* key);
/* Return *argv[] list of pointers to all values stored with key, n is the number of values */
char** mvm_multisearch(mvm* m, char* key, int* n);
/* Free & set p to NULL */
void mvm_free(mvm** p);
