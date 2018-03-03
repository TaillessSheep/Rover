#include <Servo.h>

Servo LF,LB,RF,RB;  // Left_Front, Left_Back, Right_Front, Left_Back
byte channel_1 = 2; // Forward and backward
byte channel_3 = 4; // Turning
byte channel_5 = 7; // Saftety

int c1,c3,c5;               // Read value
int LS_1, RS_1, LS_3, RS_3; // commanded speed(L/R) from ch_1 or ch_2
int LS,RS;                  // Overall leftspeed and rightspeed

int MaxSpeed=250;         // Max speed for single task(F&b or turning)
int LV, RV;               // Servo write value for L & R in microseconds
int LV_F,LV_B,RV_F,RV_B;  // LV and RV with off set for the back or front wheel

int midF=1470;  // Mid Frequency
int zLen=40;    // +- Frequency near midF

bool Switch = 0;              // The switch...
int offSet[4]={30,0,40,40};   // off set for each individual servo
                              // {LF,LB,RF,RB} respectively

//---------------------------------------------------------------------------
void setup(){
  // connect servos
  LF.attach(3);LB.attach(5);RF.attach(6);RB.attach(9);
  
  // dont want the it start moving before we give order
  LF.writeMicroseconds(midF+offSet[0]);
  LB.writeMicroseconds(midF+offSet[1]);
  RF.writeMicroseconds(midF+offSet[2]);
  RB.writeMicroseconds(midF+offSet[3]);

  // just for debugging
  Serial.begin(9600);
  }

void loop(){
  // to determinant if the switch should be on or off
  checkSwitch();

  // switch on: 
  if(Switch){
    readSignal();
    speedProcess();
    servoControl();
    }

  // switch off:  
  // set all servos to stop
  else{
    LF.writeMicroseconds(midF+offSet[0]);
    LB.writeMicroseconds(midF+offSet[1]);
    RF.writeMicroseconds(midF+offSet[2]);
    RB.writeMicroseconds(midF+offSet[3]);    
    }
  }

//----------------------------------------------------------------------------
// to determinant if the switch should be on or off
void checkSwitch(){
  // read ch_5
  c5 = pulseIn (channel_5, HIGH);

  // several conditions:
   // invalid reading from ch_5  
  bool invalid_reading = c5<100;
   // the switch was on before  and ch_5 is still in mid-position
  bool keep_it_on = Switch and (1400 < c5 and c5 < 1600);
   // the switch was off beofe  and ch_5 is still not in mid-position
  bool keep_it_off= !Switch and (1400 > c5 or c5 > 1600);

  // either of the condition above is ture, no need to change 
  // the state of the switch
  if (invalid_reading or keep_it_on or keep_it_off)return;

  // ch_5 have been switched on (it was not in mid-p)
  // then check if ch_3 is in mid for at least 600 milliseconds(0.6 second)
  else if (1400 < c5 and c5 < 1600){
    // start timer
    unsigned long int pre = millis();
    // for at least 600 milliseconds
    while(millis()-pre < 600){
      c5 = pulseIn (channel_5, HIGH);
      c3 = pulseIn (channel_3, HIGH);
      // ch_3 and ch_5 in mid-p
      bool switchon = 1400 < c5 and c5 < 1600;
      bool ch3_onMid = 1400 < c3 and c3 < 1600;
      // if either of them is off the mid-p
      // we consider this as an accident
      if (!(switchon and ch3_onMid))return;
      }
    // not an accident, set Switch on
    Switch = 1;
    }
  // ch_5 have been switch off
  else Switch = 0;
  }
//*************************************************************************
// just reading signal
void readSignal(){
  c1 = pulseIn(channel_1, HIGH);
  c3 = pulseIn(channel_3, HIGH);
  }
//*************************************************************************
// process the signal: using channel 1 as turning signal the 3 as forward/backward signal
void speedProcess(){
  // when signal is near the midF
  // reduce the sensitiveness when near the mid position
  if (c1 > midF-zLen and c1 < midF+zLen) c1 = midF;
  if (c3 > midF-zLen and c3 < midF+zLen) c3 = midF;

  // Left speed 
  LS_1 = map(c1,980, 1980, -MaxSpeed, MaxSpeed);// from ch 1
  LS_3 = map(c3,980, 1980, -MaxSpeed, MaxSpeed);// from ch 3

  // Right speed
  RS_1 = -LS_1; // ch 1: turning channel; the turning sould the opposite
  RS_3 = LS_3;  // ch 3

  // Add up the two Channels
  LS = LS_1 + LS_3;
  RS = RS_1 + RS_3;
  }
//***********************************************************************
void servoControl(){
  // From speed to write value
  LV = LS + midF;
  RV = -RS + midF;
    // right side seems overall faster that the leftside, so reduced it a bit by mapping
    // might be removed in the final program
  RV = map (RV,955,1990,1000,1930); 

  // add off set
  LV_F = LV + offSet[0];
  LV_B = LV + offSet[1];
  RV_F = RV + offSet[2];
  RV_B = RV + offSet[3];

  //servo.write
  LF.writeMicroseconds(LV_F);
  LB.writeMicroseconds(LV_B);
  RF.writeMicroseconds(RV_F);
  RB.writeMicroseconds(RV_B);    
  }
