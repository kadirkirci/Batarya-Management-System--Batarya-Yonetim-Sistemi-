#include <OneWire.h>
#include "SPI.h"




OneWire  ds(7);  // on pin 7 (a 4.7K resistor is necessary)

#define WRCFG 0x01 //Write Configuration Registers
#define RDCFG 0x02 // Read config
#define RDCV 0x04 // Read cells
#define STCVAD 0x10 // Start all A/D's - poll status
#define RDFLG 0x06 //Read Flags
#define RDTMP 0x08 //Read Temperatures
#define STCDC 0x60 //A/D converter and poll Status
#define STOWAD 0x20 //Staret Test - poll status
#define STTMPAD 0x30// Temperature Reading - ALL
#define address1 0x80
#define address2 0x81
#define PLINT 0x50
#define cs 10   //  Uno - 10 Mega 53
#define mosi 11 //  Uno - 11 Mega 51
#define miso 12 //  Uno - 12 Mega 50
#define sck 13 //   Uno - 13 Mega 52 

#define Toplam_pil 20 // Toplam pil sayısı
#define dengeleme_fark_hassas 0.03 // 

const int relay=6;
int karakter=1;


int a=1;

byte CFGR1 = 0;
byte CFGR2 = 0;
float toplamvolt;
boolean dengeleme_basla=false;
byte address;
int highestCellNumber;
int lowestCellNumber;
float pilvolt[21];
float toplamVoltaj=0;
float voltageLowest;
float voltageHighest;
int w=1;
int f;

//Functions
byte byteTemp;
void setup()
{
pinMode(cs,OUTPUT);
pinMode(mosi,OUTPUT);
pinMode(miso,INPUT);
pinMode(sck,OUTPUT);
pinMode(relay,OUTPUT);


digitalWrite(cs, HIGH);

SPI.setBitOrder(MSBFIRST);
SPI.setDataMode(SPI_MODE3);
SPI.setClockDivider(SPI_CLOCK_DIV16);
SPI.begin();
Serial.begin(9600);

}
void loop()
{

address=address1;  
readV();
//readConfig();
delay(1000);
address=address2;
readV2();
//send_data();

//ToplamVoltajHesap();
//highestCellNumber = highestCell();
//lowestCellNumber = lowestCell();

if(dengeleme_basla==true)
{
  

balance();
}   
        
writeReg();
//readConfig();
sicaklik();
}


void writeReg()
{

//Serial.println("Writing config1...");
digitalWrite(cs, LOW);
SPI.transfer(address);
SPI.transfer(WRCFG);
SPI.transfer(0x01);//0
SPI.transfer(CFGR1);//1
SPI.transfer(CFGR2);//2
SPI.transfer(0x00);//3
SPI.transfer(0x53);//4
SPI.transfer(0xAB);//5
digitalWrite(cs, HIGH);
}








void send_data()
{

     int i=0 ; 
     String str="(PV)";    
     for(i=1;i<20;i++)
     {
         str=str+"(" + String(i) + ")" + "{" + String(pilvolt[i])+ "}" +"(/" + String(i) + ")"; 
           
     }
     str=str+"(/PV)";
     Serial.println(str);






  
}

void readV()
{
  CFGR2=0x00;
  CFGR1=0x00;
address=address1;  
writeReg();

digitalWrite(cs,LOW);
SPI.transfer(STCVAD);
delay(20); 
digitalWrite(cs,HIGH); 

byte volt[18];
digitalWrite(cs,LOW);
SPI.transfer(address);
SPI.transfer(RDCV);
for(int j = 0; j<18;j++)
{
volt[j] = SPI.transfer(RDCV);
}
digitalWrite(cs,HIGH);

pilvolt[1]=(((volt[0] & 0xFF) | (volt[1] & 0x0F) << 8)*1.5*0.001);
pilvolt[2]=(((volt[1] & 0xF0) >> 4 | (volt[2] & 0xFF) << 4)*1.5*0.001);
pilvolt[3]=(((volt[3] & 0xFF) | (volt[4] & 0x0F) << 8)*1.5*0.001);
pilvolt[4]=(((volt[4] & 0xF0) >> 4 | (volt[5] & 0xFF) << 4)*1.5*0.001);
pilvolt[5]=(((volt[6] & 0xFF) | (volt[7] & 0x0F) << 8)*1.5*0.001);
pilvolt[6]=(((volt[7] & 0xF0) >> 4 | (volt[8] & 0xFF) << 4)*1.5*0.001);
pilvolt[7]=(((volt[9] & 0xFF) | (volt[10] & 0x0F) << 8)*1.5*0.001);
pilvolt[8]=(((volt[10] & 0xF0) >> 4 | (volt[11] & 0xFF) << 4)*1.5*0.001);
pilvolt[9]=(((volt[12] & 0xFF) | (volt[13] & 0x0F) << 8)*1.5*0.001);
pilvolt[10]=(((volt[13] & 0xF0) >> 4 | (volt[14] & 0xFF) << 4)*1.5*0.001);
pilvolt[11]=(((volt[15] & 0xFF) | (volt[16] & 0x0F) << 8)*1.5*0.001);
pilvolt[12]=(((volt[16] & 0xF0) >> 4 | (volt[17] & 0xFF) << 4)*1.5*0.001);

}


