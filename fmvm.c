#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "fmvm.h"

#define BRACKETSSZ 5
#define DEFAULT_HT_SIZE 5119
#define DEFAULT_EXPAND_FACTOR 5
#define DEFAULT_LOAD_FACTOR 0.75

/* Set up hash table with no elements. */
htable* htable_init(int size)
{
   htable* newhtable = (htable*)calloc(1, sizeof(htable));
   if(newhtable == NULL) {
      ON_ERROR("Creation of Hash Table Failed.\n");
   }
   newhtable->arr = (mvmcell**)calloc(size, sizeof(mvmcell*));
   if(newhtable->arr == NULL) {
      ON_ERROR("Creation of Hash Table Array Pointers Failed.\n");
   }
   newhtable->numdata = (int*)calloc(size, sizeof(int));
   if(newhtable->numdata == NULL) {
      ON_ERROR("Creation of Array Failed.\n");
   }
   newhtable->capacity = size;
   return newhtable;
}

/* Set up multi-value map with no elements */
mvm* mvm_init(void)
{
   mvm* m;
   m = (mvm*)calloc(1, sizeof(mvm));
   if(m == NULL) {
      ON_ERROR("Creation of Multi-Value Map Failed.\n");
   }
   m->hash = htable_init(DEFAULT_HT_SIZE);

   return m;
}

/* Number of key/value pairs stored */
int mvm_size(mvm* m)
{
   if(m == NULL) {
      return 0;
   }
   return m->hash->numkeys;
}

/* Free & set p to NULL */
void mvmcell_free(mvmcell *p)
{
   free(p->key);
   free(p->data);
   free(p);
   p = NULL;
}

/* Hash functions taken from cse.yorku.ca/~oz/hash.html
   Modified Bernstein Hashing
   5381 & 33 are magic numbers required by the algorithm.
*/
unsigned long hash(char *str)
{
   unsigned long hash = 5381;
   int c;

   while((c = (*str++)))    {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
   }
   return hash;
}

/* SDBM Database Library Algorithm
   65599 is a magic constant required by algorithm.
*/
unsigned long hash2(char *str)
{
   unsigned long hash = 0;
   int c;

   while((c = (*str++)))    {
      /* hash(i) = hash(i-1) * 65599 + str[i] */
      hash = c + (hash << 6) + (hash << 16) - hash;
   }
   return hash;
}

bool isPrime(int n)
{
   int i;
   if(n <= 1) {
      return false;
   }
   for(i = 2; i < n; i++) {
      if(n % i == 0) {
         return false;
      }
   }
   return true;
}

int nextPrime(int n)
{
   int i = n*DEFAULT_EXPAND_FACTOR;

   while(!isPrime(i)) {
      i++;
   }
   return i;
}

mvmcell* allocate_node(char* key, char* data)
{
  mvmcell* newnode = (mvmcell*)malloc(sizeof(mvmcell));
  if(newnode == NULL) {
    ON_ERROR("Creation of Multi-Value Map Cell Failed");
  }
  newnode->key = (char*)malloc(sizeof(char)*strlen(key)+1);
  if(newnode->key == NULL) {
    ON_ERROR("Creation of Multi-Value Map Key Failed");
  }
  newnode->data = (char*)malloc(sizeof(char)*strlen(data)+1);
  if(newnode->data == NULL) {
    ON_ERROR("Creation of Multi-Value Map Data Failed");
  }
  strcpy(newnode->key, key);
  strcpy(newnode->data, data);
  newnode->next = NULL;
  return newnode;
}

void linkedlist_insert(htable* h, mvmcell* newnode, int index)
{
   if(newnode->next != NULL) {
      h->arr[index] = newnode;
   }
   else {
      newnode->next = h->arr[index];
      h->arr[index] = newnode;
   }
}

/* Returns index of key after linear probing. */
int linear_probing(htable* h, mvmcell* newnode, int offset)
{
   int i = 0, size = h->capacity;
   int index = (offset+i)%size;;

   while(h->arr[index] != NULL) {
      if(strcmp(h->arr[index]->key, newnode->key) == 0) {
         linkedlist_insert(h, newnode, index);
         return index;
      }
      i++;
      index = (offset+i)%size;
   }
   linkedlist_insert(h, newnode, index);
   ++h->numkeys;
   return index;
}

/* Returns index of key inserted. */
int htable_insert(htable* h, mvmcell* newnode)
{
   int index, offset;
   int size = h->capacity;

   index = hash(newnode->key)%(size);
   offset = hash2(newnode->key)%(size) + index;

   if(h->arr[index] == NULL) {
      linkedlist_insert(h, newnode, index);
      ++h->numkeys;
   }
   else {
      if(strcmp(h->arr[index]->key, newnode->key) == 0) {
         linkedlist_insert(h, newnode, index);
      }
      else {
         index = linear_probing(h, newnode, offset);
      }
   }
   return index;
}

void resize_if_full(mvm* m)
{
   int i, index;
   htable* new;
   if((m->hash->numkeys/m->hash->capacity) >= DEFAULT_LOAD_FACTOR) {
      new = htable_init(nextPrime(m->hash->capacity));
      for(i = 0; i < m->hash->capacity; i++) {
         if(m->hash->arr[i] != NULL) {
            index = htable_insert(new, m->hash->arr[i]);
            new->numdata[index] = m->hash->numdata[i];
         }
      }
      free(m->hash->numdata);
      free(m->hash->arr);
      free(m->hash);
      m->hash = new;
      printf("Table Rehashed\n");
   }
}

