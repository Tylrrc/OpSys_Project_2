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
#define INITIAL_HAND_CAP 1

struct Player
{
    int PlayerNumber;
    int* hand = (int *)malloc(sizeof(int)*INITIAL_HAND_CAP);
    int hand_size = 0;
    int hand_capacity = 0;
};

int *Deck = (int *)malloc(sizeof(int)*FULL_DECK);
int CURRENT_DECK_SIZE = FULL_DECK - 1;

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

void print_deck(){
   printf("\nDECK CONTAINS: \n\n");
   for(int i = 0; i <= CURRENT_DECK_SIZE; ++i){
      printf("%i ", Deck[i]);
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

void push(int *arr, int index, int value, int *size, int *capacity){
     if(size > capacity){
          realloc(arr, sizeof(arr) * 2);
          *capacity = sizeof(arr) * 2;
     }
     
     arr[index] = value;
     *size = *size + 1;
}

void deal_cards(Player* p[]){
   //printf("( %i )", Deck[CURRENT_DECK_SIZE]);
   for(int i = 0; i < PLAYER_COUNT; ++i){ 
         push(p[i]->hand, p[i]->hand_size, Deck[CURRENT_DECK_SIZE--], &p[i]->hand_size, &p[i]->hand_capacity);
         //printf("%i ", p[i]->hand[0]);
   }
}

void end_of_round(Player *p[]){
   for(int i = 0; i < PLAYER_COUNT; ++i){
      free(p[i]->hand);
      p[i]->hand_capacity = 0;
      p[i]->hand_size = 0;
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

   for(int i = 0; i < PLAYER_COUNT; ++i){
      players[i] = new Player;
      players[i]->PlayerNumber = i + 1;
      player_threads[i] = players[i];
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
      CURRENT_DECK_SIZE = FULL_DECK - 1;
      create_deck();
      printf("\n\n******** ROUND %i ********\n", round);
      shuffle_deck();
      deal_cards(player_threads);
      deal_cards(player_threads);

      printf("HAND SIZE == %i",player_threads[0]->hand_size);
      print_deck();

      end_of_round(player_threads);

   }

   return 0;
}