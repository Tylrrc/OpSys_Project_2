#include <stdlib.h>
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

// Function prototypes

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

/*
* Function: draw
* -----------------------------------------------
* Removes the top card (last in the Deck vector) from the deck and
* inserts it into player's hand
*
* Pre: Deck mutex is locked (in turn() function)
* Post: Player's hand has one more card and deck has one less card
*
* p: player whose turn it is
*
*/
void draw(struct Player* p){
   long drawn = (long)vector_get(&Deck, vector_total(&Deck) - 1);
   printf("\nPLAYER %i DRAWS: ", p->PlayerNumber);
   printf("%ld\n", drawn);
   if (LOGGING) {
       fprintf(fileptr, "PLAYER %i: draws ", p->PlayerNumber);
       fprintf(fileptr, "%ld", drawn);
   }

   vector_add(&p->Hand, (void *)drawn);
   vector_delete(&Deck, vector_total(&Deck)-1);

   print_deck();
}

/*
* Function: discard
* -----------------------------------------------
* Removes either the first or second card (randomly determined) 
* from the player's hand and inserts it into the deck
*
* Pre: Deck mutex is locked (in turn() function)
* Post: Deck has one more card and player's hand has one less card
*
* p: player whose turn it is
*
*/
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
   
   shuffle_deck();


   vector_delete(&p->Hand, discardee);

   print_deck();
}

