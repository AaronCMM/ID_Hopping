/* 2. 报文过滤接收程序 */
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

int main()
{
	char timestamp[128] = {0};
	int s, nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame;
	struct can_filter rfilter[1];
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

	bind(s, (struct sockaddr *)&addr, sizeof(addr));   //将套接字与can0绑定
	printf("end of bind \n");

	//定义接收规则，只接收表示符等于0x11的报文
	rfilter[0].can_id   = 0x11;
	rfilter[0].can_mask = CAN_SFF_MASK;

	//设置过滤规则
	/*setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
	  while(1) {
	  nbytes = read(s, &frame, sizeof(frame));   //接收报文
	//显示报文
	if (nbytes > 0) {
	printf("ID=0x%x DLC=%d data[0]=0x%x\n", frame.can_id,
	frame.can_dlc, frame.data[0]);
	}
	}*/
	int i=0;

	struct timeval time[2];
	while(i<1000){
		/* 时间字符串 */
		int l = 0;
		char timestamp[128];
		struct timeval tv;
		struct can_frame frame; 
		nbytes = read(s, &frame, sizeof(frame));   //接收报文
		gettimeofday(&tv, NULL);
		strftime(timestamp, 128, "%X", localtime(&tv.tv_sec));
		l = strlen(timestamp);
		sprintf(timestamp+l, ".%03ld", tv.tv_usec/1000);

		if (nbytes > 0) {
			printf("timestamp=%s ID=0x%x DLC=%d data[0]=0x%x\n",timestamp, frame.can_id,
					frame.can_dlc, frame.data[0]);

			if(i==0)  time[0]=tv;
			else{

				int diff=(tv.tv_sec-time[0].tv_sec)*1000+(tv.tv_usec-time[0].tv_usec)/1000;
				printf("diff:%d \n",diff);
				if(diff<90){
				char *f="anomaly detected!";
					printf("anomaly detected!\n");
					nbytes = write(s, &f, sizeof(f));
					break;
				}
				else{
					time[0]=tv;
				}
			}
			++i;
		}

	}
	close(s);
	return 0;
}
