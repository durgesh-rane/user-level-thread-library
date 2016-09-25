#include <stdlib.h>
#include <ucontext.h>
#include "myqueue.h"
#include "mythread.h"

void initQueue(_Queue *q)		//DONE
{
  q->front = NULL; 
  q->rear = NULL;
}

void enqueue(_Queue *q, _MyThread *thread)	//DONE
{
	_QueueNode *tempNode = (_QueueNode *)malloc(sizeof(_QueueNode));
	tempNode->thread = thread;
	tempNode->next = NULL;
	if(isQueueEmpty(q))
	{
		q->front = tempNode;
		q->rear = tempNode;
	}
	else
	{
		q->rear->next = tempNode;
		q->rear = tempNode;
	}
}

_MyThread *dequeue(_Queue *q)
{
	if(isQueueEmpty(q))
	{
		return NULL;
	}
	else
	{
		_MyThread *frontThread = q->front->thread;
		_QueueNode *tempNode = q->front;		
		if(q->front == q->rear)
		{
			initQueue(q);
		}
		else
		{
			q->front = q->front->next;
		}
		free(tempNode);		
		tempNode = NULL;
		return(frontThread);
	}
}

int isQueueEmpty(_Queue *q)	//DONE
{
	if(q->front == NULL && q->rear == NULL)
		return 1;
	else
		return 0;
}

int isPresentInQueue(_Queue *q, _MyThread *thread)	
{
	_QueueNode *tempNode = q->front;
	while(tempNode)
	{
		if(tempNode->thread == thread)
			return 1;
		tempNode = tempNode->next;
	}
	return 0;
}

int removeFromQueue(_Queue *q, _MyThread *thread)
{
	if(q->front == NULL || q->rear == NULL)
		return 0;
	
	_QueueNode *tempNode, *prevNode;
	prevNode = NULL;
	tempNode = q->front;
	
	while(tempNode != NULL && tempNode->thread != thread)
	{
		prevNode = tempNode;
		tempNode = tempNode->next;
	}
	if(tempNode == NULL)
		return 0;
	if(tempNode == q->front && tempNode == q->rear)
	{
		initQueue(q);
		prevNode = NULL;
	}
	else if(tempNode == q->front)
		q->front = tempNode->next;
	else if(tempNode == q->rear)
	{
		q->rear = prevNode;
		q->rear->next = NULL;
	}
	else
		prevNode->next = tempNode->next;
	free(tempNode);
	tempNode = NULL;
	return 1;
}