/*
* Function: turn
* -----------------------------------------------
* When it is any given player's turn, the mutex locks for the deck 
* (deck_mutex) and game status (status_mutex) are obtained. Cards 
* will be removed from the player's hand if said hand has a size greater 
* than or equal to 2. Once the hand size is equal to 1, the player will 
* draw a card from the deck. If the two cards in the hand match, the 
* player wins the round. The deck and game status mutexes are released 
* and the thread terminates.
* 
* Pre: Current player (pl --> p) has 0 or more cards with no matching pairs
* Post: Current player has two cards (that may or may not match)
* 
* pl: player whose turn it is
*
*/
void* turn(void* pl){

   struct Player *p = (struct Player *) pl;
   printf("\n++++++++++++PLAYER %i's TURN++++++++++++\n", p->PlayerNumber);
   
   pthread_mutex_lock(&deck_mutex);

   
   // If player has 2 cards, get rid of one
   if(vector_total(&p->Hand) == 2){
      discard(p);
      print_hand(p);
   }

   // Draw card. Hand size == 2
   draw(p);
   print_hand(p);
   pthread_mutex_lock(&status_mutex);

   // If both cards in player's hand are the same, player wins the round
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

/*
* Function: print_hand
* -----------------------------------------------
* Prints the contents of the player's hand
* 
* p: player with hand to be printed
*
*/
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

/*
* Function: print_deck
* -----------------------------------------------
* Prints the contents of the deck
*
*/
void print_deck(){
   printf("\nDECK:\n");
   for(int i = 0; i < vector_total(&Deck); ++i){
      printf(" %ld ", (long)vector_get(&Deck, i));
   }
   printf("\n");
}

/*
* Function: log_deck
* -----------------------------------------------
* Outputs the contents of the deck to a file
*
*/
void log_deck() {
    if (LOGGING) {
        fprintf(fileptr, "DECK:");
        for(int i = 0; i < vector_total(&Deck); ++i){
            fprintf(fileptr, " %i", (int)vector_get(&Deck, i));
        }
        // fprintf(fileptr, "\n");
    }
}

/*
* Function: shuffle_deck
* -----------------------------------------------
* Shuffles the deck by randomly selecting a "swap" card and
* switching it with the "current" (index j) card
* 
* Pre: Deck has 1 or more cards
* Post: Order of deck contents is different from before shuffle_deck() call
*
*/
void shuffle_deck(){
   int swap, temp;
   for(int i = 0; i < SHUFFLE_COUNT; ++i)
      for(int j = 0; j < vector_total(&Deck); ++j){
         swap = rand() % vector_total(&Deck);
         temp = (long)vector_get(&Deck, j);
         vector_set(&Deck, j, vector_get(&Deck, swap));
         vector_set(&Deck, swap, (void *)temp);
   }

   if (LOGGING) {fprintf(fileptr, "DEALER: shuffles\n");}
}

/*
* Function: create_deck
* -----------------------------------------------
* Initializes a "vector" and appends 
* {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} for each suit, 
* totalling 52 cards.
* 
* The following mappings can be observed:
* 
* Ace --> 1
* Jack --> 11
* Queen --> 12
* King --> 13
* 
* Pre: Deck is an empty container 
* Post: Deck contains {1,...,13,1,...,13,1,...,13,1,...,13}, size = 52
* 
*/
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

/*
* Function: deal
* -----------------------------------------------
* Initializes the player's hand. The card at the "top" of the deck is
* dealt to the player.  
* 
* Pre: Player p has 0 cards in hand (round has just begun)
* Post: Deck has 1 less card. Player hand has 1 more card
* 
* p: player to whom a card is dealt
*
*/
void deal(struct Player* p){
   vector_init(&p->Hand);
   vector_add(&p->Hand, vector_get(&Deck, vector_total(&Deck)-1));
   vector_delete(&Deck, vector_total(&Deck)-1);
   if (LOGGING) {fprintf(fileptr, "DEALER: deals to PLAYER %i", p->PlayerNumber);}
}

/*
* Function: end_round
* -----------------------------------------------
* Releases memory allocated to the containers that housed the deck
* and player's hands
* 
* Post: Dynamically allocated memory used to store the Deck and Hand 
* contents is freed
*
*/
void end_round(){

   printf("ROUND END:");

   if (LOGGING) {
       fprintf(fileptr, "ROUND END STATE:\n");
       print_hand((struct Player *)vector_get(&players, 0));
       print_hand((struct Player *)vector_get(&players, 1));
       print_hand((struct Player *)vector_get(&players, 2));
       print_deck();
       log_deck();
   }
   
   vector_free(&Deck);

   struct Player* p1 = (struct Player *)vector_get(&players, 0);
   vector_free(&p1->Hand);

   struct Player* p2 = (struct Player *)vector_get(&players, 1);
   vector_free(&p2->Hand);

   struct Player* p3 = (struct Player *)vector_get(&players, 2);
   vector_free(&p3->Hand);

}

int main(int argc, char *argv[]){

   // Check for number of arguments
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

   // Use second argument as seed for randomization function
   srand(atoi(argv[1]));

   // Initialize deck mutex
   int a = pthread_mutex_init(&deck_mutex, NULL);
   if(a != 0){
      printf("ERROR: \n\n");
      printf("Return code from line \"int a = pthread_mutex_init(&deck_mutex, NULL);\" is %d\n\n", a);
      return -1;
   }

   // Initialize status mutex
   int b = pthread_mutex_init(&status_mutex, NULL);
   if(b != 0){
      printf("ERROR: \n\n");
      printf("Return code from line \"int b = pthread_mutex_init(&status_mutex, NULL);\" is %d\n\n", a);
      return -1;
   }

   // Create an instance of Player struct for each player and assign PlayerNumber
   struct Player p1 = {.PlayerNumber = 1};
   struct Player p2 = {.PlayerNumber = 2};
   struct Player p3 = {.PlayerNumber = 3};

   // Initialize vector and add "Player" struct instances
   vector_init(&players);
   vector_add(&players, &p1);
   vector_add(&players, &p2);
   vector_add(&players, &p3);

   // Gameplay. Each loop is a round
   for (int round = 1; round <= ROUND_COUNT; ++round){
      // Initialize deck
      create_deck();

      printf("\n---------- ROUND %i ----------\n", round);

      // Dealer deals a card to each player
      for(int i = 0; i < PLAYER_COUNT; ++i){
         deal((struct Player *)vector_get(&players, i));
         print_hand((struct Player *)vector_get(&players, i));
      }
      print_deck();

      int esc;

      while(!WIN){

         // Each player takes a turn
         for(int i = 0; i < THREAD_COUNT; ++i){
            if (WIN){
               break;
            }
            esc = pthread_create(&threads[i], NULL, turn, (vector *)vector_get(&players, i));
            if (esc){ printf("FAILED TO CREATE THREAD");return -1; }
         }

         // Wait for each player thread to finish
         for(int i = 0; i < THREAD_COUNT; ++i){
            if(pthread_join(threads[i],NULL)){
               continue;
            }
         }
      }

      // Exit round
      end_round();
      WIN = false;
   }

   fclose(fileptr);
   pthread_mutex_destroy(&deck_mutex);
   pthread_mutex_destroy(&status_mutex);
   pthread_exit(NULL);
   
}
