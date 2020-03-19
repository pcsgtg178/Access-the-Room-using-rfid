#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <RFID.h>

#define SS_PIN 4 //D2           config variable SS_PIN = 4 or GPIO4(D2)
#define RTS_PIN 5 //D1          config variable RTS_PIN = 5 or GPIO5(D1)
#define ssid "Pattapon" //      config variable ssid = "Pattapon"
#define pass "pattapon123" //   config variable pass = "pattapon123"
#define _roomid 1  //           config variable _roomid = 1
#define BUZZER 14 //D5          config variable BUZZER = 14 or GPIO14(D5)
#define LED_GREEN 12 //D6       config variable LED_GREEN = 12 or GPIO12(D6)
#define LED_RED 13 //D7         config variable LED_RED = 13 or GPIO13(D7)

RFID rfid(SS_PIN, RTS_PIN);//   config RFID use SS_PIN and RTS_PIN

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
    
    WiFi.begin(ssid,pass); //config Connect WiFi with ssid and pass

    while (WiFi.status() != WL_CONNECTED) //loop wait connect wifi
   {
      delay(500);
      Serial.print(".");
   }
   
    Serial.println("");
    Serial.println("WiFi connected"); 
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); //print ip adress wifi
    
    digitalWrite(BUZZER,LOW);     //give BUZZER OFF
    digitalWrite(LED_GREEN,LOW);
    digitalWrite(LED_RED,LOW);
}

void loop(){
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
        
        const char* root_0_USERID = root_0["USERID"]; //config variable string root_0_USERID = object USERID
        const char* root_0_NAME = root_0["NAME"];     //config variable string root_0_NAME = object NAME
        const char* root_0_ROOMID = root_0["ROOMID"]; //config variable string root_0_ROOMID = object ROOMID
        const char* root_0_DETAIL = root_0["DETAIL"]; //config variable string root_0_DETAIL = object DETAIL

        Serial.print("USERID : "); Serial.println(root_0_USERID); //print  root_0_USERID
        Serial.print("NAME : "); Serial.println(root_0_NAME);
        Serial.print("ROOMID : "); Serial.println(root_0_ROOMID);
        Serial.print("DETAIL : "); Serial.println(root_0_DETAIL);
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