/* Insert one key/value pair */
void mvm_insert(mvm* m, char* key, char* data)
{
   mvmcell* newnode;
   int index;

   if(m == NULL || key == NULL || data == NULL ||
   strlen(key) == 0 || strlen(data) == 0) {
      return;
   }

   newnode = allocate_node(key, data);
   index = htable_insert(m->hash, newnode);
   ++m->hash->numdata[index];

   resize_if_full(m);
}

/* Find the length of all the key-value pairs to get the size required
 * for the list to be printed. */
int find_length(htable* h)
{
   mvmcell* head;
   int i;
   unsigned length = 0;
   for(i = 0; i < h->capacity; i++) {
      head = h->arr[i];
      while(head != NULL) {
         /* BRACKETSSZ includes []() and ' ' size for printing. */
         length += strlen(head->key) + strlen(head->data) + BRACKETSSZ;
         head = head->next;
      }
   }
   return length;
}

/* Store list as a string "[key](value) [key](value) " etc.  */
char* mvm_print(mvm* m)
{
   char *list, *temp;
   int i, templen;
   mvmcell* p;

   if(m == NULL) {
      return NULL;
   }
   list = (char*)calloc(find_length(m->hash)+1, sizeof(char));
   if(list == NULL) {
      ON_ERROR("Creation of List Failed.\n");
   }
   for(i = 0; i < m->hash->capacity; i++) {
      p = m->hash->arr[i];
      while(p != NULL) {
         templen = strlen(p->key) + strlen(p->data) + BRACKETSSZ;
         temp = (char*)calloc(templen+1, sizeof(char));
         sprintf(temp, "[%s](%s) ", p->key, p->data);
         strcat(list, temp);
         free(temp);
         p = p->next;
      }
   }
   return list;
}

/* Remove one key/value */
void mvm_delete(mvm* m, char* key)
{
   mvmcell* p;
   int index, offset, i = 0;
   int size;

   if(m == NULL || key == NULL || strlen(key) == 0) {
      return;
   }
   size = m->hash->capacity;
   index = hash(key)%size;
   offset = hash2(key)%size + index;

   if(strcmp(m->hash->arr[index]->key, key) == 0) {
      p = m->hash->arr[index];
      m->hash->arr[index] = m->hash->arr[index]->next;
      mvmcell_free(p);
   }
   else {
      while(strcmp(m->hash->arr[(offset+i)%size]->key, key) != 0) {
         i++;
      }
      index = (offset+i)%size;
      p = m->hash->arr[index];
      m->hash->arr[index] = m->hash->arr[index]->next;
      mvmcell_free(p);
   }
   --m->hash->numdata[index];
   if(m->hash->arr[index] == NULL) {
      --m->hash->numkeys;
   }
}

/* Return the corresponding value for a key */
char* mvm_search(mvm* m, char* key)
{
   int index, offset, i = 0;
   int size = m->hash->capacity;
   index = hash(key)%size;
   offset = hash2(key)%size + index;

   if(m->hash->arr[index] == NULL) {
      return NULL;
   }

   if(strcmp(m->hash->arr[index]->key, key) == 0) {
      return m->hash->arr[index]->data;
   }
   else {
      while(m->hash->arr[(offset+i)%size] != NULL) {
         if(strcmp(m->hash->arr[(offset+i)%size]->key, key) != 0) {
            i++;
         }
      }
      if(m->hash->arr[(offset+i)%size] == NULL) {
         return NULL;
      }
      return m->hash->arr[(offset+i)%size]->data;
   }
   return NULL;
}

char** add_value_pointers(htable* h, int index)
{
   int i = 0;
   char** c;
   mvmcell* head = h->arr[index];
   c = (char**)calloc(h->numdata[index], sizeof(char*));
   while(head != NULL) {
      c[i] = head->data;
      i++;
      head = head->next;
   }
   return c;
}

/* Return *argv[] list of pointers to all values stored with key, n is the number of values */
char** mvm_multisearch(mvm* m, char* key, int* n)
{
   char** c;
   int index, offset, i = 0;
   int size = m->hash->capacity;

   if(m == NULL || key == NULL || n == NULL) {
      return NULL;
   }

   index = hash(key)%size;
   offset = hash2(key)%size + index;

   if(strcmp(m->hash->arr[index]->key, key) != 0) {
      while(strcmp(m->hash->arr[(offset+i)%size]->key, key) != 0) {
         i++;
      }
      index = (offset+i)%size;
   }

   c = add_value_pointers(m->hash, index);
   *n = m->hash->numdata[index];

   return c;
}

/* Free the linked list & set p to NULL */
void free_linked_list(mvmcell* p)
{
   mvmcell* current;
   while((current = p) != NULL) {
      p = p->next;
      free(current->key);
      free(current->data);
      free(current);
   }
   p = NULL;
}

/* Free all elements of htable & set h to NULL. */
void htable_free(htable* h)
{
   int i;
   for(i = 0; i < h->capacity; i++) {
      if(h->arr[i] != NULL) {
         free_linked_list(h->arr[i]);
      }
   }
   free(h->arr);
   free(h->numdata);
   h = NULL;
}

/* Free & set p to NULL */
void mvm_free(mvm** p)
{
   mvm* temp;
   if(p == NULL || *p == NULL) {
      return;
   }
   temp = *p;
   htable_free(temp->hash);
   free(temp->hash);
   free(temp);
   *p = NULL;
}
