// ****************************************************************************
//       Sketch: WTriggerUno
// Date Created: 4/22/2015
//
//     Comments: Demonstrates basic serial control of the WAV Trigger from an
//               Arduino.
//
//  Programmers: Jamie Robertson, info@robertsonics.com
//
// ****************************************************************************
//
// To use this sketch, you'll need to:
//
// 1) Download and install the AltSoftSerial library.
// 2) Download and install the Metro library.
// 3) Connect 2 wires from the UNO to the WAV Trigger's serial connector:
//
//    Uno           WAV Trigger
//    ===           ===========
//    GND  <------> GND
//    Pin9 <------> RX
//
//    If you want to power the WAV Trigger from the Uno, then close the 5V
//    solder jumper on the WAV Trigger and connect a 3rd wire:
//
//    5V   <------> 5V
//
//    (If you are using an Arduino with extra hardware serial ports, such as
//    an Arduino Mega or Teensy, you don't need AltSoftSerial, and you should
//    edit the wavTrigger.h library file to select the desired serial port
//    according to the documentation contained in that file. And use the
//    appropriate TX/RX pin connections to your Arduino)
//
// 4) Download and install the demo wav files onto the WAV Trigger's microSD
//    card. You can find them here:
//
//    http://robertsonics.com/2015/04/25/arduino-serial-control-tutorial/
//
//    You can certainly use your own tracks instead, although the demo may
//    not make as much sense. If you do, make sure your tracks are at least 10
//    to 20 seconds long and have no silence at the start of the file.

//#include <Metro.h>
#include <AltSoftSerial.h>    // Arduino build environment requires this
#include <wavTrigger.h>

#define BAUDRATE 9600
#define LED 13                // our LED
#define MAXTRACK  16
#define NUMBER_OF_DRUMS 4
//
// define the analog pin for the piezo input
//
#define DRUM_1 A0    // Our analog pin
#define DRUM_2 A1    // Our analog pin
#define DRUM_3 A2    // Our analog pin
#define DRUM_4 A3    // Our analog pin
#define TRACK_FOR_DRUM_ONE 0
#define TRACK_FOR_DRUM_TWO 1
#define TRACK_FOR_DRUM_THREE 2
#define TRACK_FOR_DRUM_FOUR 3
//
// define the pins used for each of the eight
// associated triggers.  The idea is that there
// are eight files the play the same sound at one
// of eight levels.
//
#define DRUM_1_LEVEL_1 6
#define DRUM_1_LEVEL_2 7
#define DRUM_1_LEVEL_3 8
#define DRUM_1_LEVEL_4 9

#define DRUM_2_LEVEL_1 10
#define DRUM_2_LEVEL_2 11
#define DRUM_2_LEVEL_3 12
#define DRUM_2_LEVEL_4 13

#define DRUM_3_LEVEL_1 4
#define DRUM_3_LEVEL_2 4
#define DRUM_3_LEVEL_3 5
#define DRUM_3_LEVEL_4 5

#define DRUM_4_LEVEL_1 2
#define DRUM_4_LEVEL_2 2
#define DRUM_4_LEVEL_3 3
#define DRUM_4_LEVEL_4 3
//
// Level at which we thing the drum has been hit
//
#define TRIGGERLEVEL 80
#define LEVELMAX 1000
#define DELAY_BETWEEN_BEATS 40
#define TRIGGER_HIGH_TIME  10
#define VOLUME_MAX 10
#define VOLUME_MIN -70
#define DRUM_VOLUME_MIN 0
#define VOLUME_HIGH 900
#define VOLUME_LOW 25
#define DRUM_VOLUME_HIGH 1000
#define DRUM_VOLUME_LOW 80
//
// Programing state defines
//
#define PROGRAMMING_ALLOWED_PIN 3
#define PROGRAMMING_SWITCH_PIN  2
#define PROGRAMMING_ALLOWED 1
#define PROGRAMMING_NOT_ALLOWED 0

