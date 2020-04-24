#include <Servo.h>
#define NODBG
//commands
#define RESR0 '0'
#define RESR1 '1'
#define RESR2 '2'
#define RESLED '3'
#define RESS0 '4' 
#define RESS1 '5'
#define REST0 '6' //COMMAND IS TEMPERATURE AS CELCIUS 
#define REST1 '7'
#define REST2 '8'
#define RESPHO '9'
#define COMPASSIVE '0'
#define COMACTIVE '1'


//IOCONFIG
#define SERVO0PIN 7
#define SERVO1PIN 6

#define T0PIN A0
#define T1PIN A1
#define T2PIN A2

#define T0R 9940.0  //pullup resistor value, psu side first IS NOT USED BECAUSE IT AFFECTS MUCH LESS THAN SENSOR CAL
#define T1R 1010.0
#define T2R 9950.0
#define T_0INV 0.0033540164346805303
#define BINV 0.00025316455696202533

#define RELAY0PIN 40
#define RELAY1PIN 42
#define RELAY2PIN 44
#define LEDPIN 46

#define PHOTOPIN 52

#define OKPIN LED_BUILTIN  //active when temperature is ok

#define RACTIVE LOW   //relay active
#define RPASSIVE HIGH //relay passive
#define SERVOSTEP 15 //servo 1 degree step in milliseconds

//THERMAL
#define TRIGGERH 0.5  //Thermal compatibility 
#define TRIGGERL -0.5
#define OKH 2.0
#define OKL -2.0

Servo s0,s1;
bool ron[3]={false};
int T[3]={35,40,45};

void setpos(Servo s,int pos);
void reportT();
void exec(char resource, unsigned char command);
float getT(int pin);
void controlT();
void photo();

void setup() {
  s0.attach(SERVO0PIN);
  s1.attach(SERVO1PIN);
  pinMode(LEDPIN,OUTPUT);
  pinMode(RELAY0PIN,OUTPUT);
  pinMode(RELAY1PIN,OUTPUT);
  pinMode(RELAY2PIN,OUTPUT);
  digitalWrite(LEDPIN,RPASSIVE);
  digitalWrite(RELAY0PIN,RPASSIVE);
  digitalWrite(RELAY1PIN,RPASSIVE);
  digitalWrite(RELAY2PIN,RPASSIVE);
  pinMode(OKPIN,OUTPUT);
  pinMode(T0PIN,INPUT);
  pinMode(T1PIN,INPUT);
  pinMode(T2PIN,INPUT);
  Serial.begin(9600);
}


void loop() {
  char res,com;
  while(!Serial.available())controlT();
  res=Serial.read();
  while(!Serial.available())controlT();
  com=Serial.read();
  exec(res,com);
  #ifdef DBG
  reportT();
  #endif
}

void setpos(Servo s,int pos){
  int p=s.read();
  int inc=(pos>p)?1:-1;
  if(p==pos)return;
  for(int i=p+inc;i!=pos;i+=inc){
    s.write(i);
    delay(SERVOSTEP);
  }
}

float getT(int pin){
  int r=analogRead(pin);
  float val=((float)r)/(1024-r);
  val=log(val);
  return 1/(T_0INV+val*BINV)-273.15;
}

void exec(char resource, unsigned char command){
  uint8_t digital= (command==COMPASSIVE) ? RPASSIVE : RACTIVE;
  switch(resource){
    case RESR0:
      digitalWrite(RELAY0PIN,digital);
      break;
    case RESR1:
      digitalWrite(RELAY1PIN,digital);
      break;
    case RESR2:
      digitalWrite(RELAY2PIN,digital);
      break;
    case RESLED:
      digitalWrite(LEDPIN,digital);
      break;
    case RESS0:
      setpos(s0,command);
      break;
    case RESS1:
      setpos(s1,command);
      break; 
    case REST0:
      T[0]=(int)command;
      break;
    case REST1:
      T[1]=(int)command;
      break;
    case REST2:
      T[2]=(int)command;
      break;
    case RESPHO:
      photo();
      break;
  }
}

void photo(){
  digitalWrite(PHOTOPIN,LOW);
  pinMode(PHOTOPIN,OUTPUT);
  delay(200);
  pinMode(PHOTOPIN,INPUT);
}

void reportT(){
  Serial.print(" Read:");
  Serial.print(getT(T0PIN));
  Serial.print(" ");
  Serial.print(getT(T1PIN));
  Serial.print(" ");
  Serial.print(getT(T2PIN));
  Serial.print(" Targets:");
  Serial.print(T[0]);
  Serial.print(" ");
  Serial.print(T[1]);
  Serial.print(" ");
  Serial.println(T[2]);
  
}

void controlT(){
  float dt[3];
  bool good=true;
  dt[0]=getT(T0PIN)-T[0];
  dt[1]=getT(T1PIN)-T[1];
  dt[2]=getT(T2PIN)-T[2];
  for(int i=0;i<3;i++){
    if(dt[i]>TRIGGERH)ron[i]=false;
    if(dt[i]<TRIGGERL)ron[i]=true;
    good=good * (dt[i]<OKH) *  (dt[i]>OKL);
  }

  digitalWrite(OKPIN,good);

  unsigned long order=(millis()/1000)%3;
  if(!ron[0] || order!=0)digitalWrite(RELAY0PIN,RPASSIVE);
  if(!ron[1] || order!=1)digitalWrite(RELAY1PIN,RPASSIVE);
  if(!ron[2] || order!=2)digitalWrite(RELAY2PIN,RPASSIVE);
  delay(40);

  if(ron[0] && order==0)digitalWrite(RELAY0PIN,RACTIVE);
  if(ron[1] && order==1)digitalWrite(RELAY1PIN,RACTIVE);
  if(ron[2] && order==2)digitalWrite(RELAY2PIN,RACTIVE);
   
    #ifdef DBG 
      Serial.print("Order:");
      Serial.print(order);
      Serial.print(" Active:");
      for(int i=0;i<3;i++){
        Serial.print(ron[i]);
        Serial.print(" ");
      }
      Serial.print("OK:");
      Serial.print(good);
      reportT();
    #endif
}
