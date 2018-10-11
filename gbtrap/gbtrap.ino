/***************************************************
 * Coded by TragicManner
 * version 1.3 - Now with Coroutines/LED support!
 * utilizes the Couroutines library by renaudbedard
 * https://github.com/renaudbedard/littlebits-arduino/tree/master/Libraries/Coroutines
 ***************************************************/

#include <Coroutines.h>
#include <Servo.h>

// --trap variables
int trapState = 0; // trap states are 0 for idle, 1 for pedal press waiting for doors open, 2 for doors open and audio/LED show active, 3 for doors open and audio/LED show done, 4 for doors open and waiting to close
const int trapIdle = 0;
const int trapOpening = 1;
const int trapOpenShow = 2;
const int trapOpenIdle = 3;
const int trapClosing = 4;

// --door variables: servo1--  SERVO OPEN AND CLOSED VALUES NEED TO BE ADJUSTED FOR YOUR SERVOS!
Servo servo1;
const int servo1Pin = 9;
const int servo1OpenPosition = 68;
const int servo1ClosedPosition = 23;

// --door variables: servo_2--
Servo servo2;
const int servo2Pin = 10;
const int servo2OpenPosition = 15;
const int servo2ClosedPosition = 63;

// --door variables: state--
unsigned long doorTimer = 0;
const int doorWaitTime = 300; // If your doors don't have enough time to close, make this longer
const unsigned long doorOpenTime = 15000; // This is how long the audio and LEDs are active after the doors are first opened. This time is also used to know when to auto-close doors.
const bool autoCloseDoor = true; // If this is true, then after doorOpenTime has elapsed after the doors open, the doors will automatically close.

// --Pedal variables--
const int pedalPin = 2;
// pedalState tracks whether or not the pedal is on or off
int pedalState = 0;
int latestPedal = 0;

// -- General LED variables--
int PWMLEDs[4] = {3,5,6,11}; // the PWM pins that you want to run your LEDs with
int nonPWMLEDs[2] = {7,8}; // the pins that you want to run non-PWM LEDs with -- These are LEDs that just turn on and off

// --PWM LED Variables--
int pwmTime = 12000; // How long in ms the PWM LEDs will flash for until they are turned off
int brightness[4] = {0, 255, 100, 255}; // The starting brightness for each PWM pin. 0 is off, 255 is full brightness
int fadeAmount[4] = {100,50,50,50}; // The higher this amount, the faster the LED will fade between on and off
int fadeStep = 30; // This controls how smooth the LED fades will be


// -- Coroutine Stuff --
Coroutines<3> coroutines;


void setup() {
  //Serial.begin(9600);
  // Set up the pin that watches for the pedal button
  pinMode(pedalPin, INPUT_PULLUP);
  for(int i = 0; i < sizeof(PWMLEDs)/sizeof(int); i++){
    pinMode(PWMLEDs[i], OUTPUT);
  }
  for(int k = 0; k < sizeof(nonPWMLEDs)/sizeof(int); k++){
    pinMode(nonPWMLEDs[k], OUTPUT);
  }
  coroutines.start(CloseDoors);
}

void loop() {
  // First, read the state of the pedal
  pedalState = digitalRead(pedalPin);

  bool startPedalStuff = false;

  if(latestPedal != pedalState){
    latestPedal = pedalState;

    startPedalStuff = true;
  }

  // Update the LEDs and coroutines
  coroutines.update();

  // This block detects if a button has been newly pressed (button down) and the trap is in a state to accept pedal presses
  if (startPedalStuff && trapState != trapOpening && trapState != trapClosing) {

    if (trapState == trapIdle) {
      doorTimer = millis() + doorOpenTime;
      coroutines.start(OpenDoors);
    }
    else if ((trapState == trapOpenShow || trapState == trapOpenIdle )) {
      coroutines.start(CloseDoors);
    }
  }
  // This checks if the doors have been open for a certain amount of time
  else if(trapState == trapOpenShow && millis() >= doorTimer) {
    trapState = trapOpenIdle;
    if(autoCloseDoor){
      coroutines.start(CloseDoors);
    }
  }
}

void OpenDoors(COROUTINE_CONTEXT(coroutine)) {
  BEGIN_COROUTINE;
  trapState = trapOpening;
  AttachServos();
  MoveServos(servo1OpenPosition, servo2OpenPosition);
  coroutine.wait(doorWaitTime);
  COROUTINE_YIELD;
  TurnOnNonPWMLEDs();
  DetachServos();
  trapState = trapOpenShow;
  coroutines.start(FadeLED);
  END_COROUTINE;
}

void CloseDoors(COROUTINE_CONTEXT(coroutine)) {
  BEGIN_COROUTINE;
  trapState = trapClosing;
  AttachServos();
  MoveServos(servo1ClosedPosition, servo2ClosedPosition);
  coroutine.wait(doorWaitTime);
  COROUTINE_YIELD;
  TurnOffNonPWMLEDs();
  DetachServos();
  trapState = trapIdle;
  //coroutines.start(FadeLED);
  END_COROUTINE;
}

void FadeLED(COROUTINE_CONTEXT(coroutine)) {
  COROUTINE_LOCAL(unsigned long, finishTime);
  
  BEGIN_COROUTINE;

  finishTime = millis() + pwmTime;
  
  while(finishTime > millis() && trapState == trapOpenShow) {
    for (int i = 0; i < sizeof(PWMLEDs)/sizeof(int); i++)
    {
      analogWrite(PWMLEDs[i], brightness[i]);
      
      // change the brightness for next time through the loop:
      brightness[i] = brightness[i] + fadeAmount[i];
  
      if (brightness[i] > 255){
        brightness[i] = 255;
      }
  
      if (brightness[i] < 0) {
        brightness[i] = 0;
      }
    
      // reverse the direction of the fading at the ends of the fade:
      if (brightness[i] <= 0 || brightness[i] >= 255) {
        fadeAmount[i] = -fadeAmount[i];
      }
    }
    coroutine.wait(fadeStep);
    COROUTINE_YIELD;
  }

  TurnOffPWMLEDs();
  END_COROUTINE;
}

void TurnOnPWMLEDs(){
  for(int i = 0; i < sizeof(PWMLEDs)/sizeof(int); i++)
  {
    analogWrite(PWMLEDs[i], 255);
  }
}

void TurnOffPWMLEDs(){
  for(int i = 0; i < sizeof(PWMLEDs)/sizeof(int); i++)
  {
    analogWrite(PWMLEDs[i], 0);
  }
}

void TurnOnNonPWMLEDs(){
  for(int i = 0; i < sizeof(nonPWMLEDs)/sizeof(int); i++)
  {
    digitalWrite(nonPWMLEDs[i], HIGH);
  }
}

void TurnOffNonPWMLEDs(){
  for(int i = 0; i < sizeof(nonPWMLEDs)/sizeof(int); i++)
  {
    digitalWrite(nonPWMLEDs[i], LOW);
  }
}

void AttachServos() {
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
}

void MoveServos(int servo1Position, int servo2Position) {
  servo1.write(servo1Position);
  servo2.write(servo2Position);
}

void DetachServos() {
  servo1.detach();
  servo2.detach();
}