wavTrigger wTrig;             // Our WAV Trigger object
int sum_for_DRUM_1 = 0;
int sum_for_DRUM_2 = 0;
int sum_for_DRUM_3 = 0;
int sum_for_DRUM_4 = 0;
int sum_for_VOLUME = 0;
boolean NOHIT = true;
boolean Programming = false;
boolean sentOnce = false;
int track[NUMBER_OF_DRUMS] = {1,2,3,4};
int vol = VOLUME_MIN;
int volumes[NUMBER_OF_DRUMS+1] = {5,5,5,5,5};
int baseVolumes[NUMBER_OF_DRUMS+1] = {5,5,5,5,5};
int tempTracks[NUMBER_OF_DRUMS] = {1,2,3,4};
int tempVolumes[NUMBER_OF_DRUMS+1] = {5,5,5,5,5};
//
//
// ****************************************************************************
// SETUP
// ****************************************************************************
//
//
void setup() {
  Serial.begin(BAUDRATE);
  pinMode(PROGRAMMING_SWITCH_PIN,INPUT);
  pinMode(PROGRAMMING_ALLOWED_PIN,OUTPUT);
  digitalWrite(PROGRAMMING_SWITCH_PIN,HIGH);
  digitalWrite(PROGRAMMING_ALLOWED_PIN,HIGH);
//
// Start by saying that programming is not allowed
//
  Programming = false;
//
// If the Arduino is powering the WAV Trigger, we should wait for the WAV
//  Trigger to finish reset before trying to send commands.
  delay(1000);
//
// WAV Trigger startup at 57600 BPS
//
  wTrig.start();
  delay(10);
//
// Send a stop-all command and reset the sample-rate offset, in case we have
//  reset while the WAV Trigger was already playing.
//
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);
  //
  // Set the initial master volume
  //
  vol = 5;
  wTrig.masterGain(vol);
  //wTrig.setReporting(true);
  delay(100);
}
//
//
// ****************************************************************************
// LOOP
// ****************************************************************************
//
//
void loop() {
  if((sum_for_DRUM_1=analogRead(DRUM_1))>TRIGGERLEVEL){
    vol = map(sum_for_DRUM_1, baseVolumes[TRACK_FOR_DRUM_ONE],DRUM_VOLUME_HIGH , DRUM_VOLUME_MIN, VOLUME_MAX);
    vol = constrain(vol+baseVolumes[TRACK_FOR_DRUM_ONE],DRUM_VOLUME_MIN,DRUM_VOLUME_HIGH);
    wTrig.trackGain(track[TRACK_FOR_DRUM_ONE], vol);
    wTrig.trackPlayPoly(track[TRACK_FOR_DRUM_ONE]);
    while((sum_for_DRUM_1=analogRead(DRUM_1))>TRIGGERLEVEL){
      delay(3);
    }
    delay(3);
    NOHIT = false;
  }
  if((sum_for_DRUM_2=analogRead(DRUM_2))>TRIGGERLEVEL){
    vol = map(sum_for_DRUM_2, baseVolumes[TRACK_FOR_DRUM_TWO],DRUM_VOLUME_HIGH , DRUM_VOLUME_MIN, VOLUME_MAX);
    vol = constrain(vol+baseVolumes[TRACK_FOR_DRUM_TWO],DRUM_VOLUME_MIN,DRUM_VOLUME_HIGH);
    wTrig.trackGain(track[TRACK_FOR_DRUM_TWO], vol);
    wTrig.trackPlayPoly(track[TRACK_FOR_DRUM_TWO]);
    while((sum_for_DRUM_2=analogRead(DRUM_2))>TRIGGERLEVEL){
      delay(3);
    }
    delay(3);
    NOHIT = false;
  }
  if((sum_for_DRUM_3=analogRead(DRUM_3))>TRIGGERLEVEL){
    vol = map(sum_for_DRUM_3, baseVolumes[TRACK_FOR_DRUM_THREE],DRUM_VOLUME_HIGH , DRUM_VOLUME_MIN, VOLUME_MAX);
    vol = constrain(vol+baseVolumes[TRACK_FOR_DRUM_THREE],DRUM_VOLUME_MIN,DRUM_VOLUME_HIGH);
    wTrig.trackGain(track[TRACK_FOR_DRUM_THREE], vol);
    wTrig.trackPlayPoly(track[TRACK_FOR_DRUM_THREE]);
    while((sum_for_DRUM_3=analogRead(DRUM_3))>TRIGGERLEVEL){
      delay(3);
    }
    delay(3);
    NOHIT = false;
  }
  if((sum_for_DRUM_4=analogRead(DRUM_4))>TRIGGERLEVEL){
    vol = map(sum_for_DRUM_4, baseVolumes[TRACK_FOR_DRUM_FOUR],DRUM_VOLUME_HIGH , DRUM_VOLUME_MIN, VOLUME_MAX);
    vol = constrain(vol+baseVolumes[TRACK_FOR_DRUM_FOUR],DRUM_VOLUME_MIN,DRUM_VOLUME_HIGH);
    wTrig.trackGain(track[TRACK_FOR_DRUM_FOUR], vol);
    wTrig.trackPlayPoly(track[TRACK_FOR_DRUM_FOUR]);
    while((sum_for_DRUM_4=analogRead(DRUM_4))>TRIGGERLEVEL){
      delay(3);
    }
    delay(3);
    NOHIT = false;
  }

  //
  // Only check the programming if no drum was hit in this loop.
  //
  if(NOHIT){
    // check the programming switch.
    if(digitalRead(PROGRAMMING_SWITCH_PIN) == PROGRAMMING_ALLOWED){
      // get the volume and drum selection from the control computer
      if(!sentOnce){
       sentOnce = true;
       //Serial.println("READY");
      }
      getProgrammingData();
    }else{
      // set the programming pin low to inform the control computer that
      // no data is eing accepted
      sentOnce = false;
       digitalWrite(PROGRAMMING_ALLOWED_PIN,LOW);
    }
  }
  //
  // clear hit indicateor and volume levels
  //
  NOHIT = true;
  sum_for_DRUM_1 = 0;
  sum_for_DRUM_2 = 0;
  sum_for_DRUM_3 = 0;
  sum_for_DRUM_4 = 0;
  vol = 0;
}
//
//
// ****************************************************************************
// GETPROGRANNINGDATA
// ****************************************************************************
//
//
void getProgrammingData(){
  boolean goodData = false;
  String controlData = "                                                     ";
  int index = 0; 
  controlData[0] = 0;
  digitalWrite(PROGRAMMING_ALLOWED_PIN,HIGH);
  if(Serial.available() >= 22){
    controlData = Serial.readString();
      //Serial.println(controlData);
      //Serial.println(Serial.available(),DEC);
    goodData = formatData(controlData); 
  }
  if(goodData){
    for(index=0;index<NUMBER_OF_DRUMS;index++){
      track[index] = tempTracks[index]+1;
      //Serial.print(" D ");
      //Serial.println(tempTracks[index],DEC);
      baseVolumes[index] = tempVolumes[index]+1;
      //Serial.print(" V ");
      //Serial.println(baseVolumes[index],DEC);
      wTrig.trackGain(track[index],baseVolumes[index]);
    }
    baseVolumes[NUMBER_OF_DRUMS] = tempVolumes[NUMBER_OF_DRUMS];
    //Serial.println(baseVolumes[NUMBER_OF_DRUMS],DEC);
    wTrig.masterGain(baseVolumes[NUMBER_OF_DRUMS]);
  }
}
//
// ****************************************************************************
// FORMAT DATA
// ****************************************************************************
//
//
boolean formatData(String inputData){
  String currentVolume = "";
  String currentDrum = "";
  boolean doingVolume = false;
  boolean doingDrums = false;
  boolean doneVolume = false;
  boolean doneDrums = false;
  int drumCount = 0;
  int volumeCount = 0;
  inputData = inputData + ' ';
  for(int index = 0 ; index < inputData.length(); index++){
    switch( inputData[index]){
      case 'v':
      case 'V':
         doingVolume = true;
         doingDrums = false;
         if(inputData[index+1] == ' ') index++; // skip the space after the V 
         break;
      case 'd':
      case 'D':
         doingDrums = true;
         doingVolume = false;
         if(inputData[index+1] == ' ') index++; // skip the space after the V 
         break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '-':
         if(doingVolume){
           currentVolume = currentVolume + inputData[index];
         }
         if(doingDrums){
            currentDrum = currentDrum + inputData[index];
         }
         break;
      default:
         if(doingVolume){
           currentVolume = currentVolume + inputData[index];
            tempVolumes[volumeCount] = constrain(currentVolume.toInt(),VOLUME_MIN,VOLUME_MAX);
            currentVolume = "";
            volumeCount++;
            if(volumeCount >= NUMBER_OF_DRUMS+1){
              volumeCount = 0;
              doingVolume = false;
              doneVolume = true;
            }
         }
         if(doingDrums){
            currentDrum = currentDrum + inputData[index];
            tempTracks[drumCount] = constrain(currentDrum.toInt(),0,MAXTRACK);
            currentDrum = "";
            drumCount++;
            if(drumCount >= NUMBER_OF_DRUMS){
              drumCount = 0;
              doingDrums = false;
              doneDrums = true;
            }
         }   
    }
  }
  return doneDrums & doneVolume;
}












