#pragma once
#include <stdio.h>

#define NEXT(index)		((index + 1) % 256)

struct Queue {
	int buf[256];
	int front;
	int rear;
	int count;
};

void InitQueue(Queue* queue);
int IsFull(Queue* queue);
int IsEmpty(Queue* queue);
void Enqueue(Queue* queue, int data);
int Dequeue(Queue* queue);

void InitQueue(Queue* queue) {
	queue->front = queue->rear = 0;
	queue->count = 0;
}

int IsFull(Queue* queue) {
	return queue->count == 256;
}

int IsEmpty(Queue* queue) {
	return queue->count == 0;
}

void Enqueue(Queue* queue, int data) {
	if (IsFull(queue)) {
		printf("Queue is full\n");
		return;
	}
	queue->buf[queue->rear] = data;
	queue->rear = NEXT(queue->rear);
	queue->count++;
}

int Dequeue(Queue* queue) {
	int re = 0;
	if (IsEmpty(queue)) {
		printf("Queue is empty\n");
		return re;
	}
	re = queue->buf[queue->front];
	queue->front = NEXT(queue->front);
	queue->count--;
	return re;
}

int Qsize(Queue* queue) {
	return queue->count;
}