/* 1.报文发送程序 */ //原始版本
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

void milliseconds_sleep(unsigned long mSec){
	struct timeval tv;
  	tv.tv_sec=mSec/1000;
    	tv.tv_usec=(mSec%1000)*1000;
    	int err;
    	do{
       	err=select(0,NULL,NULL,NULL,&tv);
    	}while(err<0 && errno==EINTR);
}

int main()
{
	int s, nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame[2] = {{0}};
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);  //创建套接字
	strncpy(ifr.ifr_name, "vcan0",IFNAMSIZ - 1 );
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ioctl(s, SIOCGIFINDEX, &ifr);     //指定can0设备
	//addr.can_family = AF_CAN;
	//addr.can_ifindex = ifr.ifr_ifindex;
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
	//bind(s, (struct sockaddr *)&addr, sizeof(addr));   //将套接字与can0绑定
	printf("end of bind \n");
	//禁用过滤规则，本进程不接收报文，只负责发送
  	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  	//生成两个报文
	frame[0].can_id = 0x11;
	frame[0]. can_dlc = 1;
	frame[0].data[0] = 0XAB;
	frame[1].can_id = 0x22;
	frame[1]. can_dlc = 1;
	frame[1].data[0] = 0XCD;
	//循环发送两个报文
	/*while(1) {
		nbytes = write(s, &frame[0], sizeof(frame[0]));  //发送frame[0]
		printf("send messages successed!");
		if (nbytes != sizeof(frame[0])) {
		    printf("Send Error frame[0]\n!");
		    break;        //发送错误，退出
		}
 		sleep(1);
		nbytes = write(s, &frame[1], sizeof(frame[1]));  //发送frame[1]
		if (nbytes != sizeof(frame[0])) {
    			printf("Send Error frame[1]\n!");
    			break;
		}
 		sleep(1);
	}*/
	
	while(1) {
		nbytes = write(s, &frame[0], sizeof(frame[0]));  //发送frame[0]
		printf("send messages successed!");
		if (nbytes != sizeof(frame[0])) {
		    printf("Send Error frame[0]\n!");
		    break;        //发送错误，退出
		}
 		milliseconds_sleep(500);
		nbytes = write(s, &frame[1], sizeof(frame[1]));  //发送frame[1]
		if (nbytes != sizeof(frame[0])) {
    			printf("Send Error frame[1]\n!");
    			break;
		}
		milliseconds_sleep(500);
	}
	
	close(s);
	return 0;
}


