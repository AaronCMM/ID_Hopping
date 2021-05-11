// 生成多个随机ID，并保证优先级

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
map<int,int> hash_map;


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
	set<int> hash;  // 定义有序的set集合，可以将所有ID 都排序

	while(getline(is,input,'\n'))
	{
		char* p=(char*)input.c_str();
		const char *d = ",";
		char *token;
		string ret;
		int i=0;
		token = strtok(p,d);
		/*while(token!=NULL){
			i+=1;
			token=strtok(NULL,d);
			if(i==1){
				ret=token;
				if(hash.find(ret)==hash.end()){
					cout<<ret<<endl;
					hash.insert(ret);
				}
				break;
			}
		}*/
		
		token=strtok(NULL,d);
		string str =token;
		string cstr=str.substr(0,5);
		int t=stoi(cstr,nullptr,16);
		if(hash.find(t)==hash.end()){
			hash.insert(t);
		}

	}

	fb.close();

	int size=hash.size();
	cout<<"total of ids: "<<hash.size()<<endl;
	int i=0;

	for(auto& id:hash){
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

	remain_id=11-pid;      // 剩下的可用来生成随机id的位数*/
}

vector<int> get_pid(string o){

	vector<int>m;
	int pid;
	string oid=o;
	transform(oid.begin(), oid.end(), oid.begin(), ::toupper);  
	int d=stoi(oid,nullptr,16);

	if(hash_map.find(d)!=hash_map.end()){
		pid=hash_map[d];

		/*for(int i =4; i>=0; i--)
		{
			m.push_back(((pid>>i)&1));//与1做位操作，前面位数均为0
		}*/

	}
	else{
		//cout<<"the oid is not find!"<<endl;
		pid=d;
	}

	for(int i =4; i>=0; i--)
	{
		m.push_back(((pid>>i)&1));//与1做位操作，前面位数均为0

	}

	return m ;
}

int bit_int(vector<int>& vid){
	int id=0;
	for(int j=10;j>=0;j--){
		if(vid[j]!=0){
			id+=pow(2,(10-j));
		}
	}
	printf("\t %d",id);
	return id;
}

// 传入 两个参数 原始ID，生成的ID 个数 
/*int main(int argc, char * argv[])
{
	if(argc < 3) {
		cout << "Please set the oid and pool_size number !" << endl;
		return -1;
	}

	char* sha2="sha256";
	char key[] = "4444";                //secret key

	string data=argv[1];                 //
	int pool_size=atoi(argv[2]);
	printf("pool_size is:%d\n",pool_size);

	unsigned char * mac = NULL;
	unsigned int mac_length = 0;


	vector<int>vid;
	vector<int>newid;
	vector<int>pid=get_pid(data);

	int a[6];

	while(pool_size!=0){

		int ret = HmacEncode(sha2, key, strlen(key), data.c_str(), data.length(), mac, mac_length);	

		if(0 == ret) {
			cout << "Algorithm HMAC encode succeeded!" << endl;
		}
		else {
			cout << "Algorithm HMAC encode failed!" << endl;
			return -1;
		}

		cout << "mac:";
		for(int i = 0; i < mac_length; i++) {
			printf("%-03x", (unsigned int)mac[i]);

		}
		cout << endl;
		int c=0;
		int b[6];
		unsigned int temp=(unsigned int)mac[0];
		while(temp!=0 && c<6){
			b[c++]=temp%2;
			temp/=2;
		}

		for(int j=c-1;j>=0;j--){
			a[6-j-1]=b[j];
		}

		for(auto& p:pid){
			vid.push_back(p);
			cout<<p;
		}
		cout<<endl;

		for(int j=0;j<6;j++){
			vid.push_back(a[j]);
			printf("%d",a[j]);
		}
	
		int v=bit_int(vid);
		printf("-----------\n");
		//printf("%d\n",v);
		newid.push_back(v);        // newid 作为 idpool 存储所有生成的新的随机ID
		data=to_string(v);        // 将新生成的newid 作为 HMAC的参数再次传入  f(id)=f(id,k);
		vid.clear();
		pool_size-=1;
	}

	printf("\n");

	if(mac) {
		free(mac);
		cout << "mac is freed!" << endl;
	}

	return 0;
}*/


// 传入 两个参数 原始ID，生成的ID 个数(版本2) 
vector<int> get_idpool(string d,int size)
{
	/*if(argc < 3) {
		cout << "Please set the oid and pool_size number !" << endl;
		return -1;
	}*/
	sort_pid();

	char* sha2="sha256";
	char key[] = "4444";                //secret key

	/*string data=argv[1];                
	int pool_size=atoi(argv[2]);*/
	string data=d;
	int pool_size=size;
	printf("pool_size is:%d\n",pool_size);

	unsigned char * mac = NULL;
	unsigned int mac_length = 0;


	vector<int>vid;
	vector<int>newid;
	vector<int>pid=get_pid(data);

	int a[6];

	while(pool_size!=0){

		int ret = HmacEncode(sha2, key, strlen(key), data.c_str(), data.length(), mac, mac_length);	

		if(0 == ret) {
			cout << "Algorithm HMAC encode succeeded!" << endl;
		}
		else {
			cout << "Algorithm HMAC encode failed!" << endl;
			//return -1;
		}

		/*cout << "mac:";
		for(int i = 0; i < mac_length; i++) {
			printf("%-03x", (unsigned int)mac[i]);

		}
		cout << endl;*/
		int c=0;
		int b[6];
		unsigned int temp=(unsigned int)mac[0];
		while(temp!=0 && c<6){
			b[c++]=temp%2;
			temp/=2;
		}

		for(int j=c-1;j>=0;j--){
			a[6-j-1]=b[j];
		}

		cout<<"the pid is \t";
		for(auto& p:pid){
			vid.push_back(p);
			cout<<p;
		}
		cout<<endl;

		cout<<"the new id is \t";
		for(int j=0;j<6;j++){
			vid.push_back(a[j]);
			printf("%d",a[j]);
		}
	
		int v=bit_int(vid);
		printf("\n-----------");
		 
		newid.push_back(v);        // newid 作为 idpool 存储所有生成的新的随机ID
		data=to_string(v);        // 将新生成的newid 作为 HMAC的参数再次传入  f(id)=f(id,k);
		vid.clear();
		pool_size-=1;
	}

	printf("\n");

	if(mac) {
		free(mac);
		//cout << "mac is freed!" << endl;
	}

	return newid;
}

/*int main(){
	vector<int>i=get_idpool("0x0A0",1);
	int temp=i[0];
	cout<<temp;
	i=get_idpool(to_string(temp),1);
	cout<<i[0];
	
	//sort_pid();
}*/
