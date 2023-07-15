// LIBRARY
#include "MIDIUSB.h"

//Mux control pins
int s0 = 2;
int s1 = 3;
int s2 = 4;
int s3 = 5;

int s4 = 6;
int s5 = 7;
int s6 = 8;
int s7 = 9;

//Mux in "SIG" pin
int SIG_pin = A0;
int SIG_pin2 = A1;

bool buttonDown[16];
const int NUM_POTS = 16;     // number of potentiometers to read
int potCState[NUM_POTS] = {0}; // Current state of the pot; delete 0 if 0 pots
int potPState[NUM_POTS] = {0}; // Previous state of the pot; delete 0 if 0 pots
int potVar = 0; // Difference between the current and previous state of the pot

int midiCState[NUM_POTS] = {0}; // Current state of the midi value; delete 0 if 0 pots
int midiPState[NUM_POTS] = {0}; // Previous state of the midi value; delete 0 if 0 pots

const int TIMEOUT = 300; //* Amount of time the potentiometer will be read after it exceeds the varThreshold
const int varThreshold = 5; //* Threshold for the potentiometer signal variation

boolean potMoving = true; // If the potentiometer is moving
unsigned long PTime[NUM_POTS] = {0}; // Previously stored time; delete 0 if 0 pots
unsigned long timer[NUM_POTS] = {0}; // Stores the time that has elapsed since the timer was reset; delete 0 if 0 pots

//ultrasonic sensor stuff
const int trigPin = 16;
const int echoPin = 10;
long duration; // defines variables
int distance; // defines variables
boolean ultMoving = true; // If the ultrasonic sensor value is moving
int ultCState = {0}; // Current state of the pot; delete 0 if 0 pots
int ultPState = {0}; // Previous state of the pot; delete 0 if 0 pots
int ultVar = 0; // Difference between the current and previous state of the pot
unsigned long UTime = {0}; // Previously stored time; delete 0 if 0 pots
unsigned long ultTimer = {0}; // Stores the time that has elapsed since the timer was reset; delete 0 if 0 pots
int midiultCState = {0}; // Current state of the midi value; delete 0 if 0 pots
int midiultPState = {0}; // Previous state of the midi value; delete 0 if 0 pots
const int varultThreshold = 0; //* Threshold for the potentiometer signal variation

// MIDI Assignments
byte midiCh = 1;  //* MIDI channel to be used
byte note = 36;   //* Lowest note to be used; 36 = C2; 60 = Middle C
byte cc = 1;      //* Lowest MIDI CC to be used

void setup() {
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  pinMode(SIG_pin, INPUT);  // Set up Z as an input

  //set up second mux
  pinMode(s4, OUTPUT);
  pinMode(s5, OUTPUT);
  pinMode(s6, OUTPUT);
  pinMode(s7, OUTPUT);
  digitalWrite(s4, LOW);
  digitalWrite(s5, LOW);
  digitalWrite(s6, LOW);
  digitalWrite(s7, LOW);
  pinMode(SIG_pin2, INPUT);

  //ultrasonic sensor stuff
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);


  //  Serial.begin(9600);

  for (int i = 0; i < 16; i++) {
    buttonDown[i] = false;
  }

}


void loop() {


  buttons();
  potentiometers();
  ultraSonic();
//  delay(100);
}

void ultraSonic() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
//  Serial.print("Distance: ");
//  Serial.println(distance);
if(distance < 26){
  ultCState = distance;
  midiultCState = map(ultCState, 10, 25, 0, 127); // Maps the reading of the potCState to a value usable in midi

  ultVar = abs(ultCState - ultPState); // Calculates the absolute value between the difference between the current and previous state of the pot

  if (ultVar > varultThreshold) { // Opens the gate if the potentiometer variation is greater than the threshold
    UTime = millis(); // Stores the previous time
  }
  ultTimer = millis() - UTime; // Resets the timer 11000 - 11000 = 0ms

  if (ultTimer < TIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
    ultMoving = true;
  }
  else {
    ultMoving = false;
  }

  if (ultMoving == true) { // If the potentiometer is still moving, send the change control
    if (midiultPState != midiultCState) {

      // Sends  MIDI CC
      // Use if using with ATmega32U4 (micro, pro micro, leonardo...)
      controlChange(midiCh, 35, midiultCState); //  (channel, CC number,  CC value)
      MidiUSB.flush();

      ultPState = ultCState; // Stores the current reading of the potentiometer to compare with the next
      midiultPState = midiultCState;
    }
  }
}
}

