/*
  Modbus-Arduino Example - TempSensor (Modbus IP ESP8266 AT)
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino
*/

#ifdef ESP8266              //including esp8266 microcontroller  library 
 #include <ESP8266WiFi.h>   //including esp8266 wifi library 
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>  //including modbus library 


#include <DHT.h>               //including temperature sensor library 
#include <MQ2.h>                //including MQ-2 gas sensor  library 
#include <ESP8266WebServer.h>    //including  webserver library  



// Modbus Registers Offsets

const int HUMIdity1_HREG = 0;
const int TEMPERATURE1_HREG = 1;
const int HUMIdity2_HREG= 2;
const int TEMPERATURE2_HREG = 3;
const int HUMIdity3_HREG= 4;
const int TEMPERATURE3_HREG = 5;
const int LPG1_HREG= 6;
const int SMOKE1_HREG = 7;
const int CO1_HREG= 8;
const int LPG2_HREG= 9;
const int SMOKE2_HREG = 10;
const int CO2_HREG= 11;



//  initializing variables 
float h1,t1,h2,t2,h3,t3;
int h11=20,h22=20,h33=20,t11=20,t22=20,t33=30;
int lpg, co, smoke,Smoke1,lpg1,co1;

//ModbusIP object
ModbusIP mb;

//Configuration Of PINS for DHT22 & MQ-2 Sensor.
#define DHTPIN0 0         // D4 pin
#define DHTPIN1 5          // D3 pin
#define DHTPIN2 4          // D1 pin

//TYPE OF DHT SENSOR  & MQ-2 Sensor.
#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
DHT dht1(DHTPIN0, DHTTYPE);
DHT dht2(DHTPIN1, DHTTYPE);
DHT dht3(DHTPIN2, DHTTYPE);
//FOR MQ-2 Sensor.
int Analog_Input = A0;
MQ2 mq2_1(Analog_Input);
ESP8266WebServer server(80);
void sendSensor();
void handle_OnConnect();
void handle_NotFound();

void sendSensor()  //reading values from sensor and prnting these values on serial monitor 
{
  
   h1 = dht2.readHumidity();
   h11=(h1/1)*100;
   //Serial.println(h1);
   t1 = dht2.readTemperature();
   //Serial.println(t1);
   t11=(t1/1)*100;
//
   h2 = dht2.readHumidity();
   h22=(h2/1)*100;
   //Serial.println(h1);
   t2 = dht2.readTemperature();
   //Serial.println(t1);
   t22=(t2/1)*100;
   //
   h3 = dht3.readHumidity();
   h33=(h3/1)*100;
   //Serial.println(h1);
   t3 = dht3.readTemperature();
   //Serial.println(t1);
   t33=(t3/1)*100;
   //
   float* values= mq2_1.read(false); //set it false if you don't want to print the values in the Serial
  //lpg = values[0];
  lpg = mq2_1.readLPG();
  //co = values[1];
  co = mq2_1.readCO();
  //smoke = values[2];
  smoke = mq2_1.readSmoke();
  Smoke1= (smoke*100)/100000;
  lpg1= (lpg)/100;
  co1= (co)/10000;
   //Serial.println(t11);
   // or dht.readTemperature(true) for Fahrenheit
  
  if ((isnan(h1) || isnan(t1))&&(isnan(h2) || isnan(t2))&&(isnan(h3) || isnan(t3)) ) 
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
                             
    Serial.print("Humidity for room 1: ");
    Serial.print(h1);
    Serial.print(" %\t");
    Serial.print("Temperature for room 1: ");
    Serial.print(t1);
    Serial.println(" *C ");
    Serial.print("Humidity for room 2: ");
    Serial.print(h2);
    Serial.print(" %\t");
    Serial.print("Temperature for room 2: ");
    Serial.print(t2);
    Serial.println(" *C ");
    Serial.print("Humidity for Ambient: ");
    Serial.print(h3);
    Serial.print(" %\t");
    Serial.print("Temperature for room Ambient: ");
    Serial.print(t3);
    Serial.println(" *C ");
    Serial.print("LPG:");
    Serial.print(lpg1);
    Serial.print(" CO:");
    Serial.print(co1);
  
    Serial.print("  SMOKE:");
    Serial.print(Smoke1);
    Serial.print(" %  ");
    Serial.println(" *************  ");
    
}

  
void setup() //initializing objects  
{
  Serial.begin(115200);

 dht1.begin();
 dht2.begin();
 dht3.begin();
 mq2_1.begin();
IPAddress local_IP(192, 168, 0, 102); //using that Ip address to send values to webservr and modbus poll 
IPAddress gateway(192, 168, 0, 123);  //
IPAddress subnet(255, 255, 255, 0);
 if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  WiFi.begin("TP-Link_FC18", "84558428"); //connecting to wifi using SSID and password 

  

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());
  server.on("/", handle_OnConnect); //connecting to webserver
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  
  mb.server();  
  mb.addHreg(0,0,20); //adding number of holding registers 
}
 
void loop() //functions we want to repeate again and again  
{
   //Call once inside loop() - all magic here
  sendSensor();
   
   
  mb.Hreg(0, t11);  //sending values to holding registers 
  mb.Hreg(1, h11);
  mb.Hreg(2, t22);
  mb.Hreg(3, h22);
  mb.Hreg(4, t33);
  mb.Hreg(5, h33);
  mb.Hreg(6,lpg1);
  mb.Hreg(7, Smoke1);
  mb.Hreg(8, co1);
  mb.Hreg(9, lpg1);
  mb.Hreg(10,Smoke1);
  mb.Hreg(11, co1);
  mb.task();
  yield();
  
  delay(10);
}
void handle_OnConnect() {


   
  
  
  server.send(200, "text/html", SendHTML(t1,h1,t2,h2,t3,h3,lpg1,co1,Smoke1));
  
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float TempCstat,float Humiditystat,float TempCstat1,float Humiditystat1,float TempCstat2,float Humiditystat2,int lpg1,int smoke1,int co1)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"2\" >\n";
  ptr +="<link href=\"https://fonts.googleapis.com/css?family=Open+Sans:300,400,600\" rel=\"stylesheet\">\n";
  ptr +="<title>HVAC SYSTEM Monitoring Room</title>\n";
  ptr +="<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #333333;}\n";
  ptr +="body{margin-top: 50px;}\n";
  ptr +="h1 {margin: 50px auto 30px;}\n";
  ptr +=".side-by-side{display: inline-block;vertical-align: middle;position: relative;}\n";
  ptr +=".humidity-icon{background-color: #3498db;width: 30px;height: 30px;border-radius: 50%;line-height: 36px;}\n";
  ptr +=".humidity-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr +=".humidity{font-weight: 300;font-size: 60px;color: #3498db;}\n";
  ptr +=".temperature-icon{background-color: #f39c12;width: 30px;height: 30px;border-radius: 50%;line-height: 40px;}\n";
  ptr +=".temperature-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr +=".temperature{font-weight: 300;font-size: 60px;color: #f39c12;}\n";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;right: -20px;top: 15px;}\n";
  ptr +=".data{padding: 10px;}\n";
  ptr +="</style>\n";
  ptr +="<script>\n";
  ptr +="setInterval(loadDoc,200);\n";
  ptr +="function loadDoc() {\n";
  ptr +="var xhttp = new XMLHttpRequest();\n";
  ptr +="xhttp.onreadystatechange = function() {\n";
  ptr +="if (this.readyState == 4 && this.status == 200) {\n";
  ptr +="document.getElementById(\"webpage\").innerHTML =this.responseText}\n";
  ptr +="};\n";
  ptr +="xhttp.open(\"GET\", \"/\", true);\n";
  ptr +="xhttp.send();\n";
  ptr +="}\n";
  ptr +="</script>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  
   ptr +="<div id=\"webpage\">\n";
   
   ptr +="<h1>HVAC MONITORING ROOM</h1>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side temperature-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
   ptr +="width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.915 22\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n";
   ptr +="c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n";
   ptr +="c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n";
   ptr +="c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n";
   ptr +="c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"/>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side temperature-text\">Temperature R#1</div>\n";
   ptr +="<div class=\"side-by-side temperature\">";
   ptr +=(int)TempCstat;
   ptr +="<span class=\"superscript\">°C</span></div>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">Humidity R#1</div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)Humiditystat;
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";
//
 ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side temperature-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
   ptr +="width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.915 22\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n";
   ptr +="c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n";
   ptr +="c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n";
   ptr +="c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n";
   ptr +="c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"/>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side temperature-text\">Temperature R#2</div>\n";
   ptr +="<div class=\"side-by-side temperature\">";
   ptr +=(int)TempCstat1;
   ptr +="<span class=\"superscript\">°C</span></div>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">Humidity R#2 </div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)Humiditystat1;
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";
//
   //
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side temperature-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
   ptr +="width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.915 22\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n";
   ptr +="c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n";
   ptr +="c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n";
   ptr +="c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n";
   ptr +="c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"/>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side temperature-text\">Temperature for Ambient</div>\n";
   ptr +="<div class=\"side-by-side temperature\">";
   ptr +=(int)TempCstat2;
   ptr +="<span class=\"superscript\">°C</span></div>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">Humidity For Ambient </div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)Humiditystat2;
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";
//

   ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">LPG Ratio for R#1</div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)lpg1;
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";

   ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">Carbon Dioxide Ratio for R#1</div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)co1;
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";


   ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">Smoke Ratio for R#1</div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)(smoke1);
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";
   
//
ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">LPG Ratio for R#2</div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)lpg1;
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";

   ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">Carbon Dioxide Ratio for R#2</div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)co1;
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";


   ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">Smoke Ratio for R#2</div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)(smoke1);
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";
   //
   

  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
