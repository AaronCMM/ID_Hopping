/*2021.05.17
周期性地更换seed，epoch= cur_time/T
计算公式：New_seed=SHA_256(seed,epoch);
New_seed 是 INT 类型，4 字节，32 bit，所以取 SHA 低32bit的值作为 New_seed
*/

#include "algo_hmac.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>
#include<vector>

using namespace std;

int bit_int(vector<int>& data){
	int id=0;
	for(int j=10;j>=0;j--){
		if(data[j]!=0){
			id+=pow(2,(10-j));
		}
	}
	return id;
}  

int set_seed(string s,int epoch){
    char* sha2="sha256";
    const char* seed=s.c_str();      
    string data=std::to_string(epoch);

    unsigned char * mac = NULL;
	unsigned int mac_length = 0;
    int ret=0;
    ret = HmacEncode(sha2, seed, strlen(seed), data.c_str(), data.length(), mac, mac_length);
    if(0 == ret) {
		cout << "Algorithm HMAC encode succeeded!" << endl;
	}
	else {
		cout << "Algorithm HMAC encode failed!" << endl;
	}
    int c=0;
    vector<int> new_key(32);
	unsigned int temp=(unsigned int)mac[0];
	while(temp!=0 && c<32){
		new_key[c++]=temp%2;
		temp/=2;
	}
    return bit_int(new_key);
}
