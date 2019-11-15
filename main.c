/*
TODO: Address seg fault after 2nd round
TODO: Reformat to include output to file
TODO: vector_bottom() seems off. conform that logic is correct
TODO: Ensure there are no 'off by 1' errors in program
*/

#include <stdlib.h> //random number generator functions
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#include "vector.h"

#define FULL_DECK 52
#define PLAYER_COUNT 3
#define ROUND_COUNT 3
#define THREAD_COUNT 3
#define RANKS 13
#define SUITS 4
#define SHUFFLE_COUNT 3
#define LOGGING 1

int SEED;
bool WIN = false;

struct Player{
   int PlayerNumber;
   vector Hand;
};

vector players;

vector Deck;

pthread_t threads[THREAD_COUNT];

pthread_mutex_t deck_mutex;
pthread_mutex_t status_mutex;
pthread_cond_t status;

FILE *fileptr;

void draw(struct Player*);
void discard(struct Player*);
void* turn(void*);
void print_hand(struct Player*);
void print_deck();
void log_deck();
void shuffle_deck();
void create_deck();
void deal(struct Player*);
void end_round();

void draw(struct Player* p){
   //int t = vector_total(&Deck) - 1;
   long drawn = (long)vector_get(&Deck, vector_total(&Deck) - 1);
   printf("\nPLAYER %i DRAWS: ", p->PlayerNumber);
   printf("%i\n", drawn);
   if (LOGGING) {
       fprintf(fileptr, "PLAYER %i: draws ", p->PlayerNumber);
       fprintf(fileptr, "%i", drawn);
   }

   vector_add(&p->Hand, (void *)drawn);
   vector_delete(&Deck, vector_total(&Deck)-1);

   print_deck();
}

void discard(struct Player* p){
   printf("SIZE OF PLAYER %i HAND: ", p->PlayerNumber);
   printf("< %i >", vector_total(&p->Hand));

   int discardee = rand() % vector_total(&p->Hand);

   printf("\nPLAYER %i DISCARDS: ", p->PlayerNumber);
   printf("%ld\n", (long)vector_get(&p->Hand,discardee));

   if (LOGGING) {
       fprintf(fileptr, "PLAYER %i: discards ", p->PlayerNumber);
       fprintf(fileptr, "%ld\n", (long)vector_get(&p->Hand,discardee));
   }

   vector_add(&Deck, (int)vector_get(&p->Hand,rand()%vector_total(&p->Hand)));
   
   //Insert discarded card at bottom of deck
   //vector_bottom(&Deck, vector_get(&p->Hand,discardee));

   shuffle_deck();


   vector_delete(&p->Hand, discardee);

   print_deck();
}

void* turn(void* pl){

   struct Player *p = (struct Player *) pl;
   printf("\n++++++++++++PLAYER %i's TURN++++++++++++\n", p->PlayerNumber);
   
   pthread_mutex_lock(&deck_mutex);

   if(vector_total(&p->Hand) >= 2){
      discard(p);
      print_hand(p);
   }

   draw(p);
   print_hand(p);
   pthread_mutex_lock(&status_mutex);

   if((int)vector_get(&p->Hand, 0) == (int)vector_get(&p->Hand, 1)){
      pthread_cond_signal(&status);
      WIN = true;
      printf("\nPLAYER %i WINS!!!\n", p->PlayerNumber);
      if (LOGGING) {
          fprintf(fileptr, "\nPLAYER %i: wins and exits\n", p->PlayerNumber);
      }
   }
   pthread_mutex_unlock(&status_mutex);
   pthread_mutex_unlock(&deck_mutex);

   pthread_exit(NULL);
}

void print_hand(struct Player* p){
   printf("\nPLAYER %i HAND CONTAINS:\n", p->PlayerNumber);
   for(int i = 0; i < vector_total(&p->Hand); ++i){
      printf(" %ld ", (long)vector_get(&p->Hand, i));
   }
   printf("\n");

   if (LOGGING) {
       fprintf(fileptr, "\nPLAYER %i: hand is", p->PlayerNumber);
       for(int i = 0; i < vector_total(&p->Hand); ++i){
           fprintf(fileptr, " %ld", (long)vector_get(&p->Hand, i));
       }
       fprintf(fileptr, "\n");
   }
}

void print_deck(){
   printf("\nDECK:\n");
   for(int i = 0; i < vector_total(&Deck); ++i){
      printf(" %ld ", (long)vector_get(&Deck, i));
   }
   printf("\n");
}