void readV2()
{  
  CFGR2=0x00;
  CFGR1=0x00;
address=address2;
writeReg(); 
digitalWrite(cs,LOW);
SPI.transfer(STCVAD);
delay(20); // wait at least 12ms as per data sheet, p.24
digitalWrite(cs,HIGH);

byte volt2[18];
digitalWrite(cs,LOW);
SPI.transfer(address);
SPI.transfer(RDCV);
for(int j2 = 0; j2<18;j2++)
{
volt2[j2] = SPI.transfer(RDCV);
}
digitalWrite(cs,HIGH);

pilvolt[13]=(((volt2[0] & 0xFF) | (volt2[1] & 0x0F) << 8)*1.5*0.001);
pilvolt[14]=(((volt2[1] & 0xF0) >> 4 | (volt2[2] & 0xFF) << 4)*1.5*0.001);
pilvolt[15]=(((volt2[3] & 0xFF) | (volt2[4] & 0x0F) << 8)*1.5*0.001);
pilvolt[16]=(((volt2[4] & 0xF0) >> 4 | (volt2[5] & 0xFF) << 4)*1.5*0.001);
pilvolt[17]=(((volt2[6] & 0xFF) | (volt2[7] & 0x0F) << 8)*1.5*0.001);
pilvolt[18]=(((volt2[7] & 0xF0) >> 4 | (volt2[8] & 0xFF) << 4)*1.5*0.001);
pilvolt[19]=(((volt2[9] & 0xFF) | (volt2[10] & 0x0F) << 8)*1.5*0.001);
pilvolt[20]=(((volt2[10] & 0xF0) >> 4 | (volt2[11] & 0xFF) << 4)*1.5*0.001);

}

float ToplamVoltajHesap()
{toplamVoltaj=0;
   for(w=1; w<21; w++)
  { 
  toplamVoltaj=toplamVoltaj+pilvolt[w];
  }
  Serial.print('U');
  Serial.println(toplamVoltaj);  
}

void balance()
{
  float difference[Toplam_pil];
  CFGR1 = 0x00;
  CFGR2 = 0x00;
  byte temp;
  int x;
  int i;
  float fark[Toplam_pil+1];
  

  
  
  for(i=1; i<Toplam_pil; i++)
  {
    
    if(i!=lowestCellNumber)
    {  fark[i]=pilvolt[i]-pilvolt[lowestCellNumber];
          if(i>12 & fark[i] >= dengeleme_fark_hassas)
          {
            address=address2;
            x = i - 13; 
            temp = 0x01;
            temp = temp << x;
            CFGR1 = CFGR1 + temp;
           // Serial.println("1.dongu   ");
            writeReg();
          }
  
  if(i<=8 & fark[i] >= dengeleme_fark_hassas)
        {   address=address1;
            x = i - 1; 
            temp = 0x01;
            temp = temp << x;
            CFGR1 = CFGR1 + temp;
           // Serial.println("2.dongu   ");
            writeReg();
         }
         
  if(i>8 & i<=12 & fark[i] >= dengeleme_fark_hassas)
        {   address=address1;
            x = i - 9; 
            temp = temp << x;
            CFGR2 = CFGR2 + temp;
           // Serial.print("3.dongu   ");
            writeReg();
         }
    }
    
  }
  
  

         
         
   

         
          Serial.print("En yuksek pil:   ");
          Serial.print(highestCell());
          Serial.print(" ");
          Serial.println(pilvolt[highestCell()]);
          Serial.print("En dusuk pil:   ");
          Serial.print(lowestCell());
          Serial.print(" ");
          Serial.println(pilvolt[lowestCell()]);
          Serial.print("CFGR1: ");
          Serial.println(CFGR1,BIN);
          Serial.print("Adres: ");
          Serial.println(address,HEX);
                 
}

int highestCell()
{
  voltageHighest = 0;
  int cellNumber;
  for (int i = 1; i <= Toplam_pil; i++)
  {
    if (pilvolt[i] > voltageHighest)
    {
      voltageHighest = pilvolt[i];
      cellNumber = i;
    }
  }
  return cellNumber;
}

int lowestCell()
{
  voltageLowest = 99;
  int cellNumber;
  for(int i = 1; i <= Toplam_pil; i++)
  {
    if (pilvolt [i] < voltageLowest)
    {
      voltageLowest = pilvolt[i];
      cellNumber = i;
    }
  }
  return cellNumber;
}

void readConfig()
{byte byteTemp;
  digitalWrite(cs, LOW);
  SPI.transfer(address);
  SPI.transfer(RDCFG);//Read
  for(int i = 0; i < 6; i++)
  {
  byteTemp = SPI.transfer(RDCFG);
  Serial.println(byteTemp, HEX);
  }
  digitalWrite(10, HIGH);

}




void sicaklik(){
  
  byte i;
 
  byte present = 0;

  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
  // Serial.println("No more addresses.");
   
    
    Serial.println();
    ds.reset_search();
    delay(20); // delay(250);
    return;
  }
  
  

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(800);     
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
 
  Serial.println(karakter);
  
  Serial.println(celsius);
  if(celsius>60)
 {
  digitalWrite(relay,HIGH);
 }
  karakter = karakter+1;
  if(karakter>=20)
  {
    karakter = 1;
  }
  
 
}

