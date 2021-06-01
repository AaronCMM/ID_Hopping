/* 1.报文发送程序 
对应的ECU 调用 ge_idpool 生成 备用id链表；
为了测试，ID_Pool_Size=3; 每 5s 更换一次ID，每 2s 发送一次报文
*/

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
static const int id_pool_size=5;
static const int period=20;      // seed 种子变更的周期 20s更换一次seed
int epoch=0;           //
int new_key,old_key;   //

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
int set_seed(string s,int epoch);
vector<int> get_idpool(string d,int size,bool isfirst,string key);   

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
		frame[0].data[0]=(__u8)cnt;
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
	
	int size=idpool.size();
	if( size!=0 && cnt<id_pool_size){
		frame[0].can_id=(canid_t)idpool[cnt];
		frame[0].data[0]=(__u8)(size-cnt);
		
		cout<<"the id_pool index now is:"<<cnt<<endl;
		cnt++;
		execute();
	}
	else{
		cout<<"id_pool is empty!"<<endl;
	}
	newid=frame[0].can_id;
}

void Callback_seed( void *obj, void *pa )
{
	++epoch;
	new_key= set_seed(std::to_string(old_key),epoch);
	cout<<"epoch: "<<epoch<<"  ge_seed:"<<new_key<<endl;
	old_key=new_key;
	idpool.clear();
	idpool=get_idpool("A0",id_pool_size,false,std::to_string(new_key));
	reverse(idpool.begin(),idpool.end()); 
	frame[0].can_id=0x0A0;
	oid=newid=frame[0].can_id;
	cnt=0;
    gettimeofday(&tv,NULL);
	strftime(timestamp, 128, "%X", localtime(&tv.tv_sec));
	int l = strlen(timestamp);
	sprintf(timestamp+l, ".%03ld", tv.tv_usec/1000);
	printf(" cansend change_seed: timestamp:%s\n",timestamp);
	//execute();
}

int main()
{
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);  //创建套接字
	strncpy(ifr.ifr_name, "vcan0",IFNAMSIZ - 1 );
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

    //生成ID池
	old_key=4444;
	new_key=old_key; 
	idpool=get_idpool("A0",id_pool_size,true,std::to_string(new_key));         
	
	// 防止被攻击者预测，推断出新ID，将倒序存储 ID链表
	reverse(idpool.begin(),idpool.end());

    CTimer ab;
	ab.addTimer(60,60,&Callback2,( void *)0x22,(void*)0xBD);    // 6s 更换ID
	ab.addTimer(10,10,&UserCallback,( void *)0x11,(void*)0x4E);   // 1s 验证ID是否更换，并发送消息
	ab.addTimer(200,200,&Callback_seed,( void *)0x11,(void*)0x4E);

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
