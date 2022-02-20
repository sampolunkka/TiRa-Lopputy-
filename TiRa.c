#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define WORD_MAX 100
#define FILE_NAME_MAX 256
#define WORD_LEN 7.0

typedef struct node
{
  char word[WORD_MAX];
  unsigned long freq;
} node;

unsigned long unique_words = 0;

/*
 * Function: hash_table_set_size
 * -----------------------------
 *  Approximates the appropriate size for the hash table for unique words
 *
 *  * fp: pointer to the text file containing the words
 *
 *  returns: the size for the hash table
 */

unsigned long hash_table_set_size(FILE *fp)
{
  fseek(fp, 0, SEEK_END);
  unsigned long size = ftell(fp);
  rewind(fp);
  float cpw = 1.0 / WORD_LEN;
  return (int)(cpw * 2.5 * size);
}

/*
 * Function: hash
 * --------------
 *  djb2 hash function
 *
 *  * word: unhashed string
 *
 *  returns: hashed string
 */
unsigned long hash(unsigned char *word)
{
  unsigned long hash = 5381;
  int c;

  while (c = *word++)
    hash = ((hash << 5) + hash) + c;
  return hash;
}

/*
 * Function: init
 * --------------
 *  Initializing the hash table by setting each element to NULL
 *
 *  ** hash_table:  pointer to the hash table
 *  table_size:     size of the hash table
 */
void init(node **hash_table, unsigned long table_size)
{
  for (int i = 0; i < table_size; i++)
  {
    hash_table[i] = NULL;
  }
}

/*
 * Function: insert
 * ----------------
 *  Inserts words into the hash table
 *
 *  Words are inserted into NULL elements. On collision uses increments word
 *  if the words are same, else uses linear probing to find new NULL element
 *
 *  * string:       word string
 *  ** hash_table:  hash table
 *  table_size:     size of the hash_table
 *
 *  returns: pointer to the word element in the hash table
 */
node *insert(char *string, node **hash_table, unsigned long table_size)
{
  unsigned long n = hash(string) % table_size;
  unsigned long i = 0;
  while (NULL != hash_table[n + i])
  {
    if (0 == strcmp(hash_table[n + i]->word, string))
    {
      hash_table[n + i]->freq++;
      return hash_table[n + i];
    }
    else if (n + i > table_size)
    {
      i = -n;
    }
    else if (i == -1)
    {
      exit(1);
      return NULL;
    }
    else
    {
      i++;
    }
  }

  node *entry = (struct node *)malloc(sizeof(struct node));
  ;
  if (entry == NULL)
  {
    free(entry);
    exit(1);
  }

  strcpy(entry->word, string);
  entry->freq = 1;
  hash_table[n + i] = entry;
  unique_words++;
  return entry;
}

/*
 * Function: hash_table_print
 * --------------------------
 *  Prints all the words and empty elements in the hash table
 *
 *  ** hash_table:  hash table
 *  table_size:     size of the hash_table
 */
void hash_table_print(node **hash_table, unsigned long table_size)
{
  for (int i = 0; i < table_size; i++)
  {
    if (hash_table[i] == NULL)
    {
      printf("- %i ///////////////////////////////////////// \n", i);
    }
    else
    {
      printf("# %i %s ... %lu\n", i, hash_table[i]->word, hash_table[i]->freq);
    }
  }
}

/*
 * Function: compare
 * --------------------------
 *  Compare function for qsort
 *
 *  * a: void const pointer
 *  * b: void const pointer
 */
int compare(const void *a, const void *b)
{

  const node *entry_a = *(node **)a;
  const node *entry_b = *(node **)b;

  return entry_b->freq - entry_a->freq;
}

/*
 * Function: most_frequent
 * -----------------------
 *  Calculates the most frequent words using qsort
 *
 *  ** hash_table:  hash table
 *  table_size:     size of the hash_table
 *  how_many:       the n of most frequent words wanted
 */
void most_frequent(node **hash_table, unsigned long table_size, int how_many)
{
  node **words = (node **)malloc(unique_words * sizeof(node *));
  int j = 0;
  for (int i = 0; i < table_size; i++)
  {
    if (hash_table[i] != NULL)
    {
      words[j] = hash_table[i];
      j++;
    }
  }

  qsort(words, unique_words, sizeof(node *), compare);

  printf("\nPRINTING %d MOST COMMON WORDS:\n\n#: word - frequency\n\n", how_many);
  for (int i = 0; i < how_many; i++)
  {
    printf("%d: %s - %lu\n", i + 1, words[i]->word, words[i]->freq);
  }

  free(words);
}

int main(int argc, char const *argv[])
{

  printf("Input a path to text file:\n");
  char file_name[FILE_NAME_MAX];
  scanf("%s", file_name);
  while (getc(stdin) != '\n')
    ;

  printf("Opening file:  <%s>\n", file_name);

  FILE *fp = fopen(file_name, "r");
  if (fp == NULL)
  {
    printf("file not found! Exiting...\n");
    exit(1);
  }

  /* t0: time elapsed in all operations */
  clock_t t0 = clock();

  unsigned long table_size = hash_table_set_size(fp);
  printf("Hash table size: %lu\n", table_size);

  node **hash_table = (node **)malloc(table_size * sizeof(node *));
  init(hash_table, table_size);
  printf("HASHTABLE initialized\n");

  int i = 0;
  char c = 0;

  double t_elapsed_tablesize = clock() - t0;
  t_elapsed_tablesize /= CLOCKS_PER_SEC;

  /* t1: time elapsed in insertion phase */
  clock_t t1 = clock();

  /* Read characters until punctuation or EOF */
  while (EOF != (c = fgetc(fp)))
  {
    unsigned char word[WORD_MAX];
    if (isalpha(c) || c == 39)
    {
      word[i] = tolower(c);
      i++;
    }
    else if (i != 0)
    {
      word[i] = '\0';
      node *n = insert(word, hash_table, table_size);
      if (n == NULL)
      {
        printf("NULL insert");
        exit(0);
      }
      i = 0;
    }
  }

  double t_elapsed_insert = clock() - t1;
  t_elapsed_insert /= CLOCKS_PER_SEC;

  /* t2: time elapsed in sorting */
  clock_t t2 = clock();

  most_frequent(hash_table, table_size, 100);

  for (int i = 0; i < table_size; i++)
  {
    free(hash_table[i]);
  }

  free(hash_table);

  double t_elapsed_sort = clock() - t2;
  t_elapsed_sort /= CLOCKS_PER_SEC;

  unsigned long wordcount = table_size / 2.5;

  printf("\nSTATISTICS:\n");
  printf("\nwps = words per second:\n");
  printf("text file: %s - Word count estimate:(%lu)\n", file_name, wordcount);
  printf("\nTABLESIZE:\nwps:(%f) - time elapsed(%f)s\n", wordcount / t_elapsed_tablesize, t_elapsed_tablesize);
  printf("\nINSERT:\nwps:(%f) - time elapsed(%f)s\n", wordcount / t_elapsed_insert, t_elapsed_insert);
  printf("\nSORT:\nwps:(%f) - time elapsed(%f)s\n", wordcount / t_elapsed_sort, t_elapsed_sort);
  printf("\nTOTAL:\n");
  printf("wps:(%f) - time elapsed(%f)s\n", wordcount / (t_elapsed_sort + t_elapsed_insert + t_elapsed_tablesize), t_elapsed_sort + t_elapsed_insert + t_elapsed_tablesize);
  printf("\nHave a good day :)\n\n");
  return 0;
}
