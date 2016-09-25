#include <stdlib.h>
#include <ucontext.h>


typedef struct MyThread
{
	ucontext_t context;
	struct MyThread *blockedWith;
	struct MyThread *parent;
	struct Queue *children;
}_MyThread;

typedef struct QueueNode 
{
	struct MyThread *thread;
	struct QueueNode *next;
}_QueueNode;

typedef struct Queue
{
	_QueueNode *front;
	_QueueNode *rear;
}_Queue;

typedef struct MySemaphore
{
	int value;
	_Queue *blockedQueue;
}_MySemaphore;

void initQueue(_Queue *q);

void enqueue(_Queue *q, _MyThread *thread);

struct MyThread *dequeue(_Queue *q);

int isQueueEmpty(_Queue *q);

int isPresentInQueue(_Queue *q, _MyThread *thread);

int removeFromQueue(_Queue *q, _MyThread *thread);
