#include"UnifiedAtWifi.h"
// #include"UnifiedAtTcpIp.h"
#define Esp  Serial1



void setup() {
  // while(1);
//  while(SD.begin()){
//      ;
//  }
//  auto file = SD.open("1.txt", FILE_WRITE);
//  String content = "Hello World";
//  file.write(content, content.size() + 1);
//  file.close();
//  file = SD.open("1.txt", FILE_READ);
//  char buf[20];
//  file.read(buf, sizeof(buf));
  //while(1);
  // put your setup code here, to run once:
  //  RfPower power;
  //  atDeepSleep(1000);
  //  int i = 0;
  //  auto func = [&](){
  //      if (atRfPower(& power) == success){
  //          Serial.printf("%d %d %d %d", 
  //            power.wifiPower, 
  //            power.bleAdvPower,
  //            power.bleScanPower,
  //            power.bleConnPower
  //          );
  //          i |= 2;
  //      } 
  //      else{
  //          i |= 1;
  //      }
  //  };
  //  func();
  //  delay(1000);
  //  func();
  // bool b = atBegin();
  
  Serial.begin(115200);
  while(!Serial);
  Esp.begin(115200);
  //atBegin();
  //atWifiConnect("POT-AL00a","12345678wc");
  
//  Serial.println(buf);
  //atMdns("miao", "mixc", 8080);
  
//  FirmwareInfo info;
//  atFirmwareInfo(& info);
//  atWifiUserList([&](WifiUserList & info){
//      debug("%d.%d.%d.%d - %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
//        info.ip[0],
//        info.ip[1],
//        info.ip[2],
//        info.ip[3],
//        info.mac[0],
//        info.mac[1],
//        info.mac[2],
//        info.mac[3],
//        info.mac[4],
//        info.mac[5]
//      );
//  });
//  
//  Serial.println(info.atVersion.c_str());
//  Serial.println(info.sdkVersion.c_str());
//  Serial.println(info.compileTime.c_str());
//  Serial.println(info.binVersion.c_str());
//  WifiApConfigure c;
//  atWifiApConfigure(&c);
//  debug(
//      "%s\n%s\n%d\n%d\n%d\n%d\n",
//      c.ssid.c_str(),
//      c.pwd.c_str(),
//      c.channel,
//      c.ecn,
//      c.maxConnect,
//      c.isSsidHidden
//  );
  //while(1);
  //debug("%d\n", b);
//  atWifiScan([&](WifiLinkInfo & info){
//      Serial.printf(
//          "%d,%s,%d,%x:%x:%x:%x:%x:%x,%d\n", 
//          info.ecn, 
//          info.ssid.c_str(), 
//          info.rssi,
//          info.mac[0],
//          info.mac[1],
//          info.mac[2],
//          info.mac[3],
//          info.mac[4],
//          info.mac[5],
//          info.channel
//      );
//  });
  //pinMode(0, OUTPUT);
    Esp.println("ATE1");
}

String ipd = "+IPD";

void loop() {
//  digitalWrite(0, HIGH);
//  digitalWrite(0, LOW);
  while (Esp.available() > 0){
      char c = Esp.read();
      Serial.print(c);
      if (c != '+'){
          continue;
      }

      
      c = Esp.read();
      Serial.print(c);
      
      if (c != 'I'){
          continue;
      }

      c = Esp.read();
      Serial.print(c);
      
      if (c != 'P') {
          continue;
      }

      c = Esp.read();
      Serial.print(c);
      
      if (c != 'D') {
          continue;
      }
      
      Serial.print(c);
      Esp.print('A');
      Serial.print((char)Esp.read());
  }
  if (Serial.available()){
      Esp.print((char)Serial.read());
  }
}
