#include"CTimer.h"
#include<stdio.h>
#include<string.h>
using namespace std;

struct timeval tv;
char timestamp[128];

void  UserCallback( void *obj, void *pa )
{
        gettimeofday(&tv,NULL);
	strftime(timestamp, 128, "%X", localtime(&tv.tv_sec));
	int l = strlen(timestamp);
	sprintf(timestamp+l, ".%03ld", tv.tv_usec/1000);
	
	printf(" UserCallback:: ( obj = 0x%x ) T_ID=%x timestamp:%s \n", obj,pa,timestamp);
}

void  Callback2( void *obj, void *pa )
{
        gettimeofday(&tv,NULL);
	strftime(timestamp, 128, "%X", localtime(&tv.tv_sec));
	int l = strlen(timestamp);
	sprintf(timestamp+l, ".%03ld", tv.tv_usec/1000);

	printf(" UserCallback:: ( obj = 0x%x ) T_ID=%x  timestamp:%s\n",obj, pa,timestamp);
}

int main() //测试函数，定义三个定时器
{
	CTimer ab;
	ab.addTimer(10,10,&Callback2,( void *)0x22,(void*)0xBD);
	ab.addTimer(1,1,&UserCallback,( void *)0x11,(void*)0x4E);

	ab.start();
	getchar();
	ab.stop();
	getchar();

	/*CTimer cd;
	  cd.addTimer(3,3,&UserCallback,( void *)0x33,0);
	  cd.start();
	  getchar();
	  cd.stop();
	  getchar();*/

	exit(0);
}
