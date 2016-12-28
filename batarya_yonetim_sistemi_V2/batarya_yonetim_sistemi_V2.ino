#include <SPI.h>

#define WRCFG 0x01 //Write Configuration Registers
#define RDCFG 0x02 // Read config
#define RDCV 0x04 // Read cells
#define STCVAD 0x10 // Start all A/D's - poll status
#define RDFLG 0x06 //Read Flags
#define RDTMP 0x08 //Read Temperatures
#define STCDC 0x60 //A/D converter and poll Status
#define STOWAD 0x20 //Staret Test - poll status
#define STTMPAD 0x30// Temperature Reading - ALL

#define cs 53   //  Uno - 10 Mega 53
#define mosi 51 //  Uno - 11 Mega 51
#define miso 50 //  Uno - 12 Mega 50
#define sck 52 //   Uno - 13 Mega 52
#define Toplam_pil 20 // Toplam pil sayısı
#define dengeleme_fark_hassas 0.03 // 
unsigned char address;
unsigned char address1=0x80;
unsigned char address2=0x81;

byte CFGR1 = 0;
byte CFGR2 = 0;
float pilvolt[20];
float toplamVolt;
int maxPil,minPil;
boolean dengelemeBasla=false;
int k=0;
void setup()
{
pinMode(cs,OUTPUT);
pinMode(mosi,OUTPUT);
pinMode(miso,INPUT);
pinMode(sck,OUTPUT);

digitalWrite(cs, HIGH);
address = address2;
SPI.setBitOrder(MSBFIRST);
SPI.setDataMode(SPI_MODE3);
SPI.setClockDivider(SPI_CLOCK_DIV16);
SPI.begin();
Serial.begin(9600);

}

void loop(){
  adresDegisimi();
  writeReg();
  readConfig();
  //readVoltage();
  if(dengelemeBasla==true)
    balance();
  else
  {
   CFGR1 = 0;
   CFGR2 = 0;
  }
  
}


void readVoltage()
{
//adresDegisimi();  
writeReg();
readConfig();
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
}
digitalWrite(cs,HIGH);
  if(address==address1)
  {
    for( k=0;k<12;k++)
    { 
      if(k%2==0 && k!=12)
      {
        pilvolt[k]=(((voltRegister[3*k/2] & 0xFF) | (voltRegister[3*k/2+1] & 0x0F) << 8)*1.5*0.001);
      }
      else 
      {
        pilvolt[k]=(((voltRegister[3*(k-1)/2+1] & 0xF0) >> 4 | (voltRegister[3*(k-1)/2+2] & 0xFF) << 4)*1.5*0.001);
       }
    
    
    }
  }
  if(address==address2)
  {
    for(k=12;k<=20;k++)
    {
      if(k%2==0 && k!=20){
        pilvolt[k]=(((voltRegister[3*(k-12)/2] & 0xFF) | (voltRegister[3*(k-12)/2+1] & 0x0F) << 8)*1.5*0.001);
              }
      else if(k%2==1){
        pilvolt[k]=(((voltRegister[3*(k-13)/2+1] & 0xF0) >> 4 | (voltRegister[3*(k-13)/2+2] & 0xFF) << 4)*1.5*0.001);
        }
      else if(k==20){
        toplamVolt = ToplamVoltajHesap();
        maxPil = highestCell();
        minPil = lowestCell();
        send_data();
       }      
    }
  }
}

void writeReg()
{
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
     for(i=1;i<21;i++)
     {
         str=str+"(" + String(i) + ")" + "{" + String(pilvolt[i-1])+ "}" +"(/" + String(i) + ")"; 
           
     }
     
    // str=str+"(" + String(toplamVolt) +")" + "(/PV)";
     Serial.println(str); 
}


void adresDegisimi()
{
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

void readConfig()
{byte byteTemp;
  digitalWrite(cs, LOW);
  SPI.transfer(address);
  SPI.transfer(RDCFG);//Read
  Serial.println(address,HEX);
  for(int i = 0; i < 6; i++)
  {
  byteTemp = SPI.transfer(RDCFG);
  Serial.println(byteTemp, HEX);
  }
  digitalWrite(10, HIGH);

}

