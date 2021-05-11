#include "algo_hmac.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>
using namespace std;
 
int main(int argc, char * argv[])
{
	if(argc < 2) {
		//参数指定hash算法,支持HmacEncode列举的那些
		cout << "Please specify a hash algorithm!" << endl;
		return -1;
	}
 
	char key[] = "4444";//secret key
	//string data = "2";//要加密传输的数据
	string data=argv[2];
 
	unsigned char * mac = NULL;
	unsigned int mac_length = 0;
	
	int ret = HmacEncode(argv[1], key, strlen(key), data.c_str(), data.length(), mac, mac_length);	
	
	if(0 == ret) {
		cout << "Algorithm HMAC encode succeeded!" << endl;
	}
	else {
		cout << "Algorithm HMAC encode failed!" << endl;
		return -1;
	}
 
	cout << "mac length: " << mac_length << endl;
	cout << "mac:";
	
	for(int i = 0; i < mac_length; i++) {
		printf("%-03x", (unsigned int)mac[i]);

	}

	cout << endl;
	
	int newid[6];
	int a[256];
	int t=0;
	for(int i=0;i<mac_length;i++){
	
		int b[8];
		int c=0;
		unsigned int temp=(unsigned int)mac[i];
		while(temp!=0){
			b[c++]=temp%2;
			temp/=2;
		}

		t+=c;
		for(int j=c-1;j>=0;j--){
			a[t-j-1]=b[j];
		}
		
		if(t>6 && t<15){
			for(int j=0;j<6;j++){
				newid[j]=a[j];
				printf("%d",newid[j]);
			}
		}
	}
	
	printf("\n");

	for(int j=0;j<t;j++){
		printf("%d",a[j]);
	}

	printf("\n");

	int id=0;
	for(int j=10;j>=0;j--){
		if(newid[j]!=0){
			id+=pow(2,(10-j));
		}
	}
	printf("%d\n",id);

	if(mac) {
		free(mac);
		cout << "mac is freed!" << endl;
	}
 
	return 0;
}
