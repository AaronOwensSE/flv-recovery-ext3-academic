#include "queue.h"
#include <cstdio>
#include <cstdlib>

queue create_queue() {
	queue new_queue;
	new_queue.head = 0;
	new_queue.tail = 0;
	new_queue.len = 0;

	return new_queue;
}

void enqueue(unsigned int value, queue &q) {
	node *new_node = (node *) malloc(sizeof(node));
	new_node->value = value;
	new_node->prev = q.tail;
	new_node->next = 0;

	if (q.len == 0) {
		q.head = new_node;
	} else {
		q.tail->next = new_node;
	}

	q.tail = new_node;
	q.len++;
}

int find(unsigned int value, queue &q) {
	node *curr = q.head;

	for (int i = 0; curr; curr = curr->next, i++) {
		if (curr->value == value) {
			return i;
		}
	}

	return -1;
}

unsigned int dequeue(queue &q) {
	if (q.len == 0) {
		return 0;
	}

	node *curr = q.head;
	unsigned int value = curr->value;

	if (q.len == 1) {
		q.head = q.tail = 0;
	} else {
		q.head = curr->next;
	}

	free(curr);
	curr = 0;
	q.len--;

	return value;
}

void print_queue(queue &q) {
	if (q.len == 0) {
		return;
	}

	for (node *curr = q.head; curr; curr = curr->next) {
		printf("0x%X (%d)", curr->value, curr->value);

		if (curr->next) {
			printf(", ");
		}
	}

	printf("\n");
}

void delete_queue(queue &q) {
	if (q.len == 0) {
		return;
	}

	node *curr = q.head;
	node *next = curr->next;

	while (next) {
		free(curr);
		curr = next;
		next = next->next;
	}

	free(curr);
	curr = 0;
}
