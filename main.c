#include <stdio.h>
#include <stdlib.h>

#define OFFSET (-45)
#define CHARLENG 78
#define BLOCK 32768

enum color{RED, BLACK};
enum bool{TRUE, FALSE};

typedef struct node{
    char *word;
    enum color color;
    enum bool filter;
    struct node *p, *r, *l, *next;
}node;

short unsigned int ist[CHARLENG] = {0}, cor[CHARLENG] = {0}, err[CHARLENG] = {0};
unsigned short int atLeast[CHARLENG] = {0};
short int exactly[CHARLENG] = {-1};
int stringDim = 0, counter = BLOCK, countStr = BLOCK, i = 0, min = 20, max = 1;
node *freeNodes = NULL, *possibles = NULL, *previous = NULL;
node nil;
char *freeString = NULL;

void setIst(const char*);                                       //Calculate Ist of reference word r
unsigned short int compare(const char*, const char*, char*);    //Calculates the comparison of r and p in res
char* createString(int, const char*);                           //Creates an empty string of the length + 2 for \n and \0
node* createNode(char*);                                        //Creates a red node with the value passed and return a pointer to it
node* insert(node*, node*);                                     //Insert the node into the tree
node* fix(node*, node*);                                        //Fix the property of red-black tree
short unsigned int search(node*, char*);                        //Return 1 if the dictionary constraints the word, otherwise 0
node* left_rotate(node*, node*);
node* right_rotate(node*, node*);
short unsigned int greater(const char*, const char*);           //return 1 if p1 greater than p2, 2 if equals and 0 otherwise
void constraints(const char*, const char*, char*, unsigned short int[stringDim][CHARLENG]);               //Generates the arrays used to check the constraints
unsigned short int checkConstraints(const char*, const char*, unsigned short int[stringDim][CHARLENG]);   //Return 1 if the word respect the constraints, 0 if not
void reset(char*, unsigned short int[stringDim][CHARLENG]);                                               //Reset to default values the array of constraints
void printFiltered(node*);                                                                                //Print all the words in the dictionary that respect the constraints
unsigned int calculatePossible(char*, unsigned short int[stringDim][CHARLENG]);                           //Calculate the number of words in the dictionary that respect the constraints and mark the others
void reset_dictionary(node*);       //Reset the constraints mark
void insert_possibles(node*);
node* maximum(node*);
node* predecessor(node*);

int main(int argc, char const *argv[]) {
  //Creation of dictionary and default node nil
  node *dictionary = &nil;
  char* temp, *end;
  int useless, f = 1;
  nil.color = 'b';
  nil.word = "nil\n";
  nil.l = &nil;
  nil.r = &nil;
  nil.p = &nil;
  nil.next = &nil;

  //Input dim
  useless = scanf(" %d\n", &stringDim);

  //Deciding buffer length for fgets
  if(stringDim + 3 < 20)
    min = 20;
  else
    min = stringDim + 3;

  char r[stringDim], p[min], res[stringDim+1];
  char correct[stringDim];
  unsigned short int wrong[stringDim][CHARLENG];
  res[stringDim] = '\0';
  //----------------------TEST---------------------------//
  temp = malloc(min+1);
  end = fgets(temp, min, stdin);

  while(temp[0] != '+'){
    dictionary = insert(dictionary, createNode(createString(stringDim, temp)));
    end = fgets(temp, min, stdin);
  }

  previous = NULL;
  reset(correct, wrong);          //Reset constraints
  reset_dictionary(dictionary);   //Reset possibles
  p[0] = temp[0];
  p[1] = temp[1];

  while(end){
    if(p[0] == '+'){
      switch(p[1]){
        case 'n' :{
          previous = NULL;
          reset(correct, wrong);
          reset_dictionary(dictionary);
          end = fgets(r, min, stdin);
          setIst(r);
          useless = fscanf(stdin, "%d\n", &max);
          if(f)
            f = 0;
          else
            printf("\n");
          break;
        }

        case 's' :
          printFiltered(possibles);
          break;

        case 'i' :{
          end = fgets(temp, min, stdin);

          while(temp[0] != '+'){
            node *n = createNode(createString(stringDim, temp));
            dictionary = insert(dictionary, n);

            if(checkConstraints(temp, correct, wrong))
              insert_possibles(n);
            else
              n->filter = FALSE;

            end = fgets(temp, min, stdin);
          }
          break;
        }

        default :
          break;
      }
    }
    else{
      if(search(dictionary, p)){
        if(compare(r, p, res)){
          printf("ok");
        }
        else{
          constraints(p, res, correct, wrong);
          printf("%s\n%d\n", res, calculatePossible(correct, wrong));
          max--;

          if(max <= 0){
            printf("ko");
          }
        }
      }
      else{
        printf("not_exists\n");
      }
    }


    end = fgets(p, min, stdin);
  }

  useless++;

  return 0;
}

