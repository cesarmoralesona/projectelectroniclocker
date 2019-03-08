//declare pin's, states, orders, variables and times
////////////////////////////////////////////////////
  /****************/
 /*declare pin's*/
/***************/
#define Green_led 10
#define Red_led 11
#define Lock 12
  /*********/
 /*states*/
/********/
enum states{OFF,ON,BUSSY,FREE,LOWBATTERY,RIGHT_PIN,WRONG_PIN};
/*********/
 /*ORDERS*/
/********/
#define Close 'C'
#define Open 'A'
#define Auto_open 'D'
  /***********/
 /*variables*/
/*********/
int State_led=OFF;
int State_box=FREE;
  /*******/
 /*times*/
/*******/
int Last_change=0;
int Time_change=0;
unsigned long  Time=0;
float Time_BUSSY;
float Time_FREE;
float Time_auto=7200000;
////////////////////////////////////////////////
//char's and int's for comCode, Box_FREE, Box_BUSSY 
static char pin[5];
static char intropin[5];
static char pinHACK[5]={"6666"};
static char keyORDER;
unsigned int index=0;
unsigned int attemp=3;
unsigned int counterprint;
///////////////////////////////////////////////
void setup() {
  pinMode(Red_led, OUTPUT);
  pinMode(Green_led, OUTPUT);
  pinMode(Lock, OUTPUT);
  Serial.begin(9600);
  keyboardBegin();
}
void loop() {
  Time=millis();
  Batterymannager(50);
  switch(State_box){
    case FREE:
     Box_FREE();
     Time_FREE=Time;
    break;
    case BUSSY:
     Time_BUSSY=Time-Time_FREE;
     if(Time_BUSSY<=Time_auto) normal_OPEN();
     else auto_OPEN();
    break;
    case LOWBATTERY:
    Serial.println("bateria baja");
    BlinkLED_ON_OFF(Green_led,100,4900);
    break;
  } 
}
void keyboardBegin(void)
{
  for(int F=2;F<=5;F++)
  {
  pinMode(F,OUTPUT);
  digitalWrite(F,HIGH);//PullUp
  }
  for(int C=6;C<=9;C++)
  {
   pinMode(C,INPUT);
   digitalWrite(C,HIGH);//PullUp
  }
}
char keyboardRead(void)
{
  char key;
  char keys[4][4]=
  {
   {'1','2','3','F'},
   {'4','5','6','E'},
   {'7','8','9','D'},
   {'A','0','B','C'}
  };
   for(int C=6;C<=9;C++)
   {
     for(int F=2;F<=5;F++)
     {
      digitalWrite(F,LOW);
      if(digitalRead(C)==LOW&&digitalRead(F)==LOW){
         unsigned int c=C-6,f=F-2;
         key=keys[c][f];
       }
      digitalWrite(F,HIGH);
     }
    digitalWrite(C,HIGH);
   }
  antirebote(key);
  return key;
}
unsigned int antirebote(char key)
{
  int counter = 0;
  char Actualkey=key;
  static char Prekey;
  do{
    if (Actualkey!=Prekey) counter=0, Prekey=key; 
    else counter++;
    delay(1);
  }while (counter<225);
  return Actualkey;
}
unsigned int comCode(char *code,char *codeintro)
{
  while((*code!=0) && (*codeintro!=0))
  {
    if(*code++!=*codeintro++)return WRONG_PIN;
  }
  return RIGHT_PIN; 
}
unsigned int Batterymannager(int mPorcent)
{
 static unsigned int Prestate_box;
 int Battery=analogRead(A0)*(100.0/1024.0);
 if(Battery<mPorcent&&State_box!=LOWBATTERY)Prestate_box=State_box;
 if(State_box==FREE&&Battery<mPorcent)State_box=LOWBATTERY;
 if(State_box==BUSSY&&Battery<mPorcent)State_box=BUSSY;
 if(Battery>=mPorcent&&State_box==LOWBATTERY) State_box=Prestate_box;
 return State_box;
}
unsigned int BlinkLED_ON_OFF(unsigned int Led,unsigned int Time_ON,unsigned int Time_OFF)
{
  Time_change=Time-Last_change;
  if((Time_change>=Time_OFF)&&(State_led==OFF)){
    digitalWrite(Led,HIGH);
    Last_change=Time;
    State_led=ON;
    return State_led;
   }
  if((Time_change>=Time_ON)&&(State_led==ON)){
    digitalWrite(Led,LOW);
    Last_change=Time;
    State_led=OFF;
    return State_led;
   } 
}
unsigned int BlinkdoubleLED_ON_OFF(unsigned int Led,unsigned int Led2,unsigned int Time_ON,unsigned int Time_OFF)
{
  Time_change=Time-Last_change;
  if((Time_change>=Time_OFF)&&(State_led==OFF)){
    digitalWrite(Led,HIGH);
    digitalWrite(Led2,HIGH);
    Last_change=Time;
    State_led=ON;
    return State_led;
   }
  if((Time_change>=Time_ON)&&(State_led==ON)){
    digitalWrite(Led,LOW);
    digitalWrite(Led2,LOW);
    Last_change=Time;
    State_led=OFF;
    return State_led;
   } 
}
char Recognise_order(char order,char key_order)
{
  if(key_order==order){
     return key_order;
  }
  else return 0;
}
void Box_FREE(void)
{
  counterprint++;
  if(counterprint<=1)Serial.println("taquilla libre");
  BlinkLED_ON_OFF(Green_led,100,900);
  char key=keyboardRead();
  if(key&&index!=4){
   Serial.println(key);
   pin[index++]=key;
  }
  if((key)&&(index==4)&&(keyORDER!=Close)){ 
    keyORDER=Recognise_order(Close,key);
    if(keyORDER==Close){
      Serial.println(keyORDER);
      digitalWrite(Lock,HIGH);
      digitalWrite(Green_led,LOW);
      State_box=BUSSY, State_led=OFF, index=0; counterprint=0;
    }
  }
}
void normal_OPEN(void)
{
  counterprint++;
  if(counterprint<=1)Serial.println("taquilla ocupada");
  BlinkLED_ON_OFF(Red_led,100,900);
  char key=keyboardRead();
  if(key&&index!=4){
   Serial.println(key);
   intropin[index++]=key;
  }
  if(index==4&&attemp!=0){
    if(strcmp(pinHACK,intropin)==RIGHT_PIN)HACK();
    if(comCode(pin,intropin)==RIGHT_PIN){
      digitalWrite(Green_led,HIGH);
      keyORDER=Recognise_order(Open,key);
      if(keyORDER==Open){
        Serial.println(keyORDER);
        digitalWrite(Lock,LOW);
        digitalWrite(Red_led,LOW);
        State_box=FREE, State_led=OFF, index=0, counterprint=0;
      }     
    }
    else{
     if(attemp!=1)Serial.println("repeat");
     attemp--, index=0;
    }
  }
  if(attemp==0){
    unsigned  int Time_seg=10;
    Time_seg=Time_seg*1000;
    digitalWrite(Red_led,HIGH);
    delay(Time_seg);
    digitalWrite(Red_led,LOW);
    attemp=3, index=0;
    Serial.println("tiene 3 intentos nuevos");
  }
}
////////HACK//////////
void HACK(void)
{
  digitalWrite(Lock,LOW);
  digitalWrite(Red_led,LOW);
  State_box=FREE, State_led=OFF, index=0, counterprint=0; 
}
void auto_OPEN(void){
  BlinkdoubleLED_ON_OFF(Red_led,Green_led,500,500);
  char key=keyboardRead();
  if(key){
    Serial.println(key);
    keyORDER=Recognise_order(Auto_open,key);
    if(keyORDER==Auto_open){
      digitalWrite(Lock,LOW);
      digitalWrite(Red_led,LOW);
      State_box=FREE, State_led=OFF, index=0, counterprint=0;
    }
  }
}
