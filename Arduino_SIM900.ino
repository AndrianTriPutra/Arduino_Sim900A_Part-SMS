#include <SoftwareSerial.h>

#define SIM900A_RxPin 10
#define SIM900A_TxPin 11
#define PinResetSIM900A 12
#define LED 13

SoftwareSerial SIM900A(SIM900A_RxPin,SIM900A_TxPin);

String  cpin              ="";
String  csq               ="";
String  cipstatus         ="";
String  respondsend       ="";
String  Reply             ="";
String  Command           ="";
String  sender_phone      ="";
String  ResponeSMS        ="";

unsigned long   currentMillis     =0;
long            previousMillis    =0;

long int time             =0;

uint8_t FI                =0;
uint8_t LI                =0;

boolean sender            =false;
boolean LEDState          ;

#define S1debug true

void setup() {//setup
  Serial.begin(9600);
  SIM900A.begin(9600);
  
  //declare pin  
  pinMode(PinResetSIM900A,OUTPUT);
  pinMode(LED,OUTPUT);
  delay(100);
  
  //warming up sim90A
  digitalWrite(PinResetSIM900A,LOW);
  delay(1000);
  digitalWrite(PinResetSIM900A,HIGH);
  delay(10000); 
  //Connection();
  //DialVoiceCall();
}//setup

void loop() {//loop
  Connection();
  ReceivedMessage();
  SendMessage();
  DeletedSMSBerkala();
}//loop

//SendCommand
void SendCommand(String command, const int timeout, boolean debug){
    
  Reply=""; 
      
  SIM900A.println(command); 
  if (command=="AT+CPIN?")
    {
     cpin="";
     if(debug){
       time = millis();   
       while( (time+timeout) > millis()){
        while(SIM900A.available()){       
          Reply += char(SIM900A.read());
         }  
        }    
       FI=0;
       LI=0;
       FI=Reply.indexOf("+CPIN:");
       LI=Reply.indexOf('Y',FI);
       cpin=Reply.substring(FI+6,LI+1);
       cpin.trim();
       if (cpin=="READY"){
         cpin="";
         cpin+="READY";
        }
       else{
         cpin="";
         cpin+="UNREADY";        
       }
      Serial.print("respon CPIN:");
      Serial.println(cpin);
     } 
    }
    
  else if(command=="AT+CSQ"){
      csq="";
      if(debug){
        time = millis();   
        while( (time+timeout) > millis()){
          while(SIM900A.available()){       
           Reply += char(SIM900A.read());
          }  
        }    
       FI=0;
       LI=0;
       FI=Reply.indexOf("+CSQ:");
       LI=Reply.indexOf(',',FI);
       csq=Reply.substring(FI+6,LI+3);
       csq.trim();
       Serial.print("response CSQ:");
       Serial.println(csq);
     } 
    } 
    
  else{  
      if(debug){
        time = millis();   
        while( (time+timeout) > millis()){
          while(SIM900A.available()){       
          Reply += char(SIM900A.read());
         }  
       }    
       Serial.println(Reply);
      } 
     } 
}//SendCommand

//CekKoneksi
void Connection(){
  SendCommand("AT+CPIN?",250,S1debug);
  SendCommand("AT+CSQ",250,S1debug);
  SendCommand("AT+CREG=2",250,S1debug);
  SendCommand("AT+CREG?",250,S1debug);  
}//CekKoneksi

//ReceivedMessage
void ReceivedMessage(){
  String      Password    ="";
              Command     ="";
              
  SendCommand("AT+CMGF=1\r",500,S1debug);//Text Mode
  SendCommand("AT+CNMI=3,3,0,0,0\r",500,S1debug);//open message manual
  SendCommand("AT+CMGR=1\r",500,S1debug);//read SMS first index
   
  FI=Reply.indexOf("GONDRIL");
  Password=Reply.substring(FI,FI+7);
  Password.trim();
  Serial.print("Key:");Serial.println(Password);
  
  if(Password=="GONDRIL"){
    
    LI=Reply.indexOf('#',FI);
    LI++;
    FI=Reply.indexOf('#',LI);
    Command=Reply.substring(LI,FI);
    Serial.print("Command:");Serial.println(Command);   

     if(Command=="LED_ON"){
      digitalWrite(LED,HIGH);  
      LEDState=true;        
     }
     
     else if(Command=="LED_OFF"){
      digitalWrite(LED,LOW);
      LEDState=false;          
     }
     
     //dapetin nomor hp pengirim
    FI=Reply.indexOf("+CMGR:");
    FI++;
    LI=Reply.indexOf('+',FI);
    FI=Reply.indexOf(',',LI);
    sender_phone=Reply.substring(LI,FI-1);
    sender_phone.trim();
    Serial.print("sender_phone:");Serial.println(sender_phone);

    if (sender_phone.length()>10){
      sender=true;
      //Serial.println("GET PHONE SEND NUMBER");
      SendCommand("AT+CMGD=1,4\r",500,S1debug);//Deleted all message
    }
    else{
      sender=false;     
    }
      
  }
}//ReceivedMessage

//SendMessage
void SendMessage(){
  if (sender){
    if (Command=="LED_ON"||Command=="LED_OFF"){ 
      ResponeSMS="";       
      ResponeSMS+="SN:";ResponeSMS.concat("BIR001");
      ResponeSMS+="\nSIM:";ResponeSMS.concat(cpin);
      ResponeSMS+="\nSQ :";ResponeSMS.concat(csq);  

     if(LEDState==true){
       ResponeSMS+="\nLED:";ResponeSMS.concat("ON"); 
      }
     else{
       ResponeSMS+="\nLED :";ResponeSMS.concat("OFF"); 
      }

     SendCommand("AT+CMGS=\""+sender_phone+"\"",1000,S1debug);
     SendCommand(ResponeSMS,500,S1debug);
     SendCommand(String ((char)26),2000,S1debug);         
    }
    sender=false;
  }
}//SendMessage

//DeletedSMSBerkala
void DeletedSMSBerkala(){
  currentMillis = millis();
  if (currentMillis - previousMillis > 300000) {//5 menit
    Serial.println("DeletedSMSBerkala");
    previousMillis = currentMillis;
  }  
}//DeletedSMSBerkala

//DialVoiceCall
void DialVoiceCall(){
   SendCommand("ATD+62xxxxxxxxxxxx;",2000,S1debug);// "ATD+628xxxxxxxx"dial the number
   delay(1000); 
}//DialVoiceCall
