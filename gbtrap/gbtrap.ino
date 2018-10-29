/***************************************************
 * Coded by TragicManner
 * version 1.3 - Now with Coroutines/LED support!
 * utilizes the Couroutines library by renaudbedard
 * https://github.com/renaudbedard/littlebits-arduino/tree/master/Libraries/Coroutines
 ***************************************************/

#include <Coroutines.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <YX5300.h>

// --door variables: state--
unsigned long doorTimer = 0;
const int doorWaitTime = 300; // If your doors don't have enough time to close, make this longer

// --trap variables
int trapState = 0; // trap states are 0 for idle, 1 for pedal press waiting for doors open, 2 for doors open and audio/LED show active, 3 for doors open and audio/LED show done, 4 for doors open and waiting to close
const int trapIdle = 0;
const int trapOpening = 1;
const int trapOpenShow = 2;
const int trapCapturing = 3;
const int trapClosing = 4;
const int trapCheckingCapture = 5;
const int trapCaptured = 6;

int stateDurations[7] = {0, 1302, 11000, 6900, doorWaitTime, 1087, 5775}; // Each index corresponds with the states defined above. If the value is 0, then the state isn't governed by time
bool pedalChangesState[7] = {true, false, true, false, false, false, false};

int timeBetweenBeeps = 336; //time between beeps in audio file for after the graph is full and the trap starts beeping
int numberOfBeeps = 15;
int timeBeforeFirstBeep = 125;
int redLightDurationForBeep = 58;

// --door variables: servo1--  SERVO OPEN AND CLOSED VALUES NEED TO BE ADJUSTED FOR YOUR SERVOS!
Servo servo1;
const int servo1Pin = 3;
const int servo1OpenPosition = 68;
const int servo1ClosedPosition = 15;

// --door variables: servo_2--
Servo servo2;
const int servo2Pin = 10;
const int servo2OpenPosition = 15;
const int servo2ClosedPosition = 68;

// --Pedal variables--
const int pedalPin = 2;
// pedalState tracks whether or not the pedal is on or off
int pedalState = 0;
int latestPedal = 0;

// -- General LED variables-- (LED Pins)
int PWMPink = 9;
int redLED = 4;
int whiteLED01 = 7;
int whiteLED02 = 8;
int PWMBlue = 11;
int blueLED = 12;
int pinkLED = 13;
int barLED01 = 14;
int barLED02 = 15;
int barLED03 = 16;
int yellowLED = 17;
int whiteLED03 = 18;
int whiteLED04 = 19;

int currentLED = 0;
int currentLEDTime = 0;
int currentPauseAtHigh = 0;
int currentPauseAtLow = 0;
int currentStartingBrightness = 0;
int currentBrightnessInterval = 0;

// --PWM LED Variables--
int fadeStep = 30; // This controls how smooth the LED fades will be
int PWMPins[2] = {PWMBlue, PWMPink};
int PWMPauseHighOpen[2] = {50, 50};
int PWMPauseLowOpen[2] = {50, 50};
int PWMFadeAmountOpen[2] = {30, 30};
int PWMPauseHighCapture[2] = {25, 25};
int PWMPauseLowCapture[2] = {25, 25};
int PWMFadeAmountCapture[2] = {50, 50};
int PWMStartingBrightnessOpen[2] = {0, 100};
int PWMStartingBrightnessCapture[2] = {100, 0};

// --PWM LED Variables that shouldn't be touched--
int PWMCurrentBrightness[2] = {0, 0};
int PWMCurrentPauseHigh[2] = {0, 0};
int PWMCurrentPauseLow[2] = {0, 0};
int PWMCurrentFadeAmount[2] = {0, 0};

// -- Coroutine Stuff --
Coroutines<7> coroutines;

// -- Audio Chip Stuff --
int RX_Pin = 5; //The pin the RX connection hooks into
int TX_Pin = 6; //The pin the TX connection hooks into
YX5300 audio; //Setting up the audio chip RX on pin 5, and TX on pin 6


void setup() 
{
  //Serial.begin(9600);
  // Initialize LEDs
  InitializeLEDs();
  // Turn on LEDs that need to start on
  digitalWrite(redLED, HIGH);
  // Set up the pin that watches for the pedal button
  pinMode(pedalPin, INPUT_PULLUP);
  pedalState = digitalRead(pedalPin);
  // Make sure servos are closed
  AttachServos();
  MoveServos(servo1ClosedPosition, servo2ClosedPosition);
  delay(doorWaitTime);
  latestPedal = pedalState;
  // Wait for the audio player to start up, then initialize it
  delay(500);
  audio.Initialize();
  DetachServos();
  delay(200); //wait for the audio player to be ready 
  audio.PlayFileWithVolume(8, 30);
  delay(2448);
  audio.SetVolume(30);
  delay(20);
  audio.CycleAudio(9);
}

