#include <SPI.h>
#include <TimerOne.h>
#include <OneWire.h>

#define WRCFG 0x01 //Write Configuration Registers
#define RDCFG 0x02 // Read config
#define RDCV 0x04 // Read cells
#define STCVAD 0x10 // Start all A/D's - poll status
#define RDFLG 0x06 //Read Flags
#define RDTMP 0x08 //Read Temperatures
#define STCDC 0x60 //A/D converter and poll Status
#define STOWAD 0x20 //Staret Test - poll status
#define STTMPAD 0x30// Temperature Reading - ALL
#define PLINT 0x50

const int analogIn = A0;

#define cs 10   //  Uno - 10 Mega 53
#define mosi 11 //  Uno - 11 Mega 51
#define miso 12 //  Uno - 12 Mega 50
#define sck 13 //   Uno - 13 Mega 52 
#define relay 6

#define Toplam_pil 20 // Toplam pil sayısı
#define dengeleme_fark_hassas 0.009 // 

OneWire  ds(7);

unsigned char address1=0x80;
unsigned char address2=0x81;
unsigned char address;
byte CFGR1 = 0; 
byte CFGR2 = 0;
float pilvolt[20];
float toplamVolt;
int maxPil,minPil;
boolean dengelemeBasla=false;
String sicaklikYazi, gelenVeri, _hiz = "0";
double Amper,ToplamAkim,okunanDeger,ortalamaAkim;
double offsetCurrent =81;
String akimYazi;

void setup()
{
pinMode(cs,OUTPUT);
pinMode(mosi,OUTPUT);
pinMode(miso,INPUT);
pinMode(sck,OUTPUT);
pinMode(relay,OUTPUT);

digitalWrite(cs, HIGH);
address = address2; 
SPI.setBitOrder(MSBFIRST);
SPI.setDataMode(SPI_MODE3);
SPI.setClockDivider(SPI_CLOCK_DIV16);
SPI.begin();
Serial.begin(9600);

//wakeUp();
writeReg();
//readConfig();
}

void loop(){
  adresDegisimi();
  writeReg();
  // deneme();
     delay(100);
 // readConfig();
 
  readVoltage();
  
  if(dengelemeBasla==true)
    balance();
  else
  {
   CFGR1 = 0;
   CFGR2 = 0;
  }
  sicaklik();
  akim_fonksiyon();
  read_data();
  delay(100);
}

void wakeUp()
{ int i;
  digitalWrite(cs,LOW);
    SPI.transfer(0x01);   // Command Set
    for(i=0;i<200;i++);
  SPI.transfer(0xE2);   // Command Wake up
  SPI.transfer(CFGR1);   // Command
  SPI.transfer(CFGR2);   // Command
  SPI.transfer(0x00);   // Command
  SPI.transfer(0x00);   // Command
  SPI.transfer(0x00);   // Command
  digitalWrite(cs,LOW);
}

void readVoltage()
{
//adresDegisimi();  

digitalWrite(cs,LOW);
SPI.transfer(STCVAD);
delay(20); 
digitalWrite(cs,HIGH);



byte voltRegister[18];
digitalWrite(cs,LOW);
SPI.transfer(address);
SPI.transfer(RDCV);

for(int j = 0; j<18;j++)
{
voltRegister[j] = SPI.transfer(RDCV);
 for(int z=0;z<200;z++);
}
digitalWrite(cs,HIGH);
  if(address==address1)
  {
    for(int k=0;k<12;k++)
    { if(k%2==0 && k!=12){
        pilvolt[k]=(((voltRegister[3*k/2] & 0xFF) | (voltRegister[3*k/2+1] & 0x0F) << 8)*1.5*0.001);
              }
    else if(k%2==1){
        pilvolt[k]=(((voltRegister[3*(k-1)/2+1] & 0xF0) >> 4 | (voltRegister[3*(k-1)/2+2] & 0xFF) << 4)*1.5*0.001);
        }
     else if(k==12){
          
      }
    
    }
  }
  if(address==address2)
  {
    for(int k=12;k<=20;k++)
    {
      if(k%2==0 && k!=20){
        pilvolt[k]=(((voltRegister[3*(k-12)/2] & 0xFF) | (voltRegister[3*(k-12)/2+1] & 0x0F) << 8)*1.5*0.001);
              }
      else if(k%2==1){
        pilvolt[k]=(((voltRegister[3*(k-13)/2+1] & 0xF0) >> 4 | (voltRegister[3*(k-13)/2+2] & 0xFF) << 4)*1.5*0.001);
        }
      else if(k==20){
        toplamVolt =ToplamVoltajHesap();
        maxPil = highestCell();
        minPil = lowestCell();
        send_data();
       }      
    }
  }
}


void writeReg()
{
  int i;
  digitalWrite(cs, LOW);
  for(i=0;i<200;i++);
  SPI.transfer(address);
  SPI.transfer(0x01);
  
  SPI.transfer(0x01);//0
  SPI.transfer(CFGR1);//1
  SPI.transfer(CFGR2);//2
  SPI.transfer(0x00);//3
  SPI.transfer(0x53);//4
  SPI.transfer(0xFA);//5
  
  for(int i=0;i<200;i++);
  digitalWrite(cs, HIGH);
}