void setIst(const char* r){
  for(i = 0; i < CHARLENG; i++){
    ist[i] = 0;
  }
  //Ist of R preparation
  for(i = 0; i < stringDim; i++){
    ist[(int)r[i] + OFFSET]++;
  }
}

unsigned short int compare(const char* r, const char* p, char* res){
  unsigned int ok = 0;

  //First control counter the number of correct characters
  for(i = 0; i < stringDim; i++){
    if(p[i] == r[i]){               //If letters are equals '+'
      res[i] = '+';
      cor[(int)p[i] + OFFSET]++;
      ok++;
    }
  }

  //If the word is correct returns 1
  if(ok == stringDim)
    return 1;

  //Second control assign the characters '|' or '/'
  for(i = 0; i < stringDim; i++){
    if(p[i] != r[i]){
      if(err[(int)p[i] + OFFSET] >= ist[(int)p[i] + OFFSET] - cor[(int)p[i] + OFFSET])    //Check property of |(pipe)
        res[i] = '/';
      else
        res[i] = '|';

      err[(int)p[i] + OFFSET]++;
    }
  }

  //Reset of counters
  for(i = 0; i < CHARLENG; i++){
    cor[i] = 0;
    err[i] = 0;
  }

  return 0;
}

char* createString(int dim, const char *word){
  if(countStr >= BLOCK){
    freeString = malloc(BLOCK * (dim+1));
    countStr = 0;
  }
  for(i = 0; i < dim; i++)
    freeString[countStr*(dim+1) + i] = word[i];

  freeString[countStr*(dim+1) + dim] = '\0';
  countStr++;

  return &freeString[(countStr-1)*(dim+1)];
}

node* createNode(char *p){
  if(counter >= BLOCK){
    freeNodes = malloc(BLOCK * sizeof(node));
    counter = 0;
  }

  freeNodes[counter].word = p;
  freeNodes[counter].color = RED;
  freeNodes[counter].filter = TRUE;
  freeNodes[counter].next = &nil;
  freeNodes[counter].p = &nil;
  counter++;

  return &freeNodes[counter - 1];
}

node* insert(node *dict, node* w){
  node *y = &nil, *x = dict;

  while(x != &nil){
    y = x;
    if(!greater(w->word, x->word))
      x = x->l;
    else
      x = x->r;
  }
  w->p = y;

  if(y == &nil)
    dict = w;
  else if(!greater(w->word, y->word))
    y->l = w;
  else
    y->r = w;

  w->l = &nil;
  w->r = &nil;

  return fix(dict, w);
}

short unsigned int search(node *dict, char *word){
  node *x = dict;
  short unsigned int n;

  while(x != &nil){
    n = greater(word, x->word);
    if(n == 2)
      return 1;
    else{
      if(n == 1)
        x = x->r;
      else
        x = x->l;
    }
  }

  return 0;
}

node* fix(node *dict, node *word){
  node *y;

  if(word == dict){
    word->color = BLACK;
    return dict;
  }

  while(word->p->color == RED){
    if(word->p == word->p->p->l){
      y = word->p->p->r;
      if(y->color == RED){
        word->p->color = BLACK;
        y->color = BLACK;
        word->p->p->color = RED;
        word = word->p->p;
      }
      else{
        if(word == word->p->r){
          word = word->p;
          dict = left_rotate(dict, word);
        }
        word->p->color = BLACK;
        word->p->p->color = RED;
        dict = right_rotate(dict, word->p->p);
      }
    }
    else{
      y = word->p->p->l;
      if(y->color == RED){
        word->p->color = BLACK;
        y->color = BLACK;
        word->p->p->color = RED;
        word = word->p->p;
      }
      else{
        if(word == word->p->l){
          word = word->p;
          dict = right_rotate(dict, word);
        }
        word->p->color = BLACK;
        word->p->p->color = RED;
        dict = left_rotate(dict, word->p->p);
      }
    }
  }
  dict->color = BLACK;

  return dict;
}

node* left_rotate(node *dict, node *x){
  node *y = x->r;
  x->r = y->l;

  if(y->l != &nil)
    y->l->p = x;
  y->p = x->p;

  if(x->p == &nil)
    dict = y;
  else if(x == x->p->l)
    x->p->l = y;
  else
    x->p->r = y;

  y->l = x;
  x->p = y;
  return dict;
}

node* right_rotate(node *dict, node *x){
  node *y = x->l;
  x->l = y->r;

  if(y->r != &nil)
    y->r->p = x;
  y->p = x->p;

  if(x->p == &nil)
    dict = y;
  else if(x == x->p->r)
    x->p->r = y;
  else
    x->p->l = y;

  y->r = x;
  x->p = y;
  return dict;
}