void log_deck() {
    if (LOGGING) {
        fprintf(fileptr, "DECK:");
        for(int i = 0; i < vector_total(&Deck); ++i){
            fprintf(fileptr, " %ld", (long)vector_get(&Deck, i));
        }
        // fprintf(fileptr, "\n");
    }
}

void shuffle_deck(){
   int swap, temp;
   //srand(SEED);
   for(int i = 0; i < SHUFFLE_COUNT; ++i)
      for(int j = 0; j < vector_total(&Deck); ++j){
         swap = rand() % vector_total(&Deck);
         temp = (long)vector_get(&Deck, j);
         vector_set(&Deck, j, vector_get(&Deck, swap));
         vector_set(&Deck, swap, (void *)temp);
   }

   if (LOGGING) {fprintf(fileptr, "DEALER: shuffles\n");}
}

void create_deck(){
   vector_init(&Deck);
   int i = 0;
   while(i < FULL_DECK){
      for(int j = 1; j <= RANKS; ++j){
         vector_add(&Deck, j);
         ++i;
      }
   }
   shuffle_deck();
}

void deal(struct Player* p){
   vector_init(&p->Hand);
   vector_add(&p->Hand, vector_get(&Deck, vector_total(&Deck)-1));
   vector_delete(&Deck, vector_total(&Deck)-1);
   if (LOGGING) {fprintf(fileptr, "DEALER: deals to PLAYER %i", p->PlayerNumber);}
}

void end_round(){
   
   vector_free(&Deck);

   struct Player* p1 = (struct Player *)vector_get(&players, 0);
   vector_free(&p1->Hand);

   struct Player* p2 = (struct Player *)vector_get(&players, 1);
   vector_free(&p2->Hand);

   struct Player* p3 = (struct Player *)vector_get(&players, 2);
   vector_free(&p3->Hand);

   /*&
   for(int i = 0; i < PLAYER_COUNT; ++i){
      vector_free(vector_get(&players, i));
   }
   */

   printf("ROUND END:");

   if (LOGGING) {
       fprintf(fileptr, "ROUND END STATE:\n");
       print_hand(p1);
       print_hand(p2);
       print_hand(p3);
       print_deck();
       log_deck();
   }
}

int main(int argc, char *argv[]){
   if (!argv[1]){
      printf("\nSecond argument is missing.\n\n");
      printf("Try: %s <seed value (integer)>\n\n", argv[0]);
      printf("Example:\n\n"); 
      printf("%s 3\n\n", argv[0]);
      return 1;
   }

   if (LOGGING) {
       fileptr = fopen("log.txt", "w");
   }

   srand(atoi(argv[1]));

   int a = pthread_mutex_init(&deck_mutex, NULL);
   if(a != 0){
      printf("ERROR: \n\n");
      printf("Return code from line \"int a = pthread_mutex_init(&deck_mutex, NULL);\" is %d\n\n", a);
      return -1;
   }

   int b = pthread_mutex_init(&status_mutex, NULL);
   if(b != 0){
      printf("ERROR: \n\n");
      printf("Return code from line \"int b = pthread_mutex_init(&status_mutex, NULL);\" is %d\n\n", a);
      return -1;
   }

   struct Player p1 = {.PlayerNumber = 1};
   struct Player p2 = {.PlayerNumber = 2};
   struct Player p3 = {.PlayerNumber = 3};

   vector_init(&players);
   
   vector_add(&players, &p1);
   vector_add(&players, &p2);
   vector_add(&players, &p3);

   for (int round = 1; round <= ROUND_COUNT; ++round){
      create_deck();

      printf("\n---------- ROUND %i ----------\n", round);

      for(int i = 0; i < PLAYER_COUNT; ++i){
         deal((struct Player *)vector_get(&players, i));
         print_hand((struct Player *)vector_get(&players, i));
      }
      print_deck();

      int esc;

      while(!WIN){
         for(int i = 0; i < THREAD_COUNT; ++i){
            esc = pthread_create(&threads[i], NULL, turn, (vector *)vector_get(&players, i));
            if (esc){ printf("FAILED TO CREATE THREAD");return -1; }
         }

         for(int i = 0; i < THREAD_COUNT; ++i){
            if(pthread_join(threads[i],NULL)){
               printf("FAILED TO JOIN THREAD");
               return -1;
            }
         }
      }

      end_round();
      
      WIN = false;
   }

   fclose(fileptr);
   pthread_mutex_destroy(&deck_mutex);
   pthread_mutex_destroy(&status_mutex);
   pthread_exit(NULL);
   
}
