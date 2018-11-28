#include <stdio.h>
#include <time.h>
#define maxSize 100
typedef struct 
{
	int data[maxSize];
	int length;
}Sqlist;

typedef struct LNode
{	
	int data;
	struct LNode * next;
}LNode;
////
typedef struct DLNode
{
	int data;
	struct DLNode * prior;
	struct DLNode * next;
}DLNode;

/**
 * [findElem description]
 * @作者     liulin
 * @时间     2018-11-06T13:06:31+0800
 * @描述     //返回第一个比大的元素
 * @param  L                        [description]
 * @param  x                        [description]
 * @return                          [description]
 */
int findElem(Sqlist L,int x)
{
	int i;
	for (i = 0; i < L.length; ++i)
	{
		if(x < L.data[i])
			return i;
		/* code */
	}
	
	return i;
}

/**
 * [insetElem description]
 * @作者    liulin
 * @时间    2018-11-06T14:24:16+0800
 * @描述    // 插入数据
 * @param L                        [description]
 * @param x                        [description]
 */
void insetElem(Sqlist L,int x)
{
	int p,i;
	p=findElem(L,x);
	for(i = L.length - 1; i >= p; i--)
	{
		// 最后一个到p 都要搬移
		L.data[i+1] = L.data[i];
	}
	// 赋值 
	L.data[p] = x; 
	// 长度加1
	L.length++;
}

//删除
int deleteElem(Sqlist L,int p,int e)
{
	if(p < 0 || p> L.length)
		return -1;
	e = L.data[p];;
	for (int i = p; i < L.length; ++i)
	{
		L.data[i] = L.data[i+1];
	}
	L.length--;
	return 0;
}


struct interval {
	int type;
	int base;
	int value;
};

struct options {
	time_t commit_interval;
	time_t refresh_interval;
	struct interval archive_interval;

	const char *protocol_db;
	const char *tempdir;
	const char *socket;

	struct {
		int compress;
		int prealloc;
		int limit;
		int generations;
		const char *directory;
	} db;
};

struct options opt = {
	.commit_interval = 86400,
	.refresh_interval = 30,

	.tempdir = "/tmp",
	.socket = "/var/run/nlbwmon.sock",
	.protocol_db = "/usr/share/nlbwmon/protocols",

	.db = {
		.directory = "/usr/share/nlbwmon/db"
	}
};

Sqlist list = {
	.length = 10,
	.data = {1,2,3,4,5},
};

int main(int argc, char const *argv[])
{

	// Sqlist list;
	// list.data[0] = 1;
	// list.data[1] = 3;
	// list.data[2] = 5;
	// list.data[3] = 8;
	// list.data[4] = 17;
	// list.length = 5;

	int result = findElem(list,0);
	insetElem(list,0);
	int a;
	deleteElem(list,0,a);
	for (int i = 0; i < list.length; ++i)
	{
		printf("hello %d\n",list.data[i]);
		/* code */
	}
	printf("%d\n", a);
    return 0;
}