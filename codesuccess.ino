#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <RFID.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SS_PIN D4 //            config variable SS_PIN = D4 or D4(GPIO5)
#define RTS_PIN D3 //           config variable RTS_PIN = D3 or D3(GPIO5)
#define ssid "Pattapon" //      config variable ssid = "Pattapon"
#define pass "pattapon123" //   config variable pass = "pattapon123"
#define _roomid 1  //           config variable _roomid = 1
#define BUZZER 14 //D5          config variable BUZZER = 14 or GPIO14(D5)
#define LED_GREEN 12 //D6       config variable LED_GREEN = 12 or GPIO12(D6)
#define LED_RED 13 //D7         config variable LED_RED = 13 or GPIO13(D7)

RFID rfid(SS_PIN, RTS_PIN);//   config RFID use SS_PIN and RTS_PIN
Adafruit_SSD1306 OLED(-1)

int i ;
int serNum0;
int serNum1;
int serNum2;
int serNum3;
int serNum4;
String serNum;
String _rfidNum;
String _str;      //variable string GET
String _res;      //variable function GET use recive response value from server
String response;  //Use recive value from function GET server
const char* host = "192.168.43.57"; // variable host server use IP adress or domain name
const int port = 80; //port number 80 is localhost 
const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) +80; //capacity data json size 1 array 4 object 80 byte for input duplication  
// example [{name:"chai",lastname:"wat",user:"plug",pass:"pass"}]     [] = 1 array, name, lastname, user, pass = 4 object 
// https://arduinojson.org/v5/assistant/

DynamicJsonBuffer jsonBuffer(capacity); //config use capacity

void setup(){

    Serial.begin(115200); //config buad rate
    SPI.begin(); // SPI start
    rfid.init();  // rfid start
    pinMode(BUZZER,OUTPUT); // config BUZZER is OUTPUT
    pinMode(LED_GREEN,OUTPUT);
    pinMode(LED_RED,OUTPUT);
    OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    WiFi.begin(ssid,pass); //config Connect WiFi with ssid and pass
    
    while (WiFi.status() != WL_CONNECTED) //loop wait connect wifi
   {
      OLED.clearDisplay();
      OLED.setTextColor(WHITE);
      OLED.setTextSize(2);
      OLED.setCursor(0,0);
      OLED.print("Wait connect WiFi..");
      OLED.display();
      delay(500);
      Serial.print(".");
   }
    OLED.clearDisplay();
    OLED.setTextColor(WHITE);
    OLED.setTextSize(2);
    OLED.setCursor(0,0);
    OLED.print("WiFi connected");
    OLED.display();
    Serial.println("");
    Serial.println("WiFi connected"); 
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); //print ip adress wifi
    
    digitalWrite(BUZZER,LOW);     //give BUZZER OFF
    digitalWrite(LED_GREEN,LOW);
    digitalWrite(LED_RED,LOW);
    DisplayWAiT_CARD();
}

void loop(){
    DisplayTitle();
    DisplayWAiT_CARD();
    serNum = response = ""; //give sernum and variable response empty
    if(rfid.isCard()){ // if have scan id card
        if(rfid.readCardSerial()) //if status read id card success
        {
            serNum0 = rfid.serNum[0]; //serNum0  = rfid number at index 0  
            serNum1 = rfid.serNum[1]; //serNum1  = rfid number at index 1
            serNum2 = rfid.serNum[2]; //serNum2  = rfid number at index 2
            serNum3 = rfid.serNum[3]; //serNum3  = rfid number at index 3
            serNum4 = rfid.serNum[4]; //serNum4  = rfid number at index 4

            serNum = String(serNum0)+'-'+String(serNum1)+'-';
            serNum += String(serNum2)+'-';
            serNum += String(serNum3)+'-';
            serNum += String(serNum4);        // serNum = serNum0+'-'+serNum1+'-'+serNum2+'-'+serNum3+'-'+serNum4
                                              // example serNum = 111-111-111-111-111
            response = ScanRFID_GET(serNum);  //response = value return from function ScanRFID_GET and parameter serNum
            if(response.length() > 0){        //if response length > 0 
                digitalWrite(BUZZER,HIGH);   
                digitalWrite(LED_GREEN,HIGH);
                digitalWrite(LED_RED,LOW); // BUZZER ON, LED_GREEN ON, LED_RED OFF
                delay(1000);              //delay 1 second
            }else{                            // if response length < 0 
                digitalWrite(BUZZER,HIGH);
                digitalWrite(LED_GREEN,LOW);
                digitalWrite(LED_RED,HIGH); // BUZZER, LED_GREEN OFF, LED_RED ON
                delay(1000);            //delay 1 second
            }
            Serial.print("ID Card : "); Serial.println(serNum);
            JsonArray& root = jsonBuffer.parseArray(response); //tranform response(is string) to JSON root
            JsonObject& root_0 = root[0]; //give root at index 0 = root_0 is JSON  object example {name:"chai",lastname:"wat"}
            
            String root_0_USERID = root_0["USERID"]; //config variable string root_0_USERID = object USERID
            String root_0_NAME = root_0["NAME"];     //config variable string root_0_NAME = object NAME
            String root_0_ROOMID = root_0["ROOMID"]; //config variable string root_0_ROOMID = object ROOMID
            String root_0_DETAIL = root_0["DETAIL"]; //config variable string root_0_DETAIL = object DETAIL
            
            if(root_0_USERID.length() > 0){
              Serial.print("USERID : "); Serial.println(root_0_USERID); //print  root_0_USERID
              Serial.print("NAME : "); Serial.println(root_0_NAME);
              Serial.print("ROOMID : "); Serial.println(root_0_ROOMID);
              Serial.print("DETAIL : "); Serial.println(root_0_DETAIL);
              FoundCard(root_0_NAME);
              
            }else{
              CardNotFound();
            }
           
        }
    }
    rfid.halt();  //stop rfid   
}
String ScanRFID_GET (String rfid_num) {   //function ScanRFID_GET and parameter String rfid_num
    _rfidNum = String(rfid_num);  //give _rfidNum = parameter rfid_num is tranform String
    Serial.print("rfid value : ");
    Serial.println(_rfidNum);
    WiFiClient client;            // give client is WiFiClient
    if(client.connect(host, port)) {          //give client connect server with host(ip address or damain name), port(localhost)
        _str = "GET /pronew/add.php?cardid=";
        _str += _rfidNum;
        _str += "&roomid=";
        _str += _roomid;
        _str += " HTTP/1.1\r\n";
        _str += "Host: ";
        _str += host;
        _str += ":";
        _str += port;
        _str += "\r\n";
        _str += "Connection: keep-alive\r\n\r\n"; //Get pattern read : https://www.w3schools.com/tags/ref_httpmethods.asp
        client.print(_str); //use _str(Get pattern)
        delay(3000);  //delay 3 second
        Serial.println("Send Data to Database");
        while (client.available()) {    //loop check response from server
            _res = client.readStringUntil('\r'); //give _res = response value from server
        }
        return _res; //return _res give fucntion
    } 
    else {
            //Nothing..
    }
}

String ScanRFID_POST (String rfid_num) { // Same as the GET function but is POST.
    _rfidNum = String(rfid_num);
    WiFiClient client;
    
    if(client.connect(host, port)) 
    {
        _str = "POST /pronew/add.php";
        _str += " HTTP/1.1\r\n";
        _str += "Host: ";
        _str += host;
        _str += ":";
        _str += port;
        _str += "\r\n";
        _str += "cardid: ";
        _str += _rfidNum;
        _str += "\r\n";
        _str += "Content-Type: application/x-www-form-urlencoded";
        _str += "\r\n";
        _str += "Connection: keep-alive\r\n\r\n"; //POST pattern read : https://www.w3schools.com/tags/ref_httpmethods.asp
        client.print(_str);
        delay(3000);
        Serial.println("Send Data to Database");
        while (client.available()) {
            _res = client.readStringUntil('\r');
        }
        return _res;
    } 
    else {
            //Nothing..
    }
}

void DisplayWAiT_CARD()
{
  OLED.clearDisplay();
  OLED.setTextColor(WHITE);
  OLED.setTextSize(2);
  OLED.setCursor(0,15);
  OLED.print("Please Tag Card");
  OLED.display();
}

void DisplayTitle()
{
  OLED.clearDisplay();
  OLED.setTextColor(WHITE);
  OLED.setTextSize(2);
  OLED.setCursor(0,0);
  OLED.print("    WELCOME    ");
  OLED.display();
}

void FoundCARD(String username){
  OLED.clearDisplay();
  OLED.setTextColor(WHITE);
  OLED.setTextSize(2);
  OLED.setCursor(0,15);
  OLED.print("Hi  ");
  OLED.setCursor(0,19);
  OLED.print(username);
  OLED.display();
}

void CardNotFound(){
  OLED.clearDisplay();
  OLED.setTextColor(WHITE);
  OLED.setTextSize(2);
  OLED.setCursor(0,15);
  OLED.print("Card not Found");
  OLED.display();
}
