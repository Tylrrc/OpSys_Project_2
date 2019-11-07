#include <stdlib.h> //random number generator functions
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#define FULL_DECK 52
#define PLAYER_COUNT 3
#define ROUND_COUNT 3
#define THREAD_COUNT 3
#define RANKS 13
#define SUITS 4
#define SHUFFLE_COUNT 3

struct Player
{
    int PlayerNumber;
};



int *Deck = (int *)malloc(sizeof(int)*FULL_DECK);
int CURRENT_DECK_SIZE = FULL_DECK;

int SEED;

Player* players [PLAYER_COUNT];
Player* player_threads [THREAD_COUNT];

pthread_t threads[THREAD_COUNT];

pthread_mutex_t deck_mutex;
pthread_mutex_t status_mutex;

void create_deck(){
   int i = 0;
   while(i < FULL_DECK){
      for(int j = 1; j <= RANKS; ++j){
         Deck[i++] = j;
      }
   }
}

void shuffle_deck(){
   int swap, temp;
   srand(SEED);
   for(int i = 0; i < SHUFFLE_COUNT; ++i)
      for(int j = 0; j < FULL_DECK; ++j){
         swap = rand() % FULL_DECK;
         temp = Deck[j];
         Deck[j] = Deck[swap];
         Deck[swap] = temp;
   }
   //print_deck();  // Just for debugging. Remove at a later time!!!
}

void print_deck(){
   printf("\nDECK CONTAINS: \n\n");
   for(int i = 0; i < CURRENT_DECK_SIZE; ++i){
      printf("%i ", Deck[i]);
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

   SEED = atoi(argv[1]);

   for(int i = 1; i <= PLAYER_COUNT; ++i){
      players[i] = new Player;
      players[i]->PlayerNumber = (i);
      player_threads[i] = players[1];
   }

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

   for (int round = 1; round <= ROUND_COUNT; ++round){
      create_deck();

      printf("\n\n******** ROUND %i ********\n", round);
      shuffle_deck();

   }
   


   return 0;
}