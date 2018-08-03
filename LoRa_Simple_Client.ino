/*
  LoRa Simple Client for Arduino :
  Support Devices: LoRa Shield + Arduino 
  
  Example sketch showing how to create a simple messageing client, 
  with the RH_RF95 class. RH_RF95 class does not provide for addressing or
  reliability, so you should only use RH_RF95 if you do not need the higher
  level messaging abilities.

  It is designed to work with the other example LoRa Simple Server
  User need to use the modified RadioHead library from:
  https://github.com/dragino/RadioHead

  modified 16 11 2016
  by Edwin Chen <support@dragino.com>

  secondly modified 2018 0708 
  by JohnSmith
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <String.h>

// Singleton instance of the radio driver
RH_RF95 rf95;
float frequency = 915.0;
//JS 18 0708 加入设备号
String device_num = "1";

void setup() 
{
  Serial.begin(9600);
  //while (!Serial) ; // Wait for serial port to be available
  Serial.println("Test by JohnSmith_NISL");
  Serial.println("Start LoRa Client");
  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);

  // Setup Spreading Factor (6 ~ 12)
  rf95.setSpreadingFactor(7);
  
  // Setup BandWidth, option: 7800,10400,15600,20800,31200,41700,62500,125000,250000,500000
  //Lower BandWidth for longer distance.
  rf95.setSignalBandwidth(125000);
  
  // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8) 
  rf95.setCodingRate4(5);
}

void loop()
{
  Serial.println("Sending to LoRa Server LG01");
  // Send a message to LoRa Server

  //JS 18 0708 试图将设备号发送给服务器:
  uint8_t data[40];
  String mess = "Hello,nisl.this is device ";
  mess.concat(device_num);
  
  int dev_num_length = device_num.length();
  
//  Serial.println("This is the String:");
//  Serial.println(mess);
//  Serial.println("This is the char[]");

  mess.toCharArray(data,mess.length()+dev_num_length);
  
  Serial.println((char*)data);
  
  Serial.print("\n");

  
  rf95.send(data, sizeof(data));
  
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf95.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now
    // JS 20180708：但是这里的判断仅仅是停留在检测recv有没有收到lora信号而已，我如果用多个设备发，那么他会认做是么？   
    // 答案：他只是简单的识别了下lora信号而已
    if (rf95.recv(buf, &len))
   {
      Serial.print("got reply: ");
      //This is the method to print char*
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is LoRa server running?");
  }
  delay(5000);
}


