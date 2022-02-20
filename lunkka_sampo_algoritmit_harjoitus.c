#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define WORD_MAX 100
#define FILE_NAME_MAX 256

typedef struct node {
  char word[WORD_MAX];
  unsigned long freq;
} node;

unsigned long unique_words = 0;

/*
FIKSIT:
  pointer taulukko malloc(x)
  fseek (x)
  isompi hashtaulu(x)
  malloc loopin jalkeen(x)
  freet malloceille(x)
  uniikit sanat laskuri(x)
  qsort(x)
*/

//Keskimääräinen sanan pituus on 5.1
//Välimerkkien kanssa heitin arvio, että se olisi lähelle 7
//Vastasi aika hyvin tiedoston todellista sanamäärää
//Sitten tein 2.5 kertasen taulukosta, jotta väljyyttä tulee mukavasti
unsigned long hash_table_set_size(FILE * fp) {
  fseek(fp, 0, SEEK_END);
  unsigned long size = ftell(fp);
  rewind(fp);
  return size*0.17*2.5;
}

//djb2 hash funktio
unsigned long hash(unsigned char * word) {
    unsigned long hash = 5381;
    int c;

    while (c = *word++)
        hash = ((hash << 5) + hash) + c;
    return hash;
}

//Hash-taulun pointerit laitetaa NULL
//Tarvitaa, koska insert tarkistaa onko NULL = paikka on tyhjä
void init(node ** hash_table, unsigned long table_size) {
  for (int i = 0; i < table_size; i++) {
    hash_table[i] = NULL;
  }
}


//Hash tauluun sijoitusta
//Jos paikka on NULL, sijoitetaan
//Jos paikka on viety, tarkistetaan onko sama sana
//Jos ei ole sama sana, luodataan kunnes löytyy sama sana tai tyhjä paikka
node * insert(char * string, node ** hash_table, unsigned long table_size) {
  unsigned long n = hash(string) % table_size;
  unsigned long i = 0;
  while(NULL != hash_table[n+i]) {
    if (0 == strcmp(hash_table[n+i]->word, string)) {
      hash_table[n+i]->freq++;
      return hash_table[n+i];
    }
    else if(n+i > table_size) {
      i = -n;

    }
    else if(i == -1) {
      exit(1);
      return NULL;
    }
    else {
      i++;
    }
  }

  //siirretty malloci tänne missä sitä vasta tarvitsee
  //lisäsin neuvostasi uniikkien sanojen laskurin jota käytän sorttauksessa taulukon koon määritykseen
  node * entry = (struct node*)malloc(sizeof(struct node));
  if (entry == NULL){ free(entry); exit(1); }

  strcpy(entry->word, string);;
  entry->freq = 1;
  hash_table[n+i] = entry;
  unique_words++;
  return entry;
}

//Käydään koko taulu läpi ja tulostetaan sanat
//Tätä apufunktiota, ei kutsuta lopullisessa toteutuksessa
void hash_table_print(node ** hash_table, unsigned long table_size) {
  for(int i = 0; i < table_size; i++) {
    if(hash_table[i] == NULL) {
      printf("- %i ///////////////////////////////////////// \n", i);
    }
    else {
      printf("# %i %s ... %lu\n", i, hash_table[i]->word, hash_table[i]->freq);
    }
  }
}

//Apufunktio qsortiin
int compare(const void * a, const void * b) {

  const node * entry_a = *(node **)a;
  const node * entry_b = *(node **)b;
  
  return entry_b->freq - entry_a->freq;
}

//qsortilla sortataan 100 yleisintä sanaa
//käytän uniikkien sanojen laskuria määrittämään aputaulukon koon
void most_frequent(node ** hash_table, unsigned long hash_table_size, int how_many) {
  node ** words = (node**)malloc(unique_words * sizeof(node*));
  int j = 0;
  for (int i = 0; i < hash_table_size; i++) {
    if (hash_table[i]!= NULL) {
      words[j] = hash_table[i];
      j++;
    }
  }

  qsort(words, unique_words, sizeof(node*), compare);

  printf("\nPRINTING %d MOST COMMON WORDS:\n\n#: word - frequency\n\n", how_many);
  for (int i = 0; i < how_many; i++) {
    printf("%d: %s - %lu\n", i+1, words[i]->word, words[i]->freq);
  }

  free(words);
  
}


int main(int argc, char const *argv[]) {

  //syöte
  printf("text file name:\n");
  char file_name[FILE_NAME_MAX];
  scanf("%s", file_name);
  while(getc(stdin) != '\n');

  printf("Opening file:  <%s>\n", file_name);

  FILE * fp = fopen(file_name, "r");
  if (fp == NULL){ printf("file not found! Exiting...\n"); exit(1);}

  clock_t t0 = clock();

  //taulun koko lasketaan
  unsigned long table_size = hash_table_set_size(fp);
  printf("Hash table size: %lu\n", table_size);

  //Luodaan hash taulu struct avain-arvo -pareille ja NULLataan paikat
  //Neuvostasi laitettu taulukon koko sizeof(node*)
  node ** hash_table = (node**)malloc(table_size * sizeof(node*));
  init(hash_table, table_size);
  printf("HASHTABLE initialized\n");

  int i = 0;
  char c = 0;

  double t_elapsed_tablesize = clock() - t0;
  t_elapsed_tablesize /= CLOCKS_PER_SEC;

  //ohjelman suorituskyvyn arviointiin
  //t1 = tauluun sanojen lisäämiseen käytettävä aika
  clock_t t1 = clock();


  //luetaan merkkejä a-z ja ' merkkejä yksitellen kunnes kohdataan välimerkki
  while(EOF != (c = fgetc(fp))) {
    unsigned char word[WORD_MAX];
    if (isalpha(c) || c == 39) {
      word[i] = tolower(c);
      i++;
    } else if (i!=0) {
      word[i] = '\0';
      node * n = insert(word, hash_table, table_size);
      if (n == NULL){printf("NULL insert"); exit(0);}
      i = 0;
    }
  }

  //t1 loppuu
  double t_elapsed_insert = clock() - t1;
  t_elapsed_insert /= CLOCKS_PER_SEC;

  //ohjelman suorituskyvyn arviointiin
  //t2 = sanojen järjestelyyn käytettävä aika
  clock_t t2 = clock();

  most_frequent(hash_table, table_size, 100);

  for (int i = 0; i < table_size; i++) {
    free(hash_table[i]);
  }
  
  free(hash_table);

  //t2 loppuu
  double t_elapsed_sort = clock() - t2;
  t_elapsed_sort /= CLOCKS_PER_SEC;


  unsigned long wordcount = table_size/2.5;
  //tulostetaan statistiikkaa
  printf("\nSTATISTICS:\n");
  printf("text file: %s - Word count estimate:(%lu)\n", file_name, wordcount);
  printf("\nTABLESIZE:\nwps:(%f) - time elapsed(%f)s\n", wordcount/t_elapsed_tablesize, t_elapsed_tablesize);
  printf("\nINSERT:\nwps:(%f) - time elapsed(%f)s\n", wordcount/t_elapsed_insert, t_elapsed_insert);
  printf("\nSORT:\nwps:(%f) - time elapsed(%f)s\n", wordcount/t_elapsed_sort, t_elapsed_sort);
  printf("\nTOTAL:\n");
  printf("wps:(%f) - time elapsed(%f)s\n", wordcount/(t_elapsed_sort + t_elapsed_insert + t_elapsed_tablesize), t_elapsed_sort + t_elapsed_insert + t_elapsed_tablesize);
  printf("\nFarewell\n\n");
  return 0;
}
