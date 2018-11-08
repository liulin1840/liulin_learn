#include<stdio.h>
#include<stdlib.h>
#define false 0
#define ok 1

// 结构体的定义,别名node ,后面定义了一个指针,指向这个结构体
typedef struct node {
    int data;
    struct node *next;
}node,*linklist;


// 用头插 创建链表
void creat_list(linklist l, int n)
{
    linklist p;
    l->next = NULL;
    int i;
    for (i = 0; i < n; i++)
    {
        p = (linklist)malloc(sizeof(node));
        // 初始化的时候,就是个指针赋值,等价于插入
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
    while (p && k<i )
    {
        p = p->next;
        k++;
    }
    if (!p || k>i)
        return false;

    *e = p->data;
    return ok;
}

/**
 * [listinsert description]
 * @作者     liulin
 * @时间     2018-11-07T15:28:04+0800
 * @描述     在链表的第几个位置上插入数据
 * @param  l                        传入头指针
 * @param  i                        位置
 * @param  e                        插入的值
 * @return                          [description]
 */
int listinsert(linklist l, int i, int e)
{
    int k = 1;
    linklist s = NULL, p = NULL;
    // 接管头指针,
    p = l;
    int j = 1;
    // 把指针移动到要插入位置的前面
    while (p && j<i)
    {
        p = p->next;
        j++;
    }
    if (!p || k> i)
        return false;

    // 指针操作, 申请一个新节点,数据域指针与赋值,连接上去即可
    s = (linklist)malloc(sizeof(node));
    s->data = e;
    s->next = p->next;
    p->next = s;
    return ok;
}

/**
 * [listdelete description]
 * @作者     liulin
 * @时间     2018-11-07T15:43:55+0800
 * @描述     插入删除都要先找位置,然后执行指针操作
 * @param  l                        [description]
 * @param  i                        [description]
 * @param  e                        [description]
 * @return                          [description]
 */
int listdelete(linklist l, int i, int *e)
{
    int j      = 1;
    linklist p = NULL, q = NULL;
    p = l;
    while (p && j<i)
    {
        p = p->next;
        j++;
    }
    if (!p->next || j>i)
        return false;
    // q 为p的后继,
    q = p->next;
    p->next = q->next;
    *e = q->data;

    // 删除的是p的后继,
    free(q);
    return ok;
}

/**
 * [see_list description]
 * @作者     liulin
 * @时间     2018-11-07T15:22:13+0800
 * @描述     显示链表的信息,内容与长度
 * @param  l                        [description]
 * @return                          [description]
 */
int see_list(linklist l)
{
    // 条件判断接管头结点,不断的在大脑中回顾图像
    // 定义链表的长度
    linklist p = NULL;
    int length = 0;

    if(l->next != NULL){
        p = l->next;
    }

    while (p){
        printf("%4d", p->data);
        p = p->next;
        length++;
    }

    if (length == 0){
        printf("链表为空");
        return false;
    }else {
        printf("  链表的长度 = %d\n",length );
    }
    printf("  \n");
    return ok;
}


int clearlist(linklist l)
{
    linklist p = NULL, q = NULL;
    p = l->next;
    while (p)
    {
        // 保存后继,删除前驱
        q = p->next;
        free(p);
        p = q;
    }
    l->next = NULL;
    return ok;
}
// l->next 指向第一个元素,l->next  为空
// 保存前趋, 进行头插,则链表逆置
void revers(linklist l){
    linklist q= NULL;
    linklist p= l->next;
    l->next   = NULL; 
    while(p){
        q = p;
        p = p->next;

        // 对他的前趋进行操作,如果不保存则会指针丢失,断链.
        q->next = l->next;
        l->next = q;    
    }
}
int main(void)
{
    int a;
    linklist lb = (linklist)malloc(sizeof(node));
    creat_list(lb, 5);
    see_list(lb);
    listinsert(lb, 2, 0);
    see_list(lb);
    //listdelete(lb, 4, &a);
    revers(lb);
    see_list(lb);
    clearlist(lb);
    return 0;
}
