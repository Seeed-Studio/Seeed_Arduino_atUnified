#include "UnifiedAtWifi.h"
#include "UnifiedAtEvent.h"

void setup(){
    Serial.begin(115200);
    while(!Serial);
    if (atWifiMode(3) == Fail){
        Serial.println("mode set error");
        return;
    }
    atWifiScan([](){});
    {
        WifiLinkedAp cur;
        WifiLinkedAp ap;
        ap.ssid = "Tenda_2B2BC0";
        if (atWifiConnect(ap.ssid, "19710101") == Fail){
            Serial.println("connect error");
            return;
        }
        Serial.printf("Start to query WiFi Connection\r\n");
        if (atWifiConnect(& cur) == Fail){
            Serial.println("failed to got connected infomation");
            return;
        }
        Serial.printf("VVV %s\n", cur.ssid.c_str());
    }
}

void loop(){
}
