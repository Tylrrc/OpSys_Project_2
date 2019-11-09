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

void create_deck();
void print_deck();
void shuffle_deck();
void push(int *arr, int index, int value, int *size, int *capacity);
void deal_cards();
void end_of_round();
void print_hand();


/*
void print_game_status(const Player* const player[], const int);
void player_draw(Player*& p, Deck*& d);
void player_discard(Player*&, Deck*&);
void push_back_deck(Deck*&, const int);
void* player_makes_move(void*);
void log_event(const string);
*/

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
pthread_cond_t status;

bool WIN = false;

void create_deck(){
   int i = 0;
   while(i < FULL_DECK){
      for(int j = 1; j <= RANKS; ++j){
         Deck[i++] = j;
      }
   }
}

void print_deck(){
   printf("\nDECK CONTAINS: ");
   for(int i = 0; i <= CURRENT_DECK_SIZE; ++i){
      printf("%i ", Deck[i]);
   }
}

void print_hand(){
   for(int i = 0; i < PLAYER_COUNT; ++i){
      printf("\nPLAYER %i\n", player_threads[i]->PlayerNumber);
      printf("CARDS IN HAND: ");
   for(int j = 0; j < player_threads[i]->hand_size; ++j){
      printf(" %i ", player_threads[i]->hand[j]);
      }
      printf("\n");
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

void deal(){
   for(int i = 0; i < PLAYER_COUNT; ++i){ 
         push(player_threads[i]->hand, player_threads[i]->hand_size, Deck[CURRENT_DECK_SIZE--], &player_threads[i]->hand_size, &player_threads[i]->hand_capacity);
         //printf("%i ", p[i]->hand[0]);
   }
}

void end_of_round(){
   for(int i = 0; i < PLAYER_COUNT; ++i){
      free(player_threads[i]->hand);
      player_threads[i]->hand_capacity = 0;
      player_threads[i]->hand_size = 0;
   }

   //also reset the deck size!!!
}

void draw(Player* p){
   push(p->hand, p->hand_size, Deck[CURRENT_DECK_SIZE--], &p->hand_size, &p->hand_capacity);
}

/*
void discard(Player* &p){
   void;
}
*/

void* turn(void* player){
   Player* p = (Player*)player;
   pthread_mutex_lock(&deck_mutex);
   if (p->hand_size >= 2){
//      discard(p);
      print_hand();
   }
   draw(p);
   print_hand();
   pthread_mutex_lock(&status_mutex);
   if(p->hand[0] == p->hand[1]){
      pthread_cond_signal(&status);
      WIN = true;
      printf("\nPlayer %i wins!\n", p->PlayerNumber);   
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
      deal();

      print_hand();
      print_deck();

      while(!WIN){
         for(int i = 0; i < THREAD_COUNT; ++i){
            pthread_create(&threads[i], NULL, turn, (void *)player_threads[i]);
         }
         
         for(int i = 0; i < THREAD_COUNT; ++i){
            if (pthread_join(threads[i], NULL)){
               return -1;
            }
           
            
         //printf("IS THIS THE LOOP WE'RE STUCK INSIDE?"); // Answer: YES!!!!!
         }
      

      end_of_round();

   }

   return 0;
}
}

/*
TODO: Implement remaining functions (See prototype list)
TODO: Add comments
TODO: Implement gameplay portion
TODO: The 'WIN' variable isn't getting triggered. 
      There are likely several issues to be addressed in 'turn' function...
      The main while loop becomes infinite...
      Compare closely with sample code to get a better idea of what's going wrong.

*/