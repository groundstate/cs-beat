

#include <Adafruit_DotStar.h>
#include <SPI.h>     

#define NUMPIXELS 59 // number of LEDs in the strip

// Pins for contorlling the LEDs
#define DATAPIN    4
#define CLOCKPIN   5

// Using software SPI
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.


#define PPSPIN 2 // can use pins 2 (IRQ 0) and 3 (IRQ 1) on the Uno
#define MODESELPIN 3 // (IRQ(1)
#define PPSTEST 13

// Colours as per DOSTAR_BRG
#define RED 0xFF00
#define BLUE 0xFF
#define GREEN 0xFF0000

// Number of display modes, less 1
#define MAXMODE 4

// Globals
int   tail = 0, head  = 0, incr = 1; // Index of last 'on' pixel
int   tail2 = 0, head2  = 0, incr2= -1; // Index of last 'on' pixel
volatile int  mode = 4; // changed in ISR
volatile int  modeChanged = 0; // changed in ISR 
volatile int triggered = 0;  // triggered by pps
uint32_t color = BLUE;   
uint32_t color2 = GREEN; 

void setup() {
  
  pinMode(PPSTEST, OUTPUT);
 
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off
  //attachInterrupt(digitalPinToInterrupt(PPSPIN),trigger,RISING); // won't compile
  attachInterrupt(0,trigger,RISING); // input PPS
  attachInterrupt(1,selectMode,RISING); // mode switch
  initMode();
 
}

void initMode(){
 tail=0;
 head=0;
 incr=1;
 color=BLUE;
 switch (mode)
  {
    case 0:
      break;
    case 1: case 2:
      tail=0;
      head=9;
      color=0xff0000;
      break;
    case 3:
      tail=0;
      head=7;
      tail2 = NUMPIXELS - 8;
      head2 = NUMPIXELS - 1;
      color2 = GREEN;
      incr2=-1;
      break;
    case 4:
      head = NUMPIXELS/2+4;
      tail = head -7;
      tail2 = NUMPIXELS/2-4;
      head2 = tail2+7;
      color2 = GREEN;
      incr=-1;
      incr2=1;
      break;
  }
}

void loop() {

  if  (triggered==1 ){ 
    if (modeChanged == 1){
      initMode();
      modeChanged=0;
    }
    switch (mode)
    {
      case 0: 
        mode0();
        break;
      case 1: 
        mode1();
        break;
      case 2:
        mode2();
        break;
      case 3:
        mode3();
        break;
      case 4:
        mode4();
        break;
    }
  }
}

// 
// Pulse rising to full lengths
//

void mode0(){
  if (incr > 0)
    strip.setPixelColor(head, color); // 'On' pixel at head
  else
    strip.setPixelColor(head, 0);
  strip.show();// Refresh strip
  if (incr > 0)
    delay(1);
  else
    delay(1);                        

  head += incr;
  if (head == NUMPIXELS){
    incr=-1;
  }
  else  if (head==0){ // sequence is finished
    triggered=0;
    strip.setPixelColor(head, 0); // first LED is still ON so off it goes
    strip.show();
    digitalWrite(PPSTEST,LOW);
    incr=1;
  }
}

// 
// Short (length)  pulse, two colours 
//
void mode1(){
  int i;

  strip.setPixelColor(tail, GREEN);
  strip.setPixelColor(tail+1, GREEN);

  for (i=tail+2;i<=head-2;i++){
    strip.setPixelColor(i, BLUE); 
  }

  strip.setPixelColor(head-1, GREEN);
  strip.setPixelColor(head, GREEN);

  strip.show();

  if (incr > 0){
    strip.setPixelColor(tail,0);
    delay(10);
  }
  else{
    strip.setPixelColor(head,0);
    //delay(1);
  }

  tail +=incr;
  head += incr;

  if (head == NUMPIXELS)
    incr = -1;
  else if (tail == 0){ // sequence is finished
    triggered=0;
    digitalWrite(PPSTEST,LOW);
    incr=1;
  }   
}

