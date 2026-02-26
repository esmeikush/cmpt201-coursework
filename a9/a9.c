#include <stdio.h>

struct Node {
  int data;
  struct Node *next;
}

// to add a node to the head.
struct Node *new_node = malloc(sizeof(struct Node));

new_node->data = 42;
// new_node -> Nothing
// head that points to smth 

// new_node's address to point to  -> old_head 
// now head needs to have updated address of new_node.

new_node->next = head;
head = new_node;

// how to traverse a linked list. 

// need a temp pointer to wall along without moving head 

struct Node *curr = head; 

while ( curr != NULL) { 
  // need to look for a block big enough to satisfy request of allocation..
  if (curr->size >= request_size) { 
    chosen = curr;
    break;
  }
  curr = curr->next; // move to next node
  
}


// use size 
struct Node {
  int size;
struct Node *next; 
}

