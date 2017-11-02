#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

ESP8266WebServer server(80);
const char* host = "esp8266";
const char* ssid = "U0324047test";
const char* password = "12345678";
String content;
String st;
int statusCode;

void setup() {
  // put your setup code here, to run once:
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  EEPROM.begin(512);
  delay(3000);
  String essid;
  for (int i = 0; i < 32; ++i)  essid += char(EEPROM.read(i)); 
  Serial.print("\n\nSSID NAME : ");
  Serial.println(essid);
  char charBuf[32];
  essid.toCharArray(charBuf, 32);
  delay(1000);
  String epassword = "";
  for (int i = 32; i < 96; ++i)   epassword += char(EEPROM.read(i));
  Serial.print("PASSWORD NAME : ");
  Serial.println(epassword);
  WiFi.begin(essid.c_str(), epassword.c_str()); 
  if(testWiFi()){
    createWebServer(1);
    server.begin();
    }
  else{
  ScanNetwork();
  WiFi.softAP(ssid,password,6,0);
  Serial.print("\nSoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  delay(5000);
  createWebServer(0);
  server.begin();
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
}
void createWebServer(int webtype){
  if(webtype == 0){
    server.on("/",[](){
        IPAddress ip = WiFi.softAPIP();
        content = "<!DOCTYPE HTML><html><title>雲端電燈的無線網路設定</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><p>     你好, 請輸入您家裡的無線網路設定</p>";
        content += st;
        //&nbsp;代表塞一個空白, 重覆5個就是塞5個空白     
        content += "<br>";
        content += "<form method='get' action='setting'>";
        content += "<table border=\"0\"><tr><td><label>SSID</label></td><td><input type=\"text\" placeholder=\"請輸入要連接的SSID\" name='ssid' maxlength=32 size=64></td></tr>";
        content += "<tr><td><label>PASSWORD</label></td><td><input type=\"text\" placeholder=\"請輸入要連接的密碼\" name='pass' maxlength=64 size=64></td></tr></table>";
        content += "<input type='reset' value=\"重設\">&nbsp;&nbsp;&nbsp;<input type='submit' value=\"儲存\"></form></html>";
        server.send(200, "text/html", content);  //200代表伺服器回應OK, text/html代表用html網頁類型, 不加這個會找不到網頁
      });
    server.on("/setting",[](){
    String qsid = server.arg("ssid");
    String qpass = server.arg("pass");
    if (qsid.length() > 0 && qpass.length() > 0) {
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
      Serial.println(qsid);
      Serial.println("");
      Serial.println(qpass);
      Serial.println("");
        
      Serial.println("writing eeprom ssid:");
      for (int i = 0; i < qsid.length(); ++i)
        {
        EEPROM.write(i, qsid[i]);
        Serial.print("Wrote: ");
        Serial.println(qsid[i]);
        }
      Serial.println("writing eeprom pass:");
      for (int i = 0; i < qpass.length(); ++i)
        {
        EEPROM.write(32+i, qpass[i]);
        Serial.print("Wrote: ");
        Serial.println(qpass[i]);
        }   
      EEPROM.commit(); //EEPROM.write並不會馬上寫入EEPROM, 而是要執行EEPROM.commit()才會實際的寫入EEPROM
      content = "儲存成功, 請按RESET鍵重新開機!";
      statusCode = 200;
      }
    else {         
      content = "<p>輸入錯誤!!!SSID或PASSWORD任何其中一欄不能是空白, 按上一頁重新輸入</p>";
      content += "<input type=\"button\" value=\"上一頁\" onclick=\"self.location.href='/'\"></html>";
      //content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
      Serial.println("Sending 404");   
      }   
    server.send(statusCode, "text/html", content);     
    });
  }
  else if(webtype == 1){ 
    server.on("/",[](){
      IPAddress ip = WiFi.localIP();
      content = "<!DOCTYPE HTML><html><title>雲端電燈控制網頁</title>";
      content += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
      content += "<body background=\"https://goo.gl/xc7YUc\">";
      content += "<p>你好, 請按下面的按鈕來開啟或關閉電燈</p>";
      content += "<input type=\"button\" value=\"打開電燈\" onclick=\"self.location.href='/led/on'\">";
      content += "&nbsp;&nbsp;&nbsp;<input type=\"button\" value=\"關閉電燈\" onclick=\"self.location.href='/led/off'\">";     
      content += "<br><br>";  //空一行
      content += "<p>若要清除無線網路設定, 請按下方的清除無線網路設定按鈕</p>";
      content += "<input type=\"button\" value=\"清除無線網路設定\" onclick=\"self.location.href='/cleareeprom'\"></html>";
      server.send(200, "text/html", content);
      });       
    server.on("/led/on",[](){
        content = "<!DOCTYPE HTML><html>";
        content += "<p>LED already Turn On</p>";
        content += "<Meta http-equiv=\"ReFresh\" Content=\"1; URL='/'\">";
        content += "</html>";
        server.send(200, "text/html", content);
        digitalWrite(16, LOW);
      });
    server.on("/led/off",[](){      
      content = "<!DOCTYPE HTML><html>";
      content += "<p>LED already Turn Off</p>";
      content += "<Meta http-equiv=\"ReFresh\" Content=\"1; URL='/'\">";
      content += "</html>";
      server.send(200, "text/html", content);
      digitalWrite(16, HIGH);
      });
    server.on("/cleareeprom",[](){                             
      for (int i = 0; i < 96; ++i)  EEPROM.write(i, 0);
      EEPROM.commit();
      content = "<!DOCTYPE HTML><html>";
      content += "<meta http-equiv=\"ReFresh\" charset=\"utf-8\" content=\"1; URL='/'\"><p>已清除無線網路設定</p></html>";
      server.send(200, "text/html", content);
      Serial.println("\nclearing eeprom");
      WiFi.disconnect();
      });
  }
}
void ScanNetwork(){
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.print("Scan Network Done...and ");
  if (n == 0) Serial.println("No Any Networks Found!");
  else{
    Serial.print(n);
    Serial.println(" Networks Found!");
    for (int i = 0; i < n; ++i){
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      byte encryption = WiFi.encryptionType(i);
      Serial.print(" Encryption Type:");
      switch (encryption) {
        case 2: Serial.println("TKIP(WPA)");
                break;
        case 5: Serial.println("WEP");
                break;
        case 4: Serial.println("CCMP(WPA)");
                break;
        case 7: Serial.println("NONE");
                break;
        case 8: Serial.println("AUTO(WPA or WPA2)");
                break;
        case 255: Serial.println("802.1x");
                }
      delay(100);
      }
  }
  Serial.println("");
  String k;
  st = "<ol type=\"1\" start=\"1\">";
  for (int i = 0; i < n; ++i){
    // Print SSID and RSSI for each network found
    st += "<table border=\"0\"><tr><td width=\"300px\">";
    k=String(i+1);
    st += k + ". ";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += "</td><td width=\"200px\">";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
    byte encryption = WiFi.encryptionType(i);
    switch (encryption) {
      case 2: st += "TKIP(WPA)";break;
      case 5: st += "WEP";break;
      case 4: st +="CCMP(WPA)";break;
      case 7: st +="NONE";break;
      case 8: st +="AUTO(WPA or WPA2)";break;
      case 255: st +="802.1x";break;
      } 
    st += "</td></tr>";   
    }
  st += "</ol></table><br>";
}
bool testWiFi(void){
  int c = 0;
  Serial.println("Waiting for WiFi to connect...");
  while(c<15){
    if(WiFi.status() == WL_CONNECTED) {return true;} 
    delay(1000);
    Serial.print(WiFi.status());
    ++c;
  }
  Serial.print("\n");
  return false;
}