//
// Short pulse which changes colour
//
void mode2(){
  
  int i;

  for (i=tail;i<=head;i++){
    strip.setPixelColor(i,color); 
  }

  strip.show();// refresh strip

  delay(7);
  
  if (incr > 0){
    strip.setPixelColor(tail,0);
    if (tail % 3 == 0) 
      color >>= 1;
  }
  else{
    strip.setPixelColor(head,0);
    if (tail % 3 == 0) 
      color <<= 1;
  }

  tail +=incr;
  head += incr;

  if (head == NUMPIXELS){
    incr = -1;
    color = BLUE;
  }
  else if (tail == 0){ //sequence finished
    triggered=0;
    digitalWrite(PPSTEST,LOW);
    incr=1;
    color = GREEN;
    for (i=tail;i<=head;i++){ // show the last
      strip.setPixelColor(i,color); 
    }
    strip.show();// refresh strip
  }   
}

//
// Pulses which run from top and botton and meet in the centre, before returning
//
void mode3(){
  
  int i;

  for (i=tail;i<=head;i++){
    strip.setPixelColor(i,color); 
  }

  for (i=tail2;i<=head2;i++){
    strip.setPixelColor(i,color2); 
  }
  
  if (head >= tail2){ // overlapped
    for (i=tail2;i<=head;i++){
      strip.setPixelColor(i,GREEN | RED ); 
    }
    delay(5+(head-tail2)*10);
  }
  
  strip.show();

  delay(2);
  
  if (incr > 0){
    strip.setPixelColor(tail,0);
  }
  else{
    strip.setPixelColor(head,0);
  }

  if (incr2 > 0){
    strip.setPixelColor(tail2,0);
  }
  else{
    strip.setPixelColor(head2,0);
  }
  
  tail +=incr;
  head += incr;

  tail2 += incr2;
  head2 += incr2;
  
  if (head == NUMPIXELS/2+4)
    incr = -1;
  else if (tail == 0){
    incr=1;
    digitalWrite(PPSTEST,LOW);
    triggered=0;
    //temporary code
    tail2 = NUMPIXELS - 8;
    head2 = NUMPIXELS - 1;
    for (i=tail;i<=head;i++){
      strip.setPixelColor(i,color); 
    }
    for (i=tail2;i<=head2;i++){
      strip.setPixelColor(i,color2); 
    }
    strip.show();
    incr2 = -1;
  }
  
  if (tail2 == NUMPIXELS/2-4)
    incr2 = 1;
  else if (head2 == NUMPIXELS){
    incr2 = -1;
  } 
  
}

//
// Two pulses which start in the middle and run to top/bottom and back
//

void mode4(){
  
  int i;

  for (i=tail;i<=head;i++){ // pulse 1
    strip.setPixelColor(i,color); 
  }

  for (i=tail2;i<=head2;i++){ // pulse 2
    strip.setPixelColor(i,color2); 
  }
  
  if (head >= tail2){ // overlapping
    for (i=tail2;i<=head;i++){
      strip.setPixelColor(i,GREEN | RED ); 
    }
  }
  
  strip.show();
  if (tail == 0){
    delay(200);
  }
  else{
    delay(2);
  }  
  // erase the previpus pixels
  if (incr > 0){
    strip.setPixelColor(tail,0);
  }
  else{
    strip.setPixelColor(head,0);
  }

  if (incr2 > 0){
    strip.setPixelColor(tail2,0);
  }
  else{
    strip.setPixelColor(head2,0);
  }
  
  tail +=incr;
  head += incr;

  tail2 += incr2;
  head2 += incr2;
  
  if (tail == 0)
            incr = 1;
  else if (head == NUMPIXELS/2+4){
    incr=-1;
    digitalWrite(PPSTEST,LOW);
    triggered=0;
    tail2 = NUMPIXELS/2 - 4;
    head2 = tail2 + 7;
    for (i=tail2;i<=head+1;i++){ // add 1 to fudge around a bug
        strip.setPixelColor(i,GREEN | RED ); 
     }
    strip.show();
    incr2 = 1;
  }
  
  if (head2 == NUMPIXELS){
    incr2 = -1;
  } 
  
}

//
// IRQ0 - PPS interrupt service
//
void trigger() {
  triggered = 1;
  digitalWrite(PPSTEST,HIGH); // flash onboard LED for diagnostics
}

//
// IRQ1- toggles the display mode
//
void selectMode() {
  mode = mode + 1;
  if (mode > MAXMODE){
    mode = 0;
  }
  modeChanged=1;
}

