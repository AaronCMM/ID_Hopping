#include "CTimer.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <iostream>
std::list<void*>m_node_list;
int is=0,tb=1; //i代表定时器的个数；t表示时间，逐秒递增
bool statues=true;//状态信息
std::mutex         m_locker;//锁
void CTimer::addTimer(int m_time,int sec_time,Callback p,void* parm1,void *parm2 ) //新建一个计时器
{
	struct Timer *a=new Timer;
	a->microsecond_time=m_time;
	a->second_time=sec_time;
	a->cont_time=sec_time;
	a->n_cb=p;//huidiao
	a->user_obj=parm1;
	a->user_data=parm2;
	m_node_list.push_back( ( void *)( a ));
}

CTimer::CTimer(){
}
CTimer::~CTimer(){}
void CTimer:: timeout()//判断定时器是否超时，以及超时时所要执行的动作
{
	std::list<void*>::iterator  it;Timer *n;
	for ( it = m_node_list.begin( ); it != m_node_list.end( ); it ++ )
	{
		if ( *it != 0 )
		{
			n = ( Timer *)( *it );
			n->cont_time--;
			if(n->cont_time==0)
			{
				n->cont_time=n->second_time;//重新设置时间
				(*n->n_cb )(n->user_data, n->user_obj );//执行回调
			}
		}
	}
}

void CTimer::stop(){
	m_locker.lock();
	statues=false;
	m_locker.unlock();
}

void milliseconds_sleep(unsigned long mSec){
	struct timeval tv;
	tv.tv_sec=mSec/1000;
	tv.tv_usec=(mSec%1000)*1000;
	int err;
	do{
		err=select(0,NULL,NULL,NULL,&tv);
	}while(err<0 && errno==EINTR);
}

void* singlesa(void*p){
	signal(SIGUSR1,(__sighandler_t)static_cast<CTimer*>(p)->timeout); //接到SIGALRM信号，则执行timeout函数
	while(1)
	{
		//sleep(1); //每隔一秒发送一个SIGALRM
		milliseconds_sleep(100);  // 0.1s
		kill(getpid(),SIGUSR1);//SIGUSR1是用户自定义信号。利用kill来发送信号
		if(!statues){break;}
	}
}
void CTimer::start(){
	pthread_t thread_timer;//创建一个线程去发送信号
	pthread_create(&thread_timer, NULL, singlesa,static_cast<void*>(this));
}