short unsigned int greater(const char *p1, const char *p2){
  for(i = 0; i < stringDim; i++){
    if(p1[i] > p2[i])
      return 1;
    if(p1[i] < p2[i])
      return 0;
  }
  return 2;
}

void constraints(const char *p, const char *res, char *corr, unsigned short int wrg[stringDim][CHARLENG]){
  unsigned short int atLeastTemp[CHARLENG] = {0};

  for(i = 0; i < stringDim; i++){
    if(res[i] == '+'){
      corr[i] = p[i];     //Assign the correct letter at the correct position
      atLeastTemp[(int)p[i] + OFFSET]++;    //Increase the number of instance of that character that are present at least
      if(exactly[(int)p[i] + OFFSET] != -1)  //If exactly number as  already been set it updates it
        exactly[(int)p[i] + OFFSET]++;
    }
    else{
      wrg[i][(int)p[i] + OFFSET]++;   //Setting character wrong in that position

      if(res[i] == '|'){
        atLeastTemp[(int)p[i] + OFFSET]++;     //Increase the number of instance of that character that are present at least
      }
      else{
        exactly[(int)p[i] + OFFSET] = atLeastTemp[(int)p[i] + OFFSET];
      }
    }
  }

  for(i = 0; i < CHARLENG; i++){
    if(atLeastTemp[i] > atLeast[i])
      atLeast[i] = atLeastTemp[i];
  }
}

unsigned short int checkConstraints(const char* word, const char* corr, unsigned short int wrg[stringDim][CHARLENG]){
  unsigned short int count[CHARLENG] = {0};

  for(i = 0; i < stringDim; i++){
    if(corr[i] == '/'){
      if(wrg[i][(int)word[i] + OFFSET]){    //Check if position i is a forbidden char in that position
        return 0;
      }
    }
    else{
      if(word[i] != corr[i])    //Check if char is different from the correct one
        return 0;
    }
    //Counts the occurrence of any char
    count[(int)word[i] + OFFSET]++;   //Increase the counter of characters and check if exceed the limit
    if(exactly[(int)word[i] + OFFSET] != -1 && count[(int)word[i] + OFFSET] > exactly[(int)word[i] + OFFSET])
      return 0;
  }

  for(i = 0; i < CHARLENG; i++){    //Check all the at least character
    if(count[i] < atLeast[i])
      return 0;
  }

  return 1;
}

void reset(char *corr, unsigned short int wrg[stringDim][CHARLENG]){
  short int k;

  for(i = 0; i < stringDim; i++){
    for(k = 0; k < CHARLENG; k++){
      wrg[i][k] = 0;
    }
    corr[i] = '/';
  }
  for(i = 0; i < CHARLENG; i++){
    atLeast[i] = 0;
    exactly[i] = -1;
  }
}

void printFiltered(node* n){
  node *curr;

  for(curr = n; curr != &nil; curr = curr->next)
    puts(curr->word);
}

unsigned int calculatePossible(char *corr, unsigned short int wrg[stringDim][CHARLENG]){
  node *curr, *prev;
  unsigned int poss, set = 0;

  for(curr = possibles, prev = NULL, poss = 0; curr != &nil; curr = curr->next){
    if(!checkConstraints(curr->word, corr, wrg)){
      curr->filter = FALSE;         //Word is not possible set the flag filter to FALSE
      set++;                        //Set flag set to attach the previous possible word
    }
    else{                     //Word is possible
      poss++;                 //Increase counter of possible words
      if(set){                //Check if it needs to be attached to the previous
        if(prev == NULL){
          possibles = curr;
        }
        else{
          prev->next = curr;
        }
        set = 0;
      }
      prev = curr;        //Update previous to current possible word
    }
  }
  prev->next = &nil;

  return poss;
}

void reset_dictionary(node* node){
  if(node != &nil){
    reset_dictionary(node->l);
    node->next = &nil;
    node->filter = TRUE;
    if(previous)
      previous->next = node;
    else
      possibles = node;
    previous = node;
    reset_dictionary(node->r);
  }
}

void insert_possibles(node *n){
  //Prev in possibles
  node *prev = predecessor(n);

  if(prev == &nil){
    n->next = possibles;
    possibles = n;
  }
  else{
    n->next = prev->next;
    prev->next = n;
  }
}

node* maximum(node *n){
  while(n->r != &nil){
    n = n->r;
  }

  return n;
}

node* predecessor(node *n){
  node *y, *prev;

  do{
    prev = maximum(n->l);

    if(prev == &nil){
      y = n->p;
      while(y != &nil && n == y->l){  //first
        n = y;
        y = y->p;
      }
      prev = y;
    }

    n = prev;
  }
  while(prev != &nil && prev->filter == FALSE);

  return prev;
}