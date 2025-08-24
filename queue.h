#ifndef QUEUE_H
#define QUEUE_H

// A linked list queue for unsigned integers.

struct node {
	node *prev;
	node *next;
	unsigned int value;
};

struct queue {
	node *head;
	node *tail;
	unsigned int len;
};

// Struct Functions
queue create_queue();
void enqueue(unsigned int value, queue &q);
int find(unsigned int value, queue &q);
unsigned int dequeue(queue &q);
void print_queue(queue &q);
void delete_queue(queue &q);

#endif
