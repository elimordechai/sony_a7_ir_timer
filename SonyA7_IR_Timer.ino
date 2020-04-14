
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 clock;



#include <IRremote.h>
#include <ErriezLCDKeypadShield.h>

LCDKeypadShield shield;
IRsend irsend;

enum mode{timeMode=0,dateMode=1,timerMode=2,elapsMode=3};
enum setupMode {hourMode=0,minuteMode=1,secondMode=2 , yearMode=3 , monthMode=4 , dayMode=5};
int dispmode=0;
int setupmode=0;

char lineDisp0[17]="";
char lineDisp1[17]="";
DateTime tt;
uint8_t frames=1;
DateTime strt;
bool timer_started = false;

void setup() {
    //pinMode(LED, OUTPUT);
   //pinMode(isObstaclePin, INPUT);
    Serial.begin(115200);
    //Wire.begin();
    Serial.println(clock.begin());
    clock.adjust(DateTime(F(__DATE__),F(__TIME__)));

    clock.clearAlarm(1);
    clock.clearAlarm(2);
    clock.disableAlarm(1);
    clock.disableAlarm(2);
    clock.writeSqwPinMode(DS3231_OFF);
    DateTime dt;
    dt = clock.now();
    if (clock.lostPower()){
      clock.adjust(DateTime(2020, 4, 7, 12, 30, 30)); //starting date and time
    }
    shield.backlightOn();
    shield.print(F("Sony A7 II Timer"));
    // Backlight control
    for (uint8_t i = 0; i < 3; i++) {
        // Turn backlight off
        shield.backlightOff();
        delay(200);
        // Turn backlight on
        shield.backlightOn();
        delay(200);
    }
    Serial.println(F("Restart"));
}

void trigger(){
  for (int i=0; i<3; i++){ 
      irsend.sendSony(0xB4B8F, 20); // shutter 
      delay(40);

  }

}
void display_time(){
  DateTime dt;
  dt = clock.now();
  shield.setCursor(0, 1);
  char st[]="hh:mm:ss        ";
  dt.toString(st);
  shield.print(st);
}
void display_date(){
  DateTime dt;
  dt = clock.now();
  shield.setCursor(0, 1);
  char st[]="DD-MM-YYYY      ";
  dt.toString(st);
  shield.print(st);
}
void calculate_timer(){
  DateTime t;
  DateTime dt;
  t= clock.now(); 
  dt= DateTime(tt.unixtime() - (t.unixtime() - strt.unixtime()));
  display_timer(dt);
}
void display_timer(DateTime tt){
  shield.setCursor(0, 1);
  char t[] = "hh:mm:ss        ";
  tt.toString(t);
  shield.print(t);
  shield.setCursor(9,1);
  shield.print("Fr=");
  shield.print(frames);
}
void display_temp()
{
  shield.setCursor(10, 0);
  shield.print(clock.getTemperature());
   shield.print("C");
}
void display_Options()
{
  shield.setCursor(0, 1);
  shield.print("F=A E=A FT=1 S=N");
}
void displayMenu(int m ){
  
  if  (m==timeMode){
      if (strcmp(lineDisp0,"TIME            ")!=0){
         shield.setCursor(0, 0);
         shield.print(F("TIME            "));
         strcpy(lineDisp0,"TIME            \0");
      }
      display_time();
  }
  if (m==dateMode){
     if (strcmp(lineDisp0,"DATE            ")!=0){
        shield.setCursor(0, 0);
        shield.print(F("DATE            "));
        strcpy(lineDisp0,"DATE            \0");
     }
     display_date();
  }
  if (m==timerMode){
     if (strcmp(lineDisp0,"TIMER           ")!=0){
        shield.setCursor(0, 0);
        shield.print(F("TIMER           "));
        strcpy(lineDisp0,"TIMER           \0");
     }
     DateTime dt;
     if (timer_started) {
        calculate_timer();
     }else {
      display_timer(tt);
     }
  }
  if (m==elapsMode){
     if (strcmp(lineDisp0,"Options         ")!=0){
        shield.setCursor(0, 0);
        shield.print(F("Options         "));
        strcpy(lineDisp0,"Options         ");
     }
     display_Options();
  }        
  display_temp();
}
void setSetupCursor(int row , int col , bool showCursor){
  shield.setCursor(row,col);
  if (showCursor) {
    shield.cursor();
  }else{
    shield.noCursor();
  }
}
void arm_alarm(){
  Serial.println(tt.unixtime());
  strt = clock.now();
  
  DateTime at = strt + TimeSpan(0, tt.hour() , tt.minute() , tt.second());
  Serial.println(strt.unixtime());
  Serial.println(at.unixtime());
  clock.setAlarm1(at, DS3231_A1_Hour);
}

