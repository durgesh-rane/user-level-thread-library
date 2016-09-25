#include <stdlib.h>
#include <ucontext.h>
#include "myqueue.h"
#include "mythread.h"

#define STACKSIZE 8192

Queue *readyQueue;
Queue *blockedQueue;
_MyThread *currentThread;
ucontext_t initContext;

_MyThread *ThreadInit(void (*start_funct)(void *), void *args)
{
	_MyThread *initThread = (_MyThread *)malloc(sizeof(_MyThread));
	getcontext(&(initThread->cxt));
	(initThread->cxt).uc_link = NULL;
	(newThread->cxt).uc_stack.ss_sp = malloc (STACKSIZE*8);
	(newThread->cxt).uc_stack.ss_size = STACKSIZE;
	initThread->blockedWith = NULL;
	initThread->parent = NULL;
	initThread->children = (_Queue *)malloc(sizeof(_Queue));
	initializeQueue(initThread->children);
		
	makecontext(&(initThread->cxt), (void(*)())start_funct, 1, args);
	
	return newThread;
}

void MyThreadInit (void(*start_funct)(void *), void *args)
{
	getcontext(&initContext)
	readyQueue = (Queue *)malloc(sizeof(Queue));
	blockedQueue = (Queue *)malloc(sizeof(Queue));
	initializeQueue(readyQueue);
	initializeQueue(blockedQueue);
	
	currentThread = ThreadInit(start_funct, args);
	swapcontext(&initContext, &(currentThread->cxt));
}

MyThread MyThreadCreate (void(*start_funct)(void *), void *args)
{
	_MyThread *createThread = ThreadInit(start_funct, args);
	createThread->parent = currentThread;
	enqueue(readyQueue, createThread);	
	enqueue(currentThread->children, createThread);
	return (MyThread)createThread;	
}

void MyThreadYield(void)
{
	_MyThread *yieldThread = currentThread;
	enqueue(readyQueue, yieldThread);
	_MyThread *runThread = dequeue(readyQueue);
	currentThread = runThread;
	swapcontext(&(yieldThread->context),&(currentThred->context));	
}

int MyThreadJoin(MyThread thread)
{
	_MyThread *joinThread = (_MyThread *)thread;
	
	if(!joinThread)
		return -1;
	if(joinThread->parent != currentThread)
		return -1;
	if(!isPresentInQueue(currentThread->children, joinThread)
		return 0;
		
	_MyThread *blockThread = currentThread;

	currentThread->blockedWith = joinThread;
	enqueue(blockedQueue, blockThread);
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
	return 0;	
}

void MyThreadJoinAll(void)
{
	if(isQueueEmpty(currentThread->children)
		return;
	_MyThread *blockThread = currentThread;
	enqueue(blockedQueue, blockThread);
	_MyThread *runThread = dequeue(readyQueue);
	if(!runThread)
	{
		setcontext(&initcontext);
	}
	else
	{
		currentThread = runThread;
		swapcontext(&(blockThread->context), &(currentThread->context));
	}
}

void MyThreadExit(void)
{
	_MyThread *exitThread = currentThread;
	_MyThread *parentThread = exitThread->parent;
	if(parentThread)
	{
		removeFromQueue(parentThread->children, exitThread);
		if(parentThread->blockedWith == exitThread || isQueueEmpty(parentThread->children)
		{
			removeFromQueue(blockedQueue, parentThread);
			enqueue(readyQueue, parentThread);
		}
	}
	_QueueNode *exitChildNode = exitThread->chidlren->front;
	while(exitChildNode)
	{
		exitChildNode->thread->parent = NULL;
		exitChildNode = exitChildNode->next;
	}

	free((exitThread->cxt).uc_stack.ss_sp);	
	free(exitThread->blockedWith);
	exitThread->blockedWith = NULL;
	free(exitThread->parent);
	exitThread->parent = NULL;
	free(exitThread->children);
	exitThread->children = NULL;
	free(exitThread);
	exitThread = NULL;
	
	_MyThread *runThread = dequeue(readyQueue);	
	if(!runThread)
	{
		setcontext(&initContext);
	}
	else
	{
		currentThread = runThread;
		setcontext(&(currentThread->context));			
	}	
}

MySemaphore MySemaphoreInit(int initialValue)
{
	if(initialValue<0)
		return NULL;
	
	_MySemaphore semaphore = (_MySemaphore *)malloc(sizeof(_MySemaphore));
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
	_MySemaphore *semaphore = (MySemaphore *)sem;
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
	_MySemaphore *semaphore = (MySemaphore *)sem;
	if(!semaphore)
		return -1;
	if(!isQueueEmpty(semaphore->blockedQueue))
		return -1;

	free(semaphore->blockedQ);
	semaphore->blockedQ = NULL;
	free(semaphore);
	semaphore = NULL;
	return 0;
}