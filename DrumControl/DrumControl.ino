//***************************************************
//
// Drum Control Computer
//
// This computer takes input from the analog pins 
// associated with potentiometers for each drum.
// It displays the values on the LCD screen on the
// end of the line associated with the current
// drum.
// It uses an encoder switch to determine which of
// sixteen drum sounds a selected for the four drums
// The push button on the encoder selects the drum
// name and moves the cursor to the next drum name.
//
// When the programming pin is set by the action 
// computer,  This computer sends the current
// selections of drum name positions and volumes 
// along with the master volune over to the action
// computer via the serial port.
//
//***************************************************
#define NUMBER_OF_DRUMS 4
#define NUMBER_OF_DRUM_SOUNDS 16
#define LCD_BIT_ONE 12
#define LCD_BIT_TWO 11
#define LCD_BIT_THREE 10
#define LCD_BIT_FOUR 9
#define LCD_BIT_FIVE 8
#define LCD_BIT_SIX 7
#define VOLUME_LCD_LOCATION 16 // where the volume is located at the end of each line on the lcd
#define DRUM_ONE_VOLUME_PIN A0
#define DRUM_TWO_VOLUME_PIN A1
#define DRUM_THREE_VOLUME_PIN A2
#define DRUM_FOUR_VOLUME_PIN A3
#define MASTER_VOLUME_PIN A4
#define ENCODER_CLOCK_PIN 2 //To pick up interrupt 0
#define ENCODER_DAT_PIN 4
#define ENCODER_SWITCH_PIN 6
#define PROGRAMMING_PIN 5 // where the action computer tells this computer to send data
#define PROGRAMMING_LED_PIN 3 // turn on this led when the action computer indicates programming allowed
#define MAXVOLUME  10 // wave trigger max Db
#define MINVOLUME  -70 // wav trigger min Db
#define ANALOG_VOLUME_MIN 80 // analog input min volume
#define ANALOG_VOLUME_MAX 1000 // analog input max volume
#define VOLUME_CHANGE_TRIGGER 20 // the value that says that a volume know was turned;
//
// LiquidCrystal Library Setup
// include the library code:
#include <LiquidCrystal.h>
LiquidCrystal lcd(LCD_BIT_ONE,LCD_BIT_TWO,LCD_BIT_THREE,LCD_BIT_FOUR,LCD_BIT_FIVE,LCD_BIT_SIX);
//
//
///
//const int drumSelect1Pin = A0;
//const int drumSelect2Pin = A1;
const int analogVolumePins[] = {
  DRUM_ONE_VOLUME_PIN,DRUM_TWO_VOLUME_PIN,DRUM_THREE_VOLUME_PIN,DRUM_FOUR_VOLUME_PIN,MASTER_VOLUME_PIN};
//
// Drum Names
//
int drumNamesLength = NUMBER_OF_DRUM_SOUNDS;
char* drumNames[]={
  "Bongo Low           ",
  "Bongo High          ",
  "Wood Block Low      ",
  "Wood Block High     ",
  "Cow Bell One        ",
  "Cow Bell Two        ",
  "Cow Bell Three      ",
  "Conga High          ",
  "Conga Low           ",
  "Tabula              ",
  "Tympani             ",
  "Gong                ",
  "Finger Snap         ",
  "Handclap            ",
  "Maraca              ",
  "Wipeout             "
};
//
// Encoder data
//
//boolean encoderState = false;
int encoderPosCount = 0; 
int pinCLKLast = 0;  
int CLKVal;
boolean bCW;
boolean evencount = false; // this enables the encoder to work properly.
boolean programming_ready = false;
boolean change_for_programming = false; // this is set true to make sure that the drums and volumes are sent at start.
//
// Switch Data 
//
//int drumPin = 2;
int pinSwitchLast = 0;
int drumPinSwitchLast = 0;
int switchVal;
int drumSwitchVal;
int drumVal;
int currentCursor = 0;
int drumIndex = 0;
//
// EEPROM data
//
//uint8_t EEPROM_drumNameIndex[] = {
//  0,2,3,4};
//uint8_t EEPROM_DrumVolume[] = {
//  12,11,10,9};
 int drumNameIndex[] = {
  0,1,2,3};
int DrumVolume[] = {
  12,11,10,9}; 
int masterVolume = 0;
int DrumAnalogVolume[] ={
  12,11,10,9};
int masterAnalogVolume = (ANALOG_VOLUME_MAX - ANALOG_VOLUME_MIN)/2;
//
//*******************************************************
// SETUP
//*******************************************************
//
void setup() {
  Serial.begin(9600);
  //while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  //}
  attachInterrupt(0, checkEncoder, HIGH);
  //
  // setup for the encoder and the encoder switch
  //
  pinMode (ENCODER_CLOCK_PIN,INPUT);
  digitalWrite(ENCODER_CLOCK_PIN,HIGH);
  pinMode (ENCODER_DAT_PIN,INPUT);
  digitalWrite(ENCODER_DAT_PIN,HIGH);
  pinMode(ENCODER_SWITCH_PIN,INPUT);
  digitalWrite(ENCODER_SWITCH_PIN,HIGH);
  pinMode(PROGRAMMING_PIN,INPUT);
  digitalWrite(PROGRAMMING_PIN,HIGH);
  pinMode(PROGRAMMING_LED_PIN,OUTPUT);
  digitalWrite(PROGRAMMING_LED_PIN,LOW);
  // set up the LCD's number of columns and rows: 
  lcd.begin(20, 4);
  //
  // Read Pin A
  // Whatever state it's in will reflect the last position  
  // 
  pinCLKLast = digitalRead(ENCODER_CLOCK_PIN);   
  for(drumIndex=0;drumIndex<NUMBER_OF_DRUMS;drumIndex++){  
    lcd.setCursor(1,drumIndex);
    lcd.print(drumNames[drumIndex]);
  }
  //
  // Set up the currennt master volume according to the analog pin.
  // 
  masterVolume = constrain(map(analogRead(MASTER_VOLUME_PIN),ANALOG_VOLUME_MIN,ANALOG_VOLUME_MAX , MINVOLUME, MAXVOLUME), MINVOLUME, MAXVOLUME);
  //
  drumIndex = 0;
  //
  // Set the cursor to drum 0 at location 0);
  lcd.setCursor(0,drumIndex);
  lcd.blink();
}
//
//*******************************************************
// LOOP
//
// The loop checks and sets the volume for each drum and the master volume.
// The data on the selected drums and volumes are transferred to the action computer
// The check button is checked to see if it has been pushed selecting a new drum sound.
// The drum sound selection is done by interrupt 0 on digital pin 2 (defined as ENCODER_CLK_PIN).
//*******************************************************
//
void loop() {
  checkVolumes();
  setButton();
  transferData();
}
//*******************************************************
//
// CheckEncoder
//
// Check the encoder value to see if it is changed
// Put the name for the next or previous drum on the lcd
// display based on which direction the encoder moved.
//
//*******************************************************

void checkEncoder(){
  detachInterrupt(0);
  encoderPosCount++;
  CLKVal = digitalRead(ENCODER_CLOCK_PIN);
  if(CLKVal != pinCLKLast){
    if (digitalRead(ENCODER_DAT_PIN) != CLKVal) {
       if(evencount){ 
        bCW = false;
        if(drumNameIndex[currentCursor]<=0){
          drumNameIndex[currentCursor]=drumNamesLength-1;
        }
        else{
          drumNameIndex[currentCursor]--;
        }
      }
    }
    else{
      if(evencount){ 
        bCW = true;
        if(drumNameIndex[currentCursor]>=drumNamesLength-1){
          drumNameIndex[currentCursor]=0;
        }
        else{
          drumNameIndex[currentCursor]++;
        }
      }
    }
    if(evencount){
      evencount = false;
    }else{
      evencount = true;
    }
  }
  pinCLKLast = CLKVal;
  lcd.setCursor(1,currentCursor);
  lcd.print(drumNames[drumNameIndex[currentCursor]]);
  lcd.setCursor(0,currentCursor);
  lcd.setCursor(VOLUME_LCD_LOCATION,currentCursor);
  lcd.print("    ");
  lcd.setCursor(VOLUME_LCD_LOCATION,currentCursor);
  lcd.print(DrumVolume[currentCursor],DEC);
  lcd.setCursor(0,currentCursor); 
  attachInterrupt(0, checkEncoder, HIGH);
}
//*******************************************************
//
// setButton
//
// Set the drum name for the current index to that drum
// Increment the index and set the cursor on that line
//*******************************************************
void setButton(){
   
  switchVal = digitalRead(ENCODER_SWITCH_PIN);
  if (switchVal != pinSwitchLast){ 
    lcd.setCursor(0, currentCursor);
    lcd.noBlink();
    if (switchVal == 1){
      change_for_programming = true;
      if(currentCursor < NUMBER_OF_DRUMS-1){
        currentCursor++;
      }
      else{
        currentCursor = 0;
      }
    }
    lcd.setCursor(0, currentCursor);
    lcd.blink();
  }
  pinSwitchLast = switchVal;
}
//*******************************************************
//
// transferData
//
// transfer the drum selection and drum volume for each drum
// to the other computer
//
//*******************************************************
void transferData(){
  String outData = "v ";
  if(digitalRead(PROGRAMMING_PIN) == 1){
    programming_ready = true;
    digitalWrite(PROGRAMMING_LED_PIN,HIGH);
  }else{
    digitalWrite(PROGRAMMING_LED_PIN,LOW);
  }
 if(programming_ready && change_for_programming){
  for(int i =0;i<NUMBER_OF_DRUMS;i++){
    outData= outData + String((int)DrumVolume[i]) + " " ;
  }
  outData= outData + String((int)masterVolume) + " d ";
  for(int i =0;i<NUMBER_OF_DRUMS;i++){
    outData= outData + String((int)drumNameIndex[i]) + " ";
  }
  Serial.println(outData);
  change_for_programming = false;
 }
 programming_ready = false;
}
//*******************************************************
//
// Get the volume valuse set for each drum
//
//*******************************************************
void checkVolumes(){
  int tempAnalogVolume;
  int tempVolume;
  int index;
  for(index=0;index<NUMBER_OF_DRUMS;index++){
    tempAnalogVolume = analogRead(analogVolumePins[index]);
    if(abs(DrumAnalogVolume[index]-tempAnalogVolume) > VOLUME_CHANGE_TRIGGER){
      tempVolume = constrain(map(tempAnalogVolume, ANALOG_VOLUME_MIN,ANALOG_VOLUME_MAX , MINVOLUME, MAXVOLUME),MINVOLUME,MAXVOLUME);
      DrumAnalogVolume[index] = tempAnalogVolume;
      DrumVolume[index] = tempVolume;
      change_for_programming = true;
      lcd.setCursor(VOLUME_LCD_LOCATION,index);
      lcd.print("    ");
      lcd.setCursor(VOLUME_LCD_LOCATION,index);
      lcd.print(DrumVolume[index],DEC);
      lcd.setCursor(0,currentCursor);
    }
  }
  tempAnalogVolume = analogRead(MASTER_VOLUME_PIN);
  if(abs(masterAnalogVolume-tempAnalogVolume) > VOLUME_CHANGE_TRIGGER){
    tempVolume = constrain(map(tempAnalogVolume, ANALOG_VOLUME_MIN,ANALOG_VOLUME_MAX , MINVOLUME, MAXVOLUME),MINVOLUME,MAXVOLUME);
    masterAnalogVolume = tempAnalogVolume;
    masterVolume = tempVolume;
    change_for_programming = true;
  }
}












