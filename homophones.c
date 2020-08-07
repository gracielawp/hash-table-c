#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

#include "fmvm.h"

#define CSEC (double) (CLOCKS_PER_SEC)
#define MAXLETTERS 40
#define MAXLEN 100

void test(void);
void findHomophones(int argc, char** argv);
void printHomophones(char* word, char* phonemes, char** homophones, int n);
/* Returns true if -n flag is set.*/
bool validateParams(int argc, char** argv);
char* arr_init(int size);
/* Returns a string containing only the last n phonemes. */
char* filterPhonemes(char* tempdata, int phoneme_num, int n);
void file2mvm(mvm* map1, mvm* map2, int n);

int main(int argc, char** argv)
{
   clock_t c1, c2;

   c1 = clock();
   test();
   c2 = clock();
   printf("Testing time took: %fs\n", (double)(c2-c1)/CSEC);

   c1 = clock();
   findHomophones(argc, argv);
   c2 = clock();
   printf("Total time to find homophones: %fs\n", (double)(c2-c1)/CSEC);

   return 0;
}

void test(void)
{
   mvm *map1, *map2;
   char* phonemes;
   char** homophones;
   int i;

   printf("Homophone Tests ... Start\n");
   phonemes = filterPhonemes("1 2 3 4 5", 5, 3);
   i = strcmp(phonemes, "3 4 5");
   free(phonemes);
   assert(i == 0);

   /* Tests that file is read correctly. */
   map1 = mvm_init();
   map2 = mvm_init();
   file2mvm(map1, map2, 4);
   phonemes = mvm_search(map1, "CHRISTMAS");
   i = strcmp(phonemes, "S M AH0 S");
   assert(i == 0);
   homophones = mvm_multisearch(map2, phonemes, &i);
   assert(i == 3);
   i = strcmp(homophones[0], "ISTHMUS");
   assert(i == 0);
   i = strcmp(homophones[1], "CHRISTMAS");
   assert(i == 0);
   i = strcmp(homophones[2], "CHRISTMAS'");
   assert(i == 0);
   free(homophones);
   phonemes = mvm_search(map1, "PASSING");
   i = strcmp(phonemes, "AE1 S IH0 NG");
   assert(i == 0);
   homophones = mvm_multisearch(map2, phonemes, &i);
   assert(i == 10);
   i = strcmp(homophones[0], "GASSING");
   assert(i == 0);
   i = strcmp(homophones[1], "MASENG");
   assert(i == 0);
   i = strcmp(homophones[2], "SURPASSING");
   assert(i == 0);
   i = strcmp(homophones[3], "KASSING");
   assert(i == 0);
   i = strcmp(homophones[4], "HASSING");
   assert(i == 0);
   i = strcmp(homophones[5], "PASSING");
   assert(i == 0);
   i = strcmp(homophones[6], "AMASSING");
   assert(i == 0);
   i = strcmp(homophones[7], "MASSING");
   assert(i == 0);
   i = strcmp(homophones[8], "CLASSING");
   assert(i == 0);
   i = strcmp(homophones[9], "HARASSING");
   assert(i == 0);
   free(homophones);
   mvm_free(&map1);
   mvm_free(&map2);
   printf("Homophone Tests ... End\n");
}

void findHomophones(int argc, char** argv)
{
   mvm *map1, *map2;
   char *phonemes, **homophones;
   clock_t c1, c2;
   int i, starti = 1;
   int n = 3;

   map1 = mvm_init();
   map2 = mvm_init();
   /* If -n flag set, change default n (3) to desired number, and
      change starting index to 3 (where the words start). */
   if(validateParams(argc, argv)){
      starti = 3;
      n = atoi(argv[2]);
   }
   file2mvm(map1, map2, n);
   c1 = clock();
   while(starti < argc) {
      phonemes = mvm_search(map1, argv[starti]);
      if(phonemes == NULL) {
         printf("Word being searched has less phonemes than %d\n", n);
      }
      else {
         homophones = mvm_multisearch(map2, phonemes, &i);
         printHomophones(argv[starti], phonemes, homophones, i);
         free(homophones);
      }
      starti++;
   }
   c2 = clock();
   printf("Search time to find all homophones: %fs\n", (double)(c2-c1)/CSEC);
   mvm_free(&map1);
   mvm_free(&map2);
}

void printHomophones(char* word, char* phonemes, char** homophones, int n)
{
   int index;
   printf("%s (%s): ", word, phonemes);
   for(index = 0; index < n; index++) {
      printf("%s ", homophones[index]);
   }
   printf("\n");
}

bool validateParams(int argc, char** argv)
{
   int n = 3;
   if(argc < 2) {
      fprintf(stderr, "Expected %s -n <number> <WORD>\n", argv[0]);
      exit(EXIT_FAILURE);
   }
   if(strcmp(argv[1], "-n") == 0) {
      n = atoi(argv[2]);
      if(n <= 0) {
         ON_ERROR("Expected a positive integer after -n flag.\n");
      }
      return true;
   }
   return false;
}

char* arr_init(int size)
{
   char* temparr = (char*)calloc(size, sizeof(char));
   if(temparr == NULL) {
      ON_ERROR("Creation of Array Failed.\n");
   }
   return temparr;
}

char* filterPhonemes(char* tempdata, int phoneme_num, int n)
{
   int i = 0, whitespace_cnt = 0;
   char* data = NULL;
   if(phoneme_num >= n) {
      while(whitespace_cnt != (phoneme_num - n)) {
         if(tempdata[i++] == ' ') {
            whitespace_cnt++;
         }
      }
      data = arr_init(strlen(tempdata)-i+1);
      strcpy(data, tempdata+i);
   }
   return data;
}

void file2mvm(mvm* map1, mvm* map2, int n)
{
   int c, phoneme_num, i = 0;
   char *tempdata, *key, *data;
   FILE* fp;

   if((fp = fopen("cmudict.txt", "r")) == NULL) {
      fprintf(stderr, "Cannot open file.\n");
      exit(EXIT_FAILURE);
   }
   do {
      phoneme_num = 1;
      i = 0;
      key = arr_init(MAXLETTERS);
      while(((c = getc(fp)) != EOF) && (c != '#')) {
         key[i++] = c;
      }
      i = 0;
      tempdata = arr_init(MAXLEN);
      while(((c = getc(fp)) != EOF) && (c != '\n')) {
         if(c != '\r') {
            tempdata[i++] = c;
         }
         if(c == ' ') {
            phoneme_num++;
         }
      }
      data = filterPhonemes(tempdata, phoneme_num, n);
      mvm_insert(map1, key, data);
      mvm_insert(map2, data, key);
      free(data);
      free(key);
      free(tempdata);
   }while(c != EOF);
   fclose(fp);
}