void loop() {
  // First, read the state of the pedal
  pedalState = digitalRead(pedalPin);

  bool changeState = false;

  if(latestPedal != pedalState)
  {
    latestPedal = pedalState;
    if(pedalChangesState[trapState])
    {
      changeState = true;
    }
  }

  // Check if it's time to change state
  if(stateDurations[trapState] > 0 && millis() >= doorTimer)
  {
    changeState = true;
  }

  // Update the LEDs and coroutines
  coroutines.update();

  if(changeState)
  {
    trapState ++;
    Serial.print(trapState + "\n");
    if(trapState > trapCaptured)
    {
      trapState = 0;
    }
    doorTimer = millis() + stateDurations[trapState];

    switch(trapState)
    {
      case trapIdle:
        TrapIdle();
        break;
      case trapOpening:
        Serial.print("OpeningDoors\n");
        coroutines.start(OpenDoors);
        break;
      case trapOpenShow:
        Serial.print("OpenShow\n");
        OpenShow();
        break;
      case trapCapturing:
        Serial.print("CapturingGhost\n");
        Capturing();
        break;
      case trapClosing:
        Serial.print("ClosingTrap\n");
        coroutines.start(CloseDoors);
        break;
      case trapCheckingCapture:
        Serial.print("CheckingCapacityAndCapture\n");
        coroutines.start(CheckCapture);
        break;
      case trapCaptured:
        Serial.print("GhostCaptured\n");
        coroutines.start(TrapCaptured);
        break;
    }
  }
}

void TrapIdle()
{
  audio.SetVolume(30);
  delay(20);
  audio.CycleAudio(9);
  TurnOffAllLEDs();
  digitalWrite(redLED, HIGH);
}

void OpenDoors(COROUTINE_CONTEXT(coroutine)) 
{
  BEGIN_COROUTINE;
  audio.PlayFileWithVolume(1, 30);
  AttachServos();
  MoveServos(servo1OpenPosition, servo2OpenPosition);
  digitalWrite(whiteLED01, HIGH);
  digitalWrite(whiteLED02, HIGH);
  digitalWrite(whiteLED03, HIGH);
  digitalWrite(whiteLED04, HIGH);
  digitalWrite(pinkLED, HIGH);
  digitalWrite(blueLED, HIGH);
  coroutine.wait(doorWaitTime);
  COROUTINE_YIELD;
  DetachServos();
  //coroutines.start(FadeLED);
  END_COROUTINE;
}

void CloseDoors(COROUTINE_CONTEXT(coroutine)) 
{
  BEGIN_COROUTINE;
  AttachServos();
  MoveServos(servo1ClosedPosition, servo2ClosedPosition);
  coroutine.wait(doorWaitTime);
  COROUTINE_YIELD;
  DetachServos();
  //coroutines.start(FadeLED);
  END_COROUTINE;
}

void OpenShow()
{
  audio.PlayFileWithVolume(5, 30);
  digitalWrite(whiteLED01, HIGH);
  digitalWrite(whiteLED02, HIGH);
  digitalWrite(whiteLED03, HIGH);
  digitalWrite(whiteLED04, HIGH);
  digitalWrite(pinkLED, HIGH);
  digitalWrite(blueLED, HIGH);
  coroutines.start(FadeLED);
}

void Capturing()
{
  audio.PlayFileWithVolume(3, 30);
  digitalWrite(whiteLED01, HIGH);
  digitalWrite(whiteLED02, HIGH);
  digitalWrite(whiteLED03, HIGH);
  digitalWrite(whiteLED04, HIGH);
  digitalWrite(pinkLED, HIGH);
  digitalWrite(blueLED, HIGH);
  coroutines.start(FadeLED);
}

void CheckCapture(COROUTINE_CONTEXT(coroutine))
{
  BEGIN_COROUTINE;
  TurnOffAllLEDs();
  digitalWrite(redLED, HIGH);
  audio.PlayFileWithVolume(2, 30);
  digitalWrite(barLED01, HIGH);
  coroutine.wait(156);
  COROUTINE_YIELD;
  digitalWrite(barLED02, HIGH);
  coroutine.wait(156);
  COROUTINE_YIELD;
  digitalWrite(barLED03, HIGH);
  coroutine.wait(156);
  COROUTINE_YIELD;
  digitalWrite(yellowLED, HIGH);
  coroutine.wait(156);
  COROUTINE_YIELD;
  //digitalWrite(barLED01, LOW);
  //digitalWrite(barLED02, LOW);
  //digitalWrite(barLED03, LOW);
  END_COROUTINE;
}