int step_element(int element,bool incr ,int upper, int lower){
  if (incr) {
      element++;
      if (element>upper) element=lower;
    }else{
      element--;
      if (element<lower) element=upper;
    }
    Serial.println(element);
    return element;
}
bool leap_year(int year)
{
  if (year % 4 == 0) {
        if (year % 100 == 0) {
            if (year % 400 == 0) {
                return true;
            } else {
                return false;
            }
        } else {
             return true;
        }
    } else {
        return false;
    }
}
void step_time(bool incr , uint8_t smode )
{
  DateTime t;
  t = clock.now();
  if (smode==minuteMode){
    t=DateTime(t.year(), t.month() ,t.day(),t.hour(),step_element(t.minute(),incr,59,0),t.second());
  }else if (smode==secondMode){
    t=DateTime(t.year(), t.month() ,t.day(),t.hour(),t.minute(),step_element(t.second(),incr,59,0));
  }else if (smode==hourMode){
    t=DateTime(t.year(), t.month() ,t.day(),step_element(t.hour(),incr,23,0),t.minute(),t.second());
  }else if (smode==yearMode){
    t=DateTime(step_element(t.year(),incr,2100,2000), t.month() ,t.day(),t.hour(),t.minute(),t.second());
  }else if (smode==monthMode){
    t=DateTime(t.year(), step_element(t.month(),incr,12,1) ,t.day(),t.hour(),t.minute(),t.second());
  }else if (smode==dayMode){
    int dupper=30;
    if ((t.month()==1) || (t.month()==3) || (t.month()==5)|| (t.month()==7)|| (t.month()==8)|| (t.month()==10) || (t.month()==12)) {
      dupper=31;
    }else if (t.month()==2) {
        if (leap_year(t.year())){
            dupper=29;
        }else{
          dupper=28;
        }
    }
    t=DateTime(t.year(), t.month() ,step_element(t.day(),incr,dupper,1),t.hour(),t.minute(),t.second());
  }
  clock.adjust(t);
}
bool keyunlock = true;
void loop() {
  if ((dispmode==timerMode) && (timer_started)) {
    if (clock.alarmFired(1)) {
       Serial.println("alarm triggered");
       trigger();
       if (frames==1){
          clock.clearAlarm(1);
          frames=1;
          //clock.isAlarm1(false);
          timer_started=false;
       }else{
          frames=frames-1;
          clock.clearAlarm(1);
          arm_alarm();
       }
    }
  }        
  if ((shield.getButtons() == ButtonUp) && (keyunlock)){
    if (setupmode==0){
      dispmode++;
      delay(200);
    }else if (setupmode==1){
      if (dispmode==timeMode){
          display_time();
          step_time(true,hourMode);
          display_time();
      }else if (dispmode==dateMode){
          step_time(true,dayMode);
          display_date();
      }else if (dispmode==timerMode){
          int hour = tt.hour()+1;
          if (hour>23) hour= 0;
          tt=DateTime(tt.year(), tt.month() ,tt.day(),hour,tt.minute(),tt.second());
          display_timer(tt);
      }
    }else if (setupmode==2){
      if (dispmode==timeMode){
          display_time();
          step_time(true,minuteMode);
          display_time();
      }else if (dispmode==dateMode){
          step_time(true,monthMode);
          display_date();
      }else if (dispmode==timerMode){
          int minute = tt.minute()+1;
          if (minute>59) minute=0;
          tt=DateTime(tt.year(), tt.month() ,tt.day(),tt.hour(),minute,tt.second());
          display_timer(tt);
      }
    }else if (setupmode==3){
      if (dispmode==timeMode){
          display_time();
          step_time(true,secondMode);
          display_time();
      }else if (dispmode==dateMode){
          step_time(true,yearMode);
          display_date();
      }else if (dispmode==timerMode){
          int second=tt.second()+1;
          if (second>59) second=0;
          tt=DateTime(tt.year(), tt.month() ,tt.day(),tt.hour(),tt.minute(),second);
          display_timer(tt);
      }
    }else if (setupmode==4){
      if (dispmode==timerMode){
          if (frames<255) frames = frames + 1;
          display_timer(tt);
      }
    }
    delay(200);
    keyunlock=false;
  }else if ((shield.getButtons() == ButtonDown)&& (keyunlock)){
    if (setupmode==0){
      dispmode--;
      Serial.println(dispmode);
      delay(200);
    }else if (setupmode==1){
      if (dispmode==timeMode){
          display_time();
          step_time(false,hourMode);
          display_time();
      }else if (dispmode==dateMode){
          step_time(false,dayMode);
          display_date();
      }else if (dispmode==timerMode){
          int hour = tt.hour()-1;
          if (hour<0) hour= 23;
          tt=DateTime(tt.year(), tt.month() ,tt.day(),hour,tt.minute(),tt.second());
          display_timer(tt);
      }
    }else if (setupmode==2){
      if (dispmode==timeMode){
          display_time();
          step_time(false,minuteMode);
          display_time();
      }else if (dispmode==dateMode){
          step_time(false,monthMode);
          display_date();
      }else if (dispmode==timerMode){
          int minute = tt.minute()-1;
          if (minute<0) minute=59;
          tt=DateTime(tt.year(), tt.month() ,tt.day(),tt.hour(),minute,tt.second());
          display_timer(tt);
      }
    }else if (setupmode==3){
      if (dispmode==timeMode){
          display_time();
          step_time(false,secondMode);
          display_time();
      }else if (dispmode==dateMode){
          step_time(false,yearMode);       
          display_date();
      }else if (dispmode==timerMode){
          int second=tt.second()-1;
          if (second<0) second=59;
          tt=DateTime(tt.year(), tt.month() ,tt.day(),tt.hour(),tt.minute(),second);
          display_timer(tt);
      }
    }else if (setupmode==4){
      if (dispmode==timerMode){
          if (frames>1) frames = frames-1;
          display_timer(tt);
      }
    }
    delay(200);
    keyunlock=false;
  }else if ((shield.getButtons() == ButtonLeft)&& (keyunlock)){
    setupmode--;
    delay(200);
    keyunlock=false;
  }else if ((shield.getButtons() == ButtonRight)&& (keyunlock)){
    setupmode++;
    delay(200);
    keyunlock=false;
  }else if (shield.getButtons() == ButtonSelect){
    if (dispmode==timeMode) trigger();
    if (dispmode==timerMode) {
      timer_started=true;
      arm_alarm();
    }
    keyunlock=true;
  }else if (shield.getButtons() == ButtonNone){
    
    keyunlock=true;
  }
  if (dispmode>elapsMode) dispmode=timeMode;
  if (dispmode<timeMode) dispmode=elapsMode;
  if (setupmode == 0 ) {
      displayMenu(dispmode);
  }else {
    if (setupmode<4) {
      setSetupCursor((setupmode-1) * 3,1,true);
    }else {
      setSetupCursor(12,1,true);
    }
  }
  delay(100);
}
