#include <stdio.h>
#include <stdlib.h>
#define OK 1
#define ERROE 0
#define OVERFLOW -2

typedef int Status;
typedef int QElemType;
typedef struct Node
{
    QElemType data;
    struct Node *next;
} QNode;
/* 当链式队列的头尾节点指针定义成为一个单独的结构体,避免在新增节点时占用过多的空间 */
typedef struct
{
    QNode *front;
    QNode *rear;
} LinkQueue;


Status InitQueue(LinkQueue *Q)
{
    Q->front = Q->rear = (QNode *)malloc(sizeof(QNode));
    if (!Q->front)
        exit(OVERFLOW);
    Q->rear->next = NULL; //rear.next始终指向NULL,头结点front不动
    return OK;
}

Status InQueue (LinkQueue *Q, int elem)
{
    QNode *p;
    p = (QNode *)malloc(sizeof(QNode));
    p->data = elem;
    p->next = NULL;
    Q->rear->next = p;
    Q->rear = p;
    return OK;
}

Status PrintQueue(LinkQueue Q)
{
    QNode *p;
    p = Q.front->next;
    printf("the queue is:");
    while(p != NULL)
    {
        printf("%d ", p->data);
        p = p->next;
    }
    return OK;
}

Status OutQueue(LinkQueue *Q)
{
    QNode *p;
    int i;
    printf("\nthe number of out queue:");
    scanf("%d", &i);
    while(i != 0)
    {
        p = Q->front;
        Q->front = Q->front->next;
        free(p);
        i--;
    }
    p = NULL;
    return OK;
}

Status EmptyQueue(LinkQueue Q)
{
    if (Q.front->next == NULL)
        printf("\nThe queue is empty!\n");
    return OK;
}

int main(int argc, char const *argv[])
{
     printf("hello\n");
     LinkQueue queue;
     InitQueue(&queue);

     for (int i = 0; i < 10; ++i)
     {
        InQueue(&queue, i);
     }
      PrintQueue(queue);
     // OutQueue(&queue);
     // PrintQueue(queue);
     // EmptyQueue(queue);

     return OK;

}