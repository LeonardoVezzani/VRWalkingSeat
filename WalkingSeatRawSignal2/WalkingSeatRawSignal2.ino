
/*
  THIS SCRIPT compute vectors on the esp 4x4 float a,b,c,vel
*/

#include <MPU6050_tockn.h>
#include <HX711_ADC.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>


/*
   HX711 - SCALES
*/
// SN_ one for each scale
const int SC_sck = 12; // D5
const int SC_out = 14; // D6

const int SA_sck = 2; //D4
const int SA_out = 0; //D3

const int SB_sck = 15; //D8
const int SB_out = 13; //D7

/*
  CELLE DI CARICO
*/
HX711_ADC LoadCellA(SA_out, SA_sck);
HX711_ADC LoadCellB(SB_out, SB_sck);
HX711_ADC LoadCellC(SC_out, SC_sck);

long tSent=0;



/*
   ESP8266----------------------------------------------------------------------------------------------------
*/
/*
const char* ssid = "CESARS";    // dati polito
const char* password = "password";

// Set your Static IP address
IPAddress local_IP(192, 168, 137, 45);
*/
const char* ssid = "Op-Leo";    // dati OP
const char* password = "bananabanana";
/*
const char* ssid= "TP-Link_34BE"; //dati firenze
const char* password = "11869894";

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184); 
*/

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);

/*
 * MPU6050 TOCKN
 */
MPU6050 mpu6050(Wire);

WiFiServer wifiServer(34197);

//=================================================================================================================SET UP
void setup() {
  
      Serial.begin(115200);
     /*
        CALIBRATION HX711----------------------------------------------------------------------------------------------------
     */
      LoadCellA.begin();
      LoadCellB.begin();
      LoadCellC.begin();
      float calibrationValue = 5000; 
      long stabilizingtime = 10000; // tare precision can be improved by adding a few seconds of stabilizing time
      boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
      
      if( LoadCellA.getTareTimeoutFlag() ||
          LoadCellB.getTareTimeoutFlag() ||
          LoadCellC.getTareTimeoutFlag()) 
          Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    
      
      /*
      * LOAD CELL --------------------------------------------------------------------------------------------------------
      */
      
      Serial.print("Calibrating...");
  
      /*CELL A*/
      LoadCellA.start(stabilizingtime, _tare);
      if (LoadCellA.getTareTimeoutFlag()) {
          Serial.println("Timeout, check A MCU>HX711 wiring and pin designations");
      }
      else {
        LoadCellA.setCalFactor(calibrationValue);
      }
      /*CELL B*/
      LoadCellB.start(stabilizingtime, _tare);

      if (LoadCellB.getTareTimeoutFlag()) {
        Serial.println("Timeout, check B MCU>HX711 wiring and pin designations");
      }
      else {
        LoadCellB.setCalFactor(calibrationValue);
      }
      /*CELL C*/
      LoadCellC.start(stabilizingtime, _tare);
      if (LoadCellC.getTareTimeoutFlag()) {
        Serial.println("Timeout, check C MCU>HX711 wiring and pin designations");
      }
      else {
        LoadCellC.setCalFactor(calibrationValue);
      }

    /*
    * WIFI SOCKET TCP
    */


/*if (!WiFi.config(local_IP, gateway, subnet)) {
  Serial.println("STA Failed to configure");
}*/
      WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connecting..");
        delay(1000);
      }

      Serial.print("Connected to WiFi. IP:");
      Serial.println(WiFi.localIP());
      wifiServer.begin();

      /*
        CALIBRATION MPU6050 ------------------------------------------------------------------------------------------------
      */
  
      Wire.begin();
      mpu6050.begin();
      mpu6050.calcGyroOffsets(true);

}

//=================================================================================================================LOOP
void loop()
{
/* VARIABLES */
float a, b, c,aOld,bOld,cOld =0; // pesi ogni bilancia
float vel=0;
  WiFiClient client = wifiServer.available();
  while(client){
          client.setNoDelay(true);
          /*
          * SCALE
          */
          /*CELL A WEIGH*/
          if (LoadCellA.update()) a = (LoadCellA.getData());//- aOld);  
          else a=aOld;
          /*CELL B WEIGH*/
          if (LoadCellB.update()) b =(LoadCellB.getData());//- bOld);
          else b=bOld;
          /*CELL C WEIGH*/
          if (LoadCellC.update()) c = (LoadCellC.getData());//- cOld);
          else c= cOld;  

          mpu6050.update();
          vel = mpu6050.getGyroZ();

byte dataArray[16] = {
      ((uint8_t*)&a)[0],
      ((uint8_t*)&a)[1],
      ((uint8_t*)&a)[2],
      ((uint8_t*)&a)[3],
      
      ((uint8_t*)&b)[0],
      ((uint8_t*)&b)[1],
      ((uint8_t*)&b)[2],
      ((uint8_t*)&b)[3],
      
      ((uint8_t*)&c)[0],
      ((uint8_t*)&c)[1],
      ((uint8_t*)&c)[2],
      ((uint8_t*)&c)[3],
      
      ((uint8_t*)&vel)[0],
      ((uint8_t*)&vel)[1],
      ((uint8_t*)&vel)[2],
      ((uint8_t*)&vel)[3]
      };


Serial.print("a" ); 
Serial.print( ",");
Serial.print(a ); 
Serial.print(",");

Serial.print("b" ); 
Serial.print( ",");
Serial.print(b ); 
Serial.print(",");

Serial.print("c" ); 
Serial.print( ",");
Serial.print(c ); 
Serial.print( ",");

Serial.print("vel" ); 
Serial.print( ",");
Serial.print(vel ); 
Serial.println(' ');



    client.write(dataArray,16);

      
    tSent = millis();
    /*
    * PASSAGGIO VALORI TRA PACCHETTI --> POTREBBE CREARE PROBLEMI DI SMOOTHING IN CASO DI TANTE MISURE MANCATE.
    */
    aOld=a;
    bOld=b;
    cOld=c;
  }
}
