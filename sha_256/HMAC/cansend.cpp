/* 1.报文发送程序 */
// 对应的ECU 调用 ge_idpool 生成 备用id链表,并在设置的时间间隔内更换id

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <errno.h>
#include <sys/time.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/prctl.h>
#include<vector>
#include<string>
#include"algo_hmac.h"
#include<iostream>
#include<bits/stdc++.h>
#include"CTimer.h"
#include<signal.h>

using namespace std;
static bool  run = true;
	int s, nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame[2] = {{0}};
	vector<int> idpool;
        canid_t oid,newid;

/*void milliseconds_sleep(unsigned long mSec){
	struct timeval tv;
	tv.tv_sec=mSec/1000;
	tv.tv_usec=(mSec%1000)*1000;
	int err;
	do{
		err=select(0,NULL,NULL,NULL,&tv);
	}while(err<0 && errno==EINTR);
}*/
void milliseconds_sleep(unsigned long mSec);

void sig_handler(int signo) 
{ 
#ifndef WIN32
	switch(signo)
	{
		case SIGINT:
			run = false;
			printf(".Exit\n");
			break;

		default: break;
	}
#endif
}

void install_sig(void) 
{ 
#ifndef WIN32
	signal(SIGINT, sig_handler);
#endif
}

bool flag=true;

void execute(){
	
	nbytes = write(s, &frame[0], sizeof(frame[0]));  //发送frame[0]
	printf("send messages successed!  can_id=%x \n",frame[0].can_id);
	if (nbytes != sizeof(frame[0])) {
		printf("Send Error frame[0]\n!");
	}
	
}

struct timeval tv;
char timestamp[128];
int cnt=0;

void  UserCallback( void *obj, void *pa )
{
        gettimeofday(&tv,NULL);
	strftime(timestamp, 128, "%X", localtime(&tv.tv_sec));
	int l = strlen(timestamp);
	sprintf(timestamp+l, ".%03ld", tv.tv_usec/1000);
	
	printf(" timestamp:%s\n",timestamp);

	//printf(" UserCallback:: ( obj = 0x%x ) T_ID=%x timestamp:%s \n", obj,pa,timestamp);
	
	if(oid != newid){
		oid=newid;
		frame[0].data[0]=(__u8)cnt;
	}
	else{
		frame[0].data[0]=(__u8)0xAB;
		execute();
	}
	oid=newid;
}

void  Callback2( void *obj, void *pa )
{
        gettimeofday(&tv,NULL);
	strftime(timestamp, 128, "%X", localtime(&tv.tv_sec));
	int l = strlen(timestamp);
	sprintf(timestamp+l, ".%03ld", tv.tv_usec/1000);

	printf(" timestamp:%s\n",timestamp);
	
	int size=idpool.size();
	if( size!=0 && cnt<3){
		frame[0].can_id=(canid_t)idpool[cnt];
		frame[0].data[0]=(__u8)(size-cnt);
		execute();
		cnt++;
		cout<<cnt<<endl;
	}
	else{
		cout<<"none"<<endl;
	}
	newid=frame[0].can_id;
}


int main()
{
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);  //创建套接字
	strncpy(ifr.ifr_name, "can0",IFNAMSIZ - 1 );
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ioctl(s, SIOCGIFINDEX, &ifr);     //指定vcan0设备

	ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
	if (!ifr.ifr_ifindex) {
		perror("if_nametoindex");
		return 1;
	}
	printf("begin bind");

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	printf("....");
	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("error bind");
		return 1;
	}

	printf("end of bind \n");

	//禁用过滤规则，本进程不接收报文，只负责发送
	//setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);


	//生成报文
	frame[0].can_id = 0x0A0;
	frame[0]. can_dlc = 1;
	frame[0].data[0] = 0XAB;
	oid=0x0A0;
	newid=0x0A0;

        //调用函数，生成ID备用链表

	vector<int> get_idpool(string d,int size);    //声明
	idpool=get_idpool("A0",3);
	/*for(auto& i:idpool){
		printf("%d ",i);
	}*/
	
	// 防止被攻击者预测，推断出新ID，将倒序存储 ID链表
	reverse(idpool.begin(),idpool.end());

	//cout<<idpool[0]<<endl;

        CTimer ab;
	ab.addTimer(50,50,&Callback2,( void *)0x22,(void*)0xBD);    // 5s
	ab.addTimer(20,20,&UserCallback,( void *)0x11,(void*)0x4E);   // 2s

	ab.start();
	getchar();
	ab.stop();
	exit(0);

	/*while(1) {
	  
	  gettimeofday(&tv2,NULL);
	  if(cnt==1) tv1=tv2;
	  else{
	  	int diff=(tv2.tv_sec-tv1.tv_sec)*1000+(tv2.tv_usec-tv1.tv_usec)/1000;   // ms
		printf("diff: %d \t",diff);
		if(diff<110){
			nbytes = write(s, &frame[0], sizeof(frame[0]));  //发送frame[0]
	  		printf("send messages successed!");
	  		if (nbytes != sizeof(frame[0])) {
	  			printf("Send Error frame[0]\n!");
	  			break;    
	  		}
		}
	  }

	  nbytes = write(s, &frame[0], sizeof(frame[0]));  //发送frame[0]
	  printf("send messages successed!");
	  if (nbytes != sizeof(frame[0])) {
	  	printf("Send Error frame[0]\n!");
	  	break;        //发送错误，退出
	  }
	  milliseconds_sleep(100);    // 每0.1s 发送一次消息

	}*/



       /*  
	run = true;
	pid_t pid = -1;
	int   status;
	pid = fork();
	int send_frame_times=100;
	char* temp;
	if(pid == -1) 
	{ 
		printf("\n\t创建进程失败\n\n");
		return  -1;
	}
	else if(pid == 0) // 子进程，用于发送CAN帧
	{
		prctl(PR_SET_PDEATHSIG,SIGKILL);
		printf("child process.\n");
		printf("child  pid:%d,parent pid:%d\n",getpid(),getppid());

		while (run && (send_frame_times > 0))
		{
			nbytes = write(s, &frame[0], sizeof(frame[0]));
			printf("send!\n");
			send_frame_times--;
			milliseconds_sleep(100);
		}

		exit(0);
	}
	else // 父进程，接收CAN帧
	{
		install_sig();

		while (run)
		{

			nbytes = read(s, &temp, sizeof(temp));
			if (nbytes > 0)
			{
				printf("the receive content is: %x \n",&temp);
				break;
			}
		}
	}

	close(s);
	return 0;
	*/

//	return 0;
}