void buttons() {
  //Loop through and read all 16 values
  //Reports back Value at channel 6 is: 346
  for (int channel = 0; channel < 16; channel ++) {
//        Serial.print("Button ");
//        Serial.print(channel);
//        Serial.print(": ");
//        Serial.println(readMux(channel));

    readMux(channel);                    // Select one at a time
    int inputValue = analogRead(SIG_pin);  // and read SIG

    if (inputValue < 1 && !buttonDown[channel]) {
      //send midi information
      noteOn(midiCh, note + channel, 127);  // channel, note, velocity
      MidiUSB.flush();
      buttonDown[channel] = true;
    } else if (inputValue >= 10  && buttonDown[channel]) {
      buttonDown[channel] = false;
    }
  }
}

void potentiometers() {
  for (int channel2 = 0; channel2 < 16; channel2++) { //run through all the channels of the second multiplexer to read the input value of the pots
        Serial.print("Potentiometer ");
        Serial.print(channel2);
        Serial.print(": ");
        Serial.println(readMux2(channel2));
    readMux2(channel2);                  // Select one at a time
    int inputValue2 = analogRead(SIG_pin2); // and read SIG of each channel
    potCState[channel2] = inputValue2;     // store the value in the array
    midiCState[channel2] = map(potCState[channel2], 0, 1023, 0, 127); // Maps the reading of the potCState to a value usable in midi
    potVar = abs(potCState[channel2] - potPState[channel2]); // Calculates the absolute value between the difference between the current and previous state of the pot

    if (potVar > varThreshold) { // Opens the gate if the potentiometer variation is greater than the threshold
      PTime[channel2] = millis(); // Stores the previous time
    }
    timer[channel2] = millis() - PTime[channel2]; // Resets the timer 11000 - 11000 = 0ms

    if (timer[channel2] < TIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
      potMoving = true;
    }
    else {
      potMoving = false;
    }

    if (potMoving == true) { // If the potentiometer is still moving, send the change control
      if (midiPState[channel2] != midiCState[channel2]) {

        // Sends  MIDI CC
        // Use if using with ATmega32U4 (micro, pro micro, leonardo...)
        controlChange(midiCh, cc + channel2, midiCState[channel2]); //  (channel, CC number,  CC value)
        MidiUSB.flush();

        potPState[channel2] = potCState[channel2]; // Stores the current reading of the potentiometer to compare with the next
        midiPState[channel2] = midiCState[channel2];
      }
    }                        // move to the next index
  }
}

int readMux(int channel) {
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4] = {
    {0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    {1, 0, 1, 0}, //channel 5
    {0, 1, 1, 0}, //channel 6
    {1, 1, 1, 0}, //channel 7
    {0, 0, 0, 1}, //channel 8
    {1, 0, 0, 1}, //channel 9
    {0, 1, 0, 1}, //channel 10
    {1, 1, 0, 1}, //channel 11
    {0, 0, 1, 1}, //channel 12
    {1, 0, 1, 1}, //channel 13
    {0, 1, 1, 1}, //channel 14
    {1, 1, 1, 1} //channel 15
  };

  //loop through the 4 sig
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);

  }

  //read the value at the SIG pin
  int val = analogRead(SIG_pin);

  //return the value
  return val;
}

int readMux2(int channel) {
  int controlPin2[] = {s4, s5, s6, s7};

  int muxChannel2[16][4] = {
    {0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    {1, 0, 1, 0}, //channel 5
    {0, 1, 1, 0}, //channel 6
    {1, 1, 1, 0}, //channel 7
    {0, 0, 0, 1}, //channel 8
    {1, 0, 0, 1}, //channel 9
    {0, 1, 0, 1}, //channel 10
    {1, 1, 0, 1}, //channel 11
    {0, 0, 1, 1}, //channel 12
    {1, 0, 1, 1}, //channel 13
    {0, 1, 1, 1}, //channel 14
    {1, 1, 1, 1} //channel 15
  };

  //loop through the 4 control pins
  for (int i = 0; i < 4; i ++) {
    digitalWrite(controlPin2[i], muxChannel2[channel][i]);

  }

  //read the value at the SIG pin
  int val2 = analogRead(SIG_pin2);
  //return the value
  return val2;

}

// Arduino MIDI functions MIDIUSB Library
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = { 0x08, 0x80 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = { 0x0B, 0xB0 | channel, control, value };
  MidiUSB.sendMIDI(event);
}
