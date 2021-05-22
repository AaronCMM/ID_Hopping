//2021.1.13

pre 文件中的 rec.c  cansend.c 是最原始的版本 1.0

CANtest /canrec2.cpp  canrec2    // 用于接收 和 发送 CAN消息（修改后的版本）

CANtest/cansend.cpp cansend2    (对应于 c 文件夹中的 canrec.c 和 send.c)

————更新于  2021.5.9

https://www.cnblogs.com/eaggle/p/7641406.html

https://blog.csdn.net/weixin_30555515/article/details/95918725

### 创建一个vcan网络接口：

```
 $ ip link add type vcan 
```

### 显示CAN设备的详情和统计信息：

```
ip -details -statistics link show vcan0
```

###  启动CAN网络设备

```
sudo ip link set up vcan0
```

### 安装can-utils

```
sudo apt-get install can-utils  
```

### HMAC文件中的代码们：

CTimer.h   ctimer.cpp // 定时器函数

generate_id.cpp   ----->  ge_id

main.cpp  // 测试 HMAC函数

ge_seed.cpp       // 周期性更换seed

### 使用NTP同步时间钟

教程：https://vitux.com/how-to-install-ntp-server-and-client-on-ubuntu/

http://linux.vbird.org/linux_server/0440ntp.php

![image-20210517164742839](https://i.loli.net/2021/05/17/sMJiGlWzgar7FkY.png)

<img src="https://i.loli.net/2021/05/17/9gvdznV1Jsu3Lyc.png" alt="image-20210517164753617" style="zoom:70%;" />

<img src="https://i.loli.net/2021/05/17/oIpv9gZcOQVRbxs.png" alt="image-20210517164810907" style="zoom:70%;" />



### 系统设计

时间同步机制，同步更新id。

逻辑：

1. 设置两个定时任务
   - 每隔 0.1 s 发送消息（周期性地发送消息）
   - 每隔 1 s 更换 id9（周期性地更换 id，对时间要求不严格）
   - key 需要周期性地变化（刚开始的时候，所有ECU共享一个seed，为了多变性，一段时间间隔后，所有ECU需要同步更换 seed）（对时间要求严格，如何保证全部同步？）<font color=blue>当前时间 / 变更周期 = epoch 次数，NSeed=Hash(seed，epoch)；</font>
2. 当==更换 id 时==，发送端要发送消息给接收端。           ==发送端广播消息，id=newid，消息为 the hash index (k-1)==
3. 接收端收到 更换id的消息，更换当前的id 列表        ==接收端，计算 newid 经过 多次hash后，是否为 nodeid ( 即 源id )==，若是，则接收该消息，并更新 table。
4. 每个ECU都维护一张 table

- [ ] - [x] | NodeID | HashIndex |     CurrentID     |
    | :----: | :-------: | :---------------: |
    |   i    |     k     | ID<sub>i</sub>(k) |

先对整个CAN 总线上的ID进行统计，并排序。按照优先级进行排序，总的ID数为28，则最大需要保留 5 bit 空间给 PID。

![image-20210509124733494](https://i.loli.net/2021/05/09/6q9HmrLSW2pYCcV.png)

其余的 6bit 字节 用来生成随机ID。（图为 id="0x0A0" ，调用 HMAC，生成 3个 随机ID ）。方便测试，先设置 IDpool_size=3

![image-20210509124758686](https://i.loli.net/2021/05/09/1SvpTqBE5dXUmeW.png)

发送端    成功发送消息：

![image-20210509124748487](https://i.loli.net/2021/05/09/Sb7PnL9upUCqx1K.png)

接收端 当 收到的 id !=node_id 时，检查 node_id 经过多次HAMC之后 是否为 id。调用的次数=  接收的数据包的 newindex -  当前table 维护的 index。

如是，则接收并更新表格。否则丢弃。

![image-20210509124820268](https://i.loli.net/2021/05/09/SsAqkyYMrOcmnUP.png)

![image-20210509124831259](https://i.loli.net/2021/05/09/HcWOs4o6qFmfa57.png)

![image-20210509124838468](https://i.loli.net/2021/05/09/RcTuYhBSxnNUetj.png)



在树莓派的运行结果

![image-20210509145432614](https://i.loli.net/2021/05/09/KyHfbCtMpBOLTnE.png)



使用基于线性反馈移位寄存器（LFSR）的伪随机发生器（PRG）进行随机化，并使用基于哈希的PRG进行同步链；

![image-20210516205705125](https://i.loli.net/2021/05/17/c8CyeYPi5nKRvuB.png)

<img src="https://i.loli.net/2021/05/17/9pGWfuZvVdF2QLs.png" alt="image-20210516210546445" style="zoom:50%;" />

- sha_256，生成一次密钥的时间

  摘自别人的论文：

<img src="https://i.loli.net/2021/05/17/6DzaJktVUTFl79o.png" alt="image-20210516212920043" style="zoom:67%;" />

- 

0A0 —> 160

key：4444，new_id：108,127,126,116,101（对应的hex：6c，7f，7e，74，65）

key：1400，new_id：109，108,67,66,123（对应的hex：7b，42，43，6c，6d）

key：808，new_id：109,113,121,75,123

