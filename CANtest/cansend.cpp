/* 1.报文发送程序 */
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

static bool  run = true;

void milliseconds_sleep(unsigned long mSec){
	struct timeval tv;
	tv.tv_sec=mSec/1000;
	tv.tv_usec=(mSec%1000)*1000;
	int err;
	do{
		err=select(0,NULL,NULL,NULL,&tv);
	}while(err<0 && errno==EINTR);
}

/*void delay_ms(const __u32 ms)
{
	struct timespec tv;

	tv.tv_sec = ms / 1000;
	tv.tv_nsec = (long)(ms % 1000) * 1000000;

	sleep(&tv);
}*/

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


int main()
{
	int s, nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame[2] = {{0}};
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);  //创建套接字
	strncpy(ifr.ifr_name, "can0",IFNAMSIZ - 1 );
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ioctl(s, SIOCGIFINDEX, &ifr);     //指定can0设备

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
		perror("bind");
		return 1;
	}

	printf("end of bind \n");

	//禁用过滤规则，本进程不接收报文，只负责发送
	//setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);


	//生成报文
	frame[0].can_id = 0x11;
	frame[0]. can_dlc = 1;
	frame[0].data[0] = 0XAB;


	/*while(1) {
	  nbytes = write(s, &frame[0], sizeof(frame[0]));  //发送frame[0]
	  printf("send messages successed!");
	  if (nbytes != sizeof(frame[0])) {
	  printf("Send Error frame[0]\n!");
	  break;        //发送错误，退出
	  }
	  milliseconds_sleep(100);    // 每0.1s 发送一次消息
	  }*/


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

		/*while(((pid = wait(&status)) == -1) && (errno == EINTR))
		  {
		  delay_ms(10);
		  }*/
	}

	close(s);
	return 0;
}
