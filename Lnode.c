#include<stdio.h>
#include<stdlib.h>
#define false 0
#define ok 1


typedef struct node {
    int data;
    struct node *next;
}node,*linklist;



void creatlist(linklist l, int n)
{
    linklist p;
    //去掉了l = (linklist)malloc(sizeof(node)); 
    l->next = NULL;
    int i;
    for (i = 0; i < n; i++)
    {
        p = (linklist)malloc(sizeof(node));
        p->data = i;
        p->next = l->next;
        l->next = p;
    }
}


int getelem(linklist l, int i, int *e)
{
    int k = 1;
    linklist p = NULL;
    p = l->next;
    while (p&&k<i)
    {
        p = p->next;
        k++;
    }
    if (!p || k>i)
        return false;
    *e = p->data;
    return ok;
}


int listinsert(linklist l, int i, int e)
{
    int k = 1;
    linklist s = NULL, p = NULL;
    p = l;
    int j = 1;
    while (p&&j<i)
    {
        p = p->next;
        j++;
    }
    if (!p || k>i)
        return false;
    s = (linklist)malloc(sizeof(node));
    s->data = e;
    s->next = p->next;
    p->next = s;
    return ok;
}


int listdelete(linklist l, int i, int *e)
{
    int j = 1;
    linklist p = NULL, q = NULL;
    p = l;
    while (p&&j<i)
    {
        p = p->next;
        j++;
    }
    if (!p->next || j>i)
        return false;
    q = p->next;
    p->next = q->next;
    *e = q->data;
    free(q);
    return ok;
}


int seelist(linklist l)
{
    linklist p = NULL;
    p = l->next;
    int k = 0;
    while (p)
    {
        printf("%4d", p->data);
        p = p->next;
        k++;
    }
    printf("\n");
    if (k == 0)
    {
        printf("链表为空");
        return false;
    }
    return ok;
}


int clearlist(linklist l)
{
    linklist p = NULL, q = NULL;
    p = l->next;
    while (p)
    {
        q = p->next;
        free(p);
        p = q;
    }
    l->next = NULL;
    return ok;
}


int main(void)
{
    int a;
    linklist lb = (linklist)malloc(sizeof(node));//这里有变动！
    creatlist(lb, 5);
    seelist(lb);
    listinsert(lb, 2, 0);
    seelist(lb);
    listdelete(lb, 2, &a);
    seelist(lb);
    clearlist(lb);
    return 0;
}
