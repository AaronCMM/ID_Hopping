/*  ID 的单次随机生成
*/
#include "algo_hmac.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>

#include<vector>
#include<set>
#include <fstream>
#include<map>
using namespace std;

int remain_id=0;               // 剩下的可用来生成随机id的位数
map<string,int> hash_map;


// 获取所有ID，并排序。根据优先级，分配PID。

void sort_pid()
	//int main(int argc, char * argv[])
{
	filebuf fb;
	string filename = "test.txt";

	if(fb.open(filename.c_str(),ios::in) == NULL) 
	{
		cout << "error" << endl;
	}

	istream is(&fb);
	string input;
	set<string> hash;  // 定义有序的set集合，可以将所有ID 都排序

	while(getline(is,input,'\n'))
	{
		char* p=(char*)input.c_str();
		const char *d = ",";
		char *token;
		string ret;
		int i=0;
		token = strtok(p,d);
		while(token!=NULL){
			i+=1;
			token=strtok(NULL,d);
			if(i==1){
				ret=token;
				if(hash.find(ret)==hash.end()){
					hash.insert(ret);
				}
				break;
			}
		}

	}

	fb.close();

	int size=hash.size();
	cout<<"total of ids: "<<hash.size()<<endl;
	int i=0;

	for(auto& id:hash){
		//ids[i++]=id;
		hash_map[id]=i++;
	}
	for(auto& id:hash_map){
		cout<<id.first<<'\t'<<id.second<<endl;
	}

	// 计算保存优先级需要的位数
	int pid=0;
	while(size!=0){
		size/=2;
		++pid;
	}
	cout<<"pid is "<<pid<<endl;

	remain_id=11-pid;      // 剩下的可用来生成随机id的位数
}

vector<int> get_pid(string oid){

	sort_pid();
	vector<int>m;
	if(hash_map.find(oid)!=hash_map.end()){
		int pid=hash_map[oid];

		for(int i =4; i>=0; i--)
		{
			m.push_back(((pid>>i)&1));//与1做位操作，前面位数均为0

		}

	}
	else{
		cout<<"the oid is not find!"<<endl;
	}
	return m ;
}

int main(int argc, char * argv[])
{
	if(argc < 2) {
		cout << "Please set the oid !" << endl;
		return -1;
	}

	char* sha2="sha256";
	char key[] = "4444";                //secret key

	//string data = "2";//要加密传输的数据
	string data=argv[1];

	unsigned char * mac = NULL;
	unsigned int mac_length = 0;

	int ret = HmacEncode(sha2, key, strlen(key), data.c_str(), data.length(), mac, mac_length);	

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

	vector<int> newid(remain_id);
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
				newid.push_back(a[j]);
				printf("%d",newid[j]);
			}
		}
	}

	printf("\n");

	vector<int>vid;
	vector<int>pid=get_pid(data);
	for(auto& p:pid){
		vid.push_back(p);
	}
	for(auto& p:newid){
		vid.push_back(p);
	}
	for(auto& p:vid){
		cout<<p;
	}
	cout<<endl;
	
	int id=0;
	for(int j=10;j>=0;j--){
		if(vid[j]!=0){
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
