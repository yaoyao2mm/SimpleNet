/*
  LoRa Simple Yun Server :
  Support Devices: LG01. 
  
  Example sketch showing how to create a simple messageing server, 
  with the RH_RF95 class. RH_RF95 class does not provide for addressing or
  reliability, so you should only use RH_RF95 if you do not need the higher
  level messaging abilities.

  It is designed to work with the other example LoRa Simple Client

  User need to use the modified RadioHead library from:
  https://github.com/dragino/RadioHead

  modified 16 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited

  secondly modified by JohnSmith
  
  0708 18 NISL
  
*/
//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//If you use Dragino Yun Mesh Firmware , uncomment below lines. 
//#define BAUDRATE 250000

#define MAX_NODE_NUM 30  //最大节点数目
#define MAX_DATA_LEN 50  //最长数据

#include <Console.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <String.h>

// Singleton instance of the radio driver
RH_RF95 rf95;

int led = A2;
float frequency = 915.0;
int recv_pac_num = 0;
char char_recv_num[10];
int node_recv_list[MAX_NODE_NUM];//已经收到的不同节点的包的个数
int node_recv_jit[MAX_NODE_NUM];//正在收的包的节点数的个数

void setup() 
{
  pinMode(led, OUTPUT); 
  
  Bridge.begin(BAUDRATE);
  Console.begin();
  
  while (!Console) ; // Wait for console port to be available
  Console.println("Start Sketch");
  if (!rf95.init())
    Console.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  
  // Setup Spreading Factor (6 ~ 12)
  rf95.setSpreadingFactor(7);
  
  // Setup BandWidth, option: 7800,10400,15600,20800,31200,41700,62500,125000,250000,500000
  rf95.setSignalBandwidth(125000);
  
  // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8) 
  rf95.setCodingRate4(5);

  int i =0;
  while(i<MAX_NODE_NUM){
    node_recv_list[i]=0;
    i++;
    }
  
  Console.print("Listening on frequency: ");
  Console.println(frequency);
}

void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      digitalWrite(led, HIGH);
      RH_RF95::printBuffer("request: ", buf, len);

      Console.println("------------------------------------");
      Console.print("got packet: ");
      Console.println((char*)buf);
      Console.print("RSSI: ");
      Console.println(rf95.lastRssi(), DEC);

      String in_str = (char*)buf; 
      String temp_dev_num;//存下节点号
      String tmp_pac_rec;//根据统计网关收到的节点的包序号
      String tmp_pac_rec_inst;//目前收到的节点的包序号
      
      if(in_str.startsWith("$")&&in_str.endsWith("$")){//检查$$开头结尾
        Console.println("check complete!");
        String temp_in = in_str.substring(1,in_str.length()-1);//去掉头尾
        //Console.println(temp_in);
        if(temp_in.startsWith("NODE")){//检查是不是node发的
          //Console.println("from node");

          temp_in = temp_in.substring(5);//去掉node@
          //Console.println(temp_in);
          int aite = temp_in.indexOf("@");
          temp_dev_num = temp_in.substring(0,aite); //去掉@
          //Console.println(temp_dev_num);
          int dev_num = temp_dev_num.toInt();//
          node_recv_list[dev_num]+=1;//

          temp_in = temp_in.substring(aite+1);//去掉下一个@
          //Console.println(temp_in);
          
          aite = temp_in.indexOf("@");
          String data_str = temp_in.substring(0,aite); //去掉下一个@
          //Console.println(data_str);

          temp_in = temp_in.substring(aite+1);//
          //Console.println(temp_in);
          tmp_pac_rec_inst = temp_in;
          int rec_pac_sud = temp_in.toInt();//实时收到的包的PN
          if(rec_pac_sud == 1){
            node_recv_list[dev_num]=1;//这里做个激进的设定来方便实验，当开发板按下按钮时，重新启动，发送一个PN为1的包，然后这时认为实验重新开始，将累计包计数器记为1                    
            }
          node_recv_jit[dev_num] = rec_pac_sud;

          Console.print("GOT Packet from device "+temp_dev_num+" instantly:  PN: "+temp_in);

          char rec_pac_had[10];
          itoa(node_recv_list[dev_num],rec_pac_had,10);
          tmp_pac_rec = rec_pac_had;
          Console.println("   Inrecord,I had recived PN: "+tmp_pac_rec);
          
          }
          else{
            Console.println("doesn't from node!");
            }
        }
        else{
          Console.println("Collision !!!!!ERORR PACKET!!!!");
          }
        
      //
      // Send a reply
      String ACK = "$GW@"+temp_dev_num+"@1@instantlyPN:@"+tmp_pac_rec_inst+"$";
      uint8_t data[MAX_DATA_LEN];

      ACK.toCharArray(data,ACK.length()+1);
      
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Console.println("Sent a reply");
      digitalWrite(led, LOW);
    }
    else
    {
      Console.println("recv failed");
    }
  }
}




