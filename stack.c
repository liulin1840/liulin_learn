
#include<stdio.h>
#include<stdlib.h>
#define MAXSIZE 100
typedef struct {
    int data[MAXSIZE];
    int top;    //栈顶
}SeqStack;

/*初始化*/
SeqStack *initStack(){
    SeqStack *s;
    s = (SeqStack *)malloc(sizeof(SeqStack));  //动态申请内存
    s->top = -1;   //空栈时，栈顶指针top=-1

    return s;
}

/*进栈*/
void push( SeqStack *s, int x ){
    if( s->top >= MAXSIZE-1 ){  //判断栈满 
        printf("栈满!"); 
    }else{
        s->top++;   
        s->data[s->top] = x;        
    }
} 

/*取栈顶元素*/
int topStack( SeqStack *s ){
    int x;

    if( isEmpty(s) ){  //判断栈是否为空
        printf("栈空");
    }else{
        return x = s->data[s->top]; 
    }   
    return 0;
} 

/*判断栈是否为空*/ 
int isEmpty( SeqStack *s ){
    if( s->top == -1 ){
        return 0;  //栈空 
    }else{
        return -1;
    }   
    return 0;
}

/*出栈*/
void pop( SeqStack *s, int *x ){   
    if( isEmpty(s) ){  //判断栈空 
        printf("栈空");
    }else{
        *x = s->data[s->top];  //将栈顶元素存入*x中 
        s->top--;   
    } 
}
int main(int argc, char const *argv[])
{
	/* code */
	return 0;
}