void TrapCaptured(COROUTINE_CONTEXT(coroutine))
{
  COROUTINE_LOCAL(int, i);
  BEGIN_COROUTINE;
  audio.PlayFileWithVolume(4, 30);
  coroutine.wait(timeBeforeFirstBeep);
  COROUTINE_YIELD;
  for(i=0; i < numberOfBeeps; i++)
  {
    digitalWrite(redLED, HIGH);
    coroutine.wait(redLightDurationForBeep);
    COROUTINE_YIELD;
    digitalWrite(redLED, LOW);
    coroutine.wait(timeBetweenBeeps);
    COROUTINE_YIELD;
  }
  END_COROUTINE;
}

void FadeLED(COROUTINE_CONTEXT(coroutine)) {
  COROUTINE_LOCAL(int, local_state);
  COROUTINE_LOCAL(int, i);
  
  BEGIN_COROUTINE;
  
  local_state = trapState;

  if(trapState == trapOpenShow)
  {
    copy(PWMStartingBrightnessOpen, PWMCurrentBrightness, 4);
    copy(PWMPauseHighOpen, PWMCurrentPauseHigh, 4);
    copy(PWMPauseLowOpen, PWMCurrentPauseLow, 4);
    copy(PWMFadeAmountOpen, PWMCurrentFadeAmount, 4);
  }

  if(trapState == trapCapturing)
  {
    copy(PWMStartingBrightnessCapture, PWMCurrentBrightness, 4);
    copy(PWMPauseLowCapture, PWMCurrentPauseHigh, 4);
    copy(PWMPauseLowCapture, PWMCurrentPauseLow, 4);
    copy(PWMFadeAmountCapture, PWMCurrentFadeAmount, 4);
  }

  coroutine.wait(fadeStep);
  COROUTINE_YIELD;
  
  while(trapState == local_state)
  {
    for(i = 0; i < (sizeof(PWMPins)/sizeof(int)); i++)
    {
      //Serial.print("Pin: " + String(i) + " Bright: " + String(PWMCurrentBrightness[i]) + "Fade: " + String(PWMCurrentFadeAmount[i]) + "\n");
      // change the brightness for next time through the loop:
      PWMCurrentBrightness[i] = PWMCurrentBrightness[i] + PWMCurrentFadeAmount[i];
    
      if (PWMCurrentBrightness[i] > 255)
      {
        PWMCurrentBrightness[i] = 255;
      }
      else if (PWMCurrentBrightness[i] < 0) 
      {
        PWMCurrentBrightness[i] = 0;
      }
    
      // reverse the direction of the fading at the ends of the fade:
      if (PWMCurrentBrightness[i] <= 0 || PWMCurrentBrightness[i] >= 255) 
      {
        PWMCurrentBrightness[i] = -PWMCurrentBrightness[i];
      }

      analogWrite(PWMPins[i], PWMCurrentBrightness[i]);
    }
    coroutine.wait(fadeStep);
    COROUTINE_YIELD;
  }
  END_COROUTINE;
}

void InitializeLEDs()
{
  pinMode(PWMPink, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(whiteLED01, OUTPUT);
  pinMode(whiteLED02, OUTPUT);
  pinMode(PWMBlue, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(pinkLED, OUTPUT);
  pinMode(barLED01, OUTPUT);
  pinMode(barLED02, OUTPUT);
  pinMode(barLED03, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(whiteLED03, OUTPUT);
  pinMode(whiteLED04, OUTPUT);
}

void TurnOffAllLEDs()
{
  digitalWrite(PWMPink, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(whiteLED01, LOW);
  digitalWrite(whiteLED02, LOW);
  digitalWrite(PWMBlue, LOW);
  digitalWrite(blueLED, LOW);
  digitalWrite(pinkLED, LOW);
  digitalWrite(barLED01, LOW);
  digitalWrite(barLED02, LOW);
  digitalWrite(barLED03, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(whiteLED03, LOW);
  digitalWrite(whiteLED04, LOW);
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

void copy(int* src, int* dst, int len) {
    memcpy(dst, src, sizeof(src[0])*len);
}