void readConfig()
{
byte byteTemp; 
digitalWrite(cs, LOW);
for(int i=0;i<200;i++);
SPI.transfer(0x80);
//Serial.println(address,HEX);
SPI.transfer(0x02); //Read
String str="";
for(int i = 0; i < 6; i++)
{
    byteTemp = SPI.transfer(0x02);
    str=str + String(byteTemp,HEX) + "--";
}
   Serial.println(str);
   for(int i=0;i<200;i++);
  digitalWrite(cs, HIGH);
 for(int i=0;i<200;i++);
}


void read_data()
{ 
  char c;
  if(Serial.available()){
    while(Serial.available()){
      c = (char)Serial.read();
    }
  }
//
//      if(c = '1'){
//        dengelemeBasla = true;
//        Serial.println("###################");
//        Serial.println("Dengeleme Acik");
//        Serial.println("###################");
//      }
//      if(c = '0'){
//        dengelemeBasla = false;
//        //Serial.println("Dengeleme Kapali");
//        Serial.println("###################");
//        Serial.println("Dengeleme Kapali");
//        Serial.println("###################");
//      }
  
}





void send_data()
{    
  
     int i=0 ; 
     String str;
    // String str="(PV)";  
    str = sicaklikYazi + akimYazi +"(" + "TV" + ")" + "{" + String(toplamVolt)+ "}" +"(/" + "TV" + ")" + "(/PV)";  
     for(i=0;i<20;i++)
     {
         str=str+"(" + String(i+1) + ")" + "{" + String(pilvolt[i])+ "}" +"(/" + String(i+1) + ")"; 
        // str = str + String(pilvolt[i]) + "-";  
     }
     //sicaklikYazi = "20";

    //  str = sicaklikYazi + "-" +String(toplamVolt) + "-" + str;
     str=str;
     
     Serial.println(str); 
    // delay(1000);
}


void adresDegisimi()
{
  int i;
  if(address==address1)
  { 
    
    address=address2;
   
    
  }
  else if(address==address2)
  { 
    address=address1;
    
  }
}

float ToplamVoltajHesap()
{float toplam =0;
   for(int w=0; w<20; w++)
  { 
  toplam=toplam+pilvolt[w];
  }
   return toplam;
}

int highestCell()
{
  int  voltageHighest = 0;
  int cellNumber;
  for (int i = 0; i < Toplam_pil; i++)
  {
    if (pilvolt[i] > voltageHighest)
    {
      voltageHighest = pilvolt[i];
      cellNumber = i;
    }
  }
  return cellNumber+1;
}

int lowestCell()
{
  int voltageLowest = 99;
  int cellNumber;
  for(int i = 0; i < Toplam_pil; i++)
  {
    if (pilvolt [i] < voltageLowest)
    {
      voltageLowest = pilvolt[i];
      cellNumber = i;
    }
  }
  return cellNumber+1;
}


void balance()
{

        Serial.println("###################");
        Serial.println("Balance Acik");
        Serial.println("###################");
  
  float difference[Toplam_pil];
  CFGR1 = 0x00;
  CFGR2 = 0x00;
  byte temp;
  int x;
  int i;
  float fark[Toplam_pil+1];
 
  for(i=1; i<Toplam_pil; i++)
  {
    
    if(i!=minPil)
    {  fark[i]=pilvolt[i]-pilvolt[minPil-1];
          if(i>12 & fark[i] >= dengeleme_fark_hassas)
          {
            address=address2;
            x = i - 13; 
            temp = 0x01;
            temp = temp << x;
            CFGR1 = CFGR1 + temp;
            writeReg();
          }
  
  if(i<=8 & fark[i] >= dengeleme_fark_hassas)
        {   address=address1;
            x = i - 1; 
            temp = 0x01;
            temp = temp << x;
            CFGR1 = CFGR1 + temp;         
            writeReg();
         }
         
  if(i>8 & i<=12 & fark[i] >= dengeleme_fark_hassas)
        {   address=address1;
            x = i - 9; 
            temp = 0x01;
            temp = temp << x;
            CFGR2 = CFGR2 + temp;
            writeReg();
         }
    }
    
  }
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
   
    
    //Serial.println();
    ds.reset_search();
    delay(20); // delay(250);
    return;
  }
  
  

  if (OneWire::crc8(addr, 7) != addr[7]) {
     // Serial.println("CRC is not valid!");
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
 
  //Serial.println(karakter);
  
  //Serial.println(celsius);

    sicaklikYazi = String(celsius);
  if(celsius>60)
 {
  digitalWrite(relay,HIGH);
 }
  
 
}

void akim_fonksiyon()
{
  
   for(int j=0; j<100; j++)
 {okunanDeger = analogRead(analogIn);
 Amper = (okunanDeger - offsetCurrent)/3.4;
  
ToplamAkim = ToplamAkim + Amper;


 }
 ortalamaAkim = ToplamAkim/100;
 ToplamAkim=0;
 akimYazi = String(ortalamaAkim);
}

