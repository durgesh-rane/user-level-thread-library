#include <stdlib.h>
#include <ucontext.h>
#include "myqueue.h"
#include "mythread.h"
#include <stdio.h>
#define STACKSIZE 8192

_Queue *readyQueue;
_Queue *blockedQueue;
_MyThread *currentThread;
ucontext_t initContext;

_MyThread *ThreadInit(void (*start_funct)(void *), void *args)
{
	_MyThread *initThread = (_MyThread *)malloc(sizeof(_MyThread));
	getcontext(&(initThread->context));
	(initThread->context).uc_link = NULL;
	(initThread->context).uc_stack.ss_sp = malloc (STACKSIZE*8);
	(initThread->context).uc_stack.ss_size = STACKSIZE;
	initThread->blockedWith = (_MyThread *)malloc(sizeof(_MyThread));
	initThread->blockedWith = NULL;
	initThread->parent = (_MyThread *)malloc(sizeof(_MyThread));
	initThread->parent = NULL;
	initThread->children = (_Queue *)malloc(sizeof(_Queue));
	initQueue(initThread->children);
	initThread->joinAll = 0;
		
	makecontext(&(initThread->context), (void(*)())start_funct, 1, args);
	return initThread;
}

void MyThreadInit (void(*start_funct)(void *), void *args)
{
	getcontext(&initContext);
	readyQueue = (_Queue *)malloc(sizeof(_Queue));
	blockedQueue = (_Queue *)malloc(sizeof(_Queue));
	initQueue(readyQueue);
	initQueue(blockedQueue);
	
	currentThread = ThreadInit(start_funct, args);
	swapcontext(&initContext, &(currentThread->context));
}

MyThread MyThreadCreate (void(*start_funct)(void *), void *args)
{
	_MyThread *createThread = ThreadInit(start_funct, args);
	createThread->parent = currentThread;
	enqueue(readyQueue, createThread);	
	enqueue(currentThread->children, createThread);
	//printf("Created thread: %d\n",createThread);
	return (MyThread)createThread;	
}

void MyThreadYield(void)
{
	_MyThread *yieldThread = currentThread;
	enqueue(readyQueue, yieldThread);
	_MyThread *runThread = dequeue(readyQueue);
	currentThread = runThread;
	swapcontext(&(yieldThread->context),&(currentThread->context));	
}

int MyThreadJoin(MyThread thread)
{
	_MyThread *joinThread = (_MyThread *)thread;
	
	if(!joinThread)
		return -1;
	if(joinThread->parent != currentThread)
		return -1;
	if(!isPresentInQueue(currentThread->children, joinThread))
		return 0;
		
	_MyThread *blockThread = currentThread;

	currentThread->blockedWith = joinThread;
	enqueue(blockedQueue, blockThread);
	_MyThread *runThread = dequeue(readyQueue);
	//if(!runThread)
	//{
	//	setcontext(&initContext);
	//}
	//else
	//{
		currentThread = runThread;
		swapcontext(&(blockThread->context), &(currentThread->context));
	//}
	return 0;	
}

void MyThreadJoinAll(void)
{
	if(isQueueEmpty(currentThread->children))
		return;
	_MyThread *blockThread = currentThread;
	blockThread->joinAll = 1;
	enqueue(blockedQueue, blockThread);
	_MyThread *runThread = dequeue(readyQueue);
	//if(!runThread)
	//{
	//	setcontext(&initContext);
	//}
	//else
	//{
		currentThread = runThread;
		swapcontext(&(blockThread->context), &(currentThread->context));
	//}
}

int NumberOfNodesInQueue(_Queue *q)
{
	int count = 0;
	_QueueNode *queueNode = q->front;
	while(queueNode)
	{
		//printf("Thread info: %d\n", queueNode->thread);
		count = count+1;
		queueNode = queueNode->next;
	}
	return count;
}

