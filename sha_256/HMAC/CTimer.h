#ifndef CTIMER_H
#define CTIMER_H
#include<pthread.h>
#include <unistd.h>
#include <signal.h>
#include<pthread.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <mutex>
using namespace std;
#define N 100 //设置最大的定时器个数

typedef void (* Callback )( void *n_obj, void *user_data );//callback
struct Timer //Timer结构体，用来保存一个定时器的信息
{
	void                        *user_data;//回调函数参数
	void                        *user_obj;//回调函数参数
	int microsecond_time; //每隔total_time秒
	int second_time; //还剩left_time秒
	int cont_time;//记录时间
	int func; //该定时器超时，要执行的代码的标志
	Callback  n_cb;//回调函数指针
}; //定义Timer类型的数组，用来保存所有的定时器

class CTimer
{
	public:
		CTimer();
		virtual~CTimer();
		void  addTimer(int ,int,Callback,void *,void*); //新建一个计时器
		static void   timeout(); //判断定时器是否超时，以及超时时所要执行的动作
		void start();
		void stop();

	private:
};

#endif // CTIMER_H
