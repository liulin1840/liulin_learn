
#include<stdio.h>
#include<stdlib.h>
//结构体定义
typedef struct _node  
{  
    int data;  //链表中的数据域   
    struct _node *next;         //链表的下一跳
    struct _node *prior;        //链表的上一跳
}LinkNode,*DLinkedList;

//双链表
//创建结点
//sizeof(指针) = 4 字节
DLinkedList Dlist_creat_node(int data)
{
    DLinkedList node = (DLinkedList)malloc(sizeof(LinkNode));
    if(NULL == node)
    {
        perror("Dlist_creat_node err");
        return NULL;
    }
    node->data = data;
    node->next = NULL;
    node->prior = NULL;

    return node;
}
//头插法,就是把新的节点放到L的后面
/**
 * [Dlist_insert_head description]
 * @作者     liulin
 * @时间     2018-11-07T17:12:47+0800
 * @描述
 * @param  L                        [description]
 * @param  data                     [description]
 * @return                          [description]
 */
int Dlist_insert_head(DLinkedList L,int data)
{
    DLinkedList p = Dlist_creat_node(data);
    if(NULL == p)
    {
        perror("Dlist_creat_node err");
        return -1;
    }
    // 先把申请的节点的前后连接好
    p->next = L->next;
    p->prior = L;

    // [] -->  <--- [] 然后再两头指 
    L->next->prior = p;
    L->next = p;
    return 0;
}
//尾插法
int DlistInsertTail(DLinkedList L,int data)
{
    DLinkedList temp = L;
    while(temp->next)
    {
        temp = temp->next;
    }

    DLinkedList p = Dlist_creat_node(data);
    if(NULL == p)
    {
        perror("Dlist_creat_node err");
        return -1;
    }
    // 尾插法就就走到最后, 然后两个箭头
    temp->next = p;
    p->prior = temp;
    return 0;
}
//以序号插入
int DlistInsertIndex(DLinkedList L,int i,int data)
{
    int n = 0;
    DLinkedList temp = L;
    while(temp->next && n < i - 1)
    {
        temp = temp->next;
    }
    if(n < (i-1) || !temp)
    {
        return -2;//超出链表长度
    }
    DLinkedList p = Dlist_creat_node(data);
    if(NULL == p)
    {
        perror("Dlist_creat_node err");
        return -1;
    }
    p->next = temp->next;
    p->prior = temp->prior;
    temp->next->prior = p;
    temp->next = p;
    return 0;
}
//删除某一值得结点
// 删除就是找到这个节点后,改前后指向
int DlistDeleteValue(DLinkedList L,int data)
{
    int n = 0;
    DLinkedList temp = L;
    while(temp->next )
    {
        temp = temp->next;
        if(temp->data == data)
        {
            // 修改前后指针的指向
            temp->next->prior = temp->prior;
            temp->prior->next = temp->next;
            n++;
        }
    }
    return n;
}
//printList
int printDLinkList(DLinkedList L)
{
    DLinkedList temp = L;
    while(temp->next)
    {
        temp = temp->next;
        printf("%4d ",temp->data);
    }
    printf("\r\n");
    return 0;
}

int main()
{
    //双链表
     DLinkedList d_List;
     d_List = Dlist_creat_node(0);

    for(int i = 100;i < 110;i++)
        DlistInsertTail(d_List,i);  //创建一个链表

    printDLinkList(d_List);

    Dlist_insert_head(d_List,99);
    printDLinkList(d_List);
    printf("hello\n");
    return 0;
}