void MyThreadExit(void)
{
	int readyQueueCount = NumberOfNodesInQueue(readyQueue);
	//printf("Ready Queue count %d\n",readyQueueCount);
	//printf("Current thread: %d\n", currentThread);
	//printf("in my thread exit\n");
	_MyThread *exitThread = currentThread;
	_MyThread *parentThread = exitThread->parent;
	if(parentThread)
	{
		//printf("in if condition parentThread: %d\n",parentThread);
		//if(removeFromQueue(parentThread->children, exitThread))
		//	printf("Successfully removed from parent thread's children queue\n");
		removeFromQueue(parentThread->children, exitThread);
		if(parentThread->blockedWith == exitThread || (parentThread->joinAll && isQueueEmpty(parentThread->children)) )
		{
		//	printf("in if condition blocked queue parentThread\n");
			if(removeFromQueue(blockedQueue, parentThread))
			{
		//		printf("Successfully removed from blockedQueue\n");
				enqueue(readyQueue, parentThread);
			}
		}
	}
	if(!isQueueEmpty(exitThread->children))
	{
		_QueueNode *exitChildNode = exitThread->children->front;
		while(exitChildNode)
		{
			//printf("in while loop to mark parent null\n");
			_MyThread *childThread = exitChildNode->thread;
			childThread->parent = NULL;
			exitChildNode = exitChildNode->next;
		}
	}
	//removeFromQueue(readyQueue, exitThread);
	//removeFromQueue(blockedQueue, exitThread);
	//free((exitThread->context).uc_stack.ss_sp);
	exitThread->blockedWith = NULL;	
	//free(exitThread->blockedWith);
	exitThread->parent = NULL;
	//free(exitThread->parent);
	exitThread->children = NULL;
	//free(exitThread->children);
	exitThread= NULL;
	//free(exitThread);
	
	_MyThread *runThread = dequeue(readyQueue);	
	if(!runThread)
	{
		//printf("in set context to init\n");
		setcontext(&initContext);
	}
	else
	{
		//printf("in set context to dequeued thread\n");
		currentThread = runThread;
		//printf("current thread %d\n",currentThread);
		//printf("current thread context %d\n", &(currentThread->context));
		setcontext(&(currentThread->context));			
	}	
}

MySemaphore MySemaphoreInit(int initialValue)
{
	if(initialValue<0)
		return NULL;
	
	_MySemaphore *semaphore = (_MySemaphore *)malloc(sizeof(_MySemaphore));
	semaphore->value = initialValue;
	semaphore->blockedQueue = (_Queue *)malloc(sizeof(_Queue));
	initQueue(semaphore->blockedQueue);
	return (MySemaphore)semaphore;
}

void MySemaphoreWait(MySemaphore sem)
{
	_MySemaphore *semaphore = (_MySemaphore *)sem;
	semaphore->value = semaphore->value - 1;
	if(semaphore->value < 0)
	{
		_MyThread *blockThread = currentThread;
		enqueue(semaphore->blockedQueue, currentThread);
		_MyThread *runThread = dequeue(readyQueue);
		if(!runThread)
		{
			setcontext(&initContext);
		}
		else
		{
			currentThread = runThread;
			swapcontext(&(blockThread->context), &(currentThread->context));
		}
	}
}

void MySemaphoreSignal(MySemaphore sem)
{
	_MySemaphore *semaphore = (_MySemaphore *)sem;
	semaphore->value = semaphore->value + 1;
	if(semaphore->value > 0)
		return;
	if(!isQueueEmpty(semaphore->blockedQueue))
	{
		_MyThread *runThread = dequeue(semaphore->blockedQueue);
		enqueue(readyQueue, runThread);
	}	
}

int MySemaphoreDestroy(MySemaphore sem)
{
	_MySemaphore *semaphore = (_MySemaphore *)sem;
	if(!semaphore)
		return -1;
	if(!isQueueEmpty(semaphore->blockedQueue))
		return -1;

	free(semaphore->blockedQueue);
	free(semaphore);
	return 0;
}
