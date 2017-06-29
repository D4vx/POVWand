// PHYSICAL COMPUTING PROJECT //
// Purpose      : Recieves bit pattern from processing and display on the POV wand
//              : wand blasts its way through a binary bit pattern representing the leds on the wand
// Author       : Dave Brown
// Student ID   : 1393595
// Date         : 29/10/2013


// Code inspired by : http://www.instructables.com/id/Persistence-of-Vision-Wand/


// include that allows for direct port manipulation
#include <avr/pgmspace.h>

// recieved characters 
#define TERMINATOR '<'
#define HEADER '>'

// setup holding pattern for the bit data
byte refreshRate = 2;    // time between each row transition .. may need tweaking for readability

const byte patternWidth = 90;  
const byte patternHeight = 16;
const int patternSize = (int)patternWidth * (int)patternHeight;  // 90 x 16

boolean bitPatternFromProcessing[patternSize]; // storage for recieved bit pattern

// incoming data storage for the bit pattern 
byte data1 = 0;
byte data2 = 0;
byte data3 = 0;

// counters
int index = 0;
int cols = 0;
int rows = 0;

// serial byte
byte inByte = 0;

// logic state variables
boolean started;
boolean finished; 
boolean hasRead = true;
boolean showBuffer = false;
boolean blast = false;

// using Port Registers to set the values on the ports
// PORTB = digital pins 8 - 13 (6 pins)
// PORTC = analog pins 0 - 5  (6 pins) .. will only be using 4
// PORTD = digital pins 0 - 7 (8 pins) .. wont be using 0 and 1 as they are TX and RX
// PORT registers control the high and low state
// DDR register controls input / output

// cannot use D0 or D1 as they are needed for serial communication

// bit pattern to check all LEDs are working
boolean test[] = {
  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //D2
  0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //D3
  0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0, //D4
  0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0, //D5
  0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0, //D6
  0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0, //D7
  0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0, //D8
  0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0, //D9
  0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0, //D10
  0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0, //D11
  0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0, //D12
  0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0, //D13
  0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0, //A0
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0, //A1
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0, //A2
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1  //A3

};

// using information from the PortManipulation documentation on www.arduino.cc

void setup() {

  Serial.begin(19200);
  // set all 16 pin modes

  // Analog pins A0 - A3  
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);


  // Digital pins D2 - D13
  for(int pin=2; pin<14; pin++) {
    pinMode(pin, OUTPUT);
  }

  // initialize routine to test LEDs are all working

  for(int i =0; i<16; i++) // columns
  {
    // PORTD - Digital pins 2 - 7
    for(int k=5; k>=0; k--) // only want 6 bits
    {
      data1 = data1 << 1;   // bit shift to the left 1 byte
      data1 |= test[(k * patternHeight + i)]; // basic x, y conversion through a linear array
    }
    // shift the bits in data1 2 places to the left as I dont want to address pins 0 and 1
    data1 = data1 << 2;

    // PORTB - Digital pins 8 - 13
    for(int k=11; k>=6; k--) // only want 6 bits.. bit 6 and 7 are not addressable so no need to bit shift them out
    {
      data2 = data2 << 1;
      data2 |= test[(k * patternHeight + i)];
    }

    // PORTC - Analog pins 0 - 3
    for(int k=16; k>=12; k--) // the remaining 6 bits
    {
      data3 = data3 << 1;
      data3 |= test[(k * patternHeight + i)];
    }
    
    // send data to pins
    PORTD = data1;
    PORTB = data2;
    PORTC = data3;
    delay(100);
  }
  
  // clear data buffers 
  data1 = 0;
  data2 = 0;
  data3 = 0;
  
  // send data to pins
  PORTD = data1;
  PORTB = data2;
  PORTC = data3;

  getPattern(); // sets the holding pattern
}



void getPattern()
{
  // for now I will use the pattern held in the load array
  for(int i = 0; i < 256; i++)
  {
    bitPatternFromProcessing[i] = test[i];
  }
}



void loop() 
{
  if(blast)
  {    
    blastToWand(bitPatternFromProcessing);
  }
  if(showBuffer)
  {
    for(int i = 0; i < patternWidth; i++) //90
    {
      for(int k = 0; k < patternHeight; k++) //16
      {
        int loc = k * patternWidth + i;
        Serial.print(bitPatternFromProcessing[loc]);
      }
      Serial.println();
    }  
    // reset the row / column counters
    cols = 0;
    rows = 0;
    showBuffer = !showBuffer; 
  }
  //delay(1000); // delay of each pattern 
}   


// function that occurs only when a serial event is detected
// checks for special characters sent from processing and echos back to processing console
void serialEvent()
{
  // check for serial data coming in
  blast = false;
  while(Serial.available() > 0)
  {
    inByte = Serial.read();  // read one byte at a time
    
    if(inByte == '-')
    {
      // start of row pattern transfer 
      started = true;
      finished = false;
      Serial.print("-START- ");       
    }
    else if (inByte == '|')
    {
      // recieved the first column of data
      Serial.print(" |END|");
      finished = true;
      rows++;
      break;
    }
    else
    {
      // store data in the array at y * arrayWidth + x
      // y = cols
      // x = rows
      int loc = cols * patternWidth + rows;  // need to confirm this
      bitPatternFromProcessing[loc] = inByte;
      Serial.print(inByte); 
      cols++;
    }
  }

  if(started && finished) // serial transmission started and finished
  {
    cols = 0; // reset column counter
    // send confirmation back to processing to get next column
    Serial.write('<');
    started = false;
    finished = false;
    if(rows >= patternWidth)
    {
      // allow for the storage buffer to echo to the console
      showBuffer = !showBuffer;
      blast = true;
    }
  }
}


// Sends the passed boolean bit pattern to the wand for processing
void blastToWand(boolean bitPattern[])
{
  for(int i = 0; i < patternWidth; i++) // for each column of the pattern
  {
    // read the column and build the binary pattern in data1, data2 and data3

    

    // PORTC - Analog pins 0 - 3
    for(int k= 0; k < 4; k++)
    {
      data1 = data1 << 1;   // bit shift to the left 1 byte
      data1 |= bitPattern[(k * patternWidth + i)]; // basic x, y conversion through a linear array
    }


    // PORTB - Digital pins 8 - 13
    // only want 6 bits.. bit 6 and 7 are not addressable so no need to bit shift them out
    for(int k = 4; k < 10; k++)
    {
      data2 = data2 << 1;
      data2 |= bitPattern[(k * patternWidth + i)];
    }

    // PORTD - Digital pins 2 - 7
    // only want 6 bits
    for(int k = 10; k < 16; k++)
    {
      data3 = data3 << 1;
      data3 |= bitPattern[(k * patternWidth + i)];
    }
    // shift the bits in data1 2 places to the left as I dont want to address pins 0 and 1

    data3 = data3 << 2; 
    // first column of the array is loaded into the 3 bytes of data 
    // using direct port manipulation to set the pin states


    
    PORTC = data1;
    PORTB = data2;
    PORTD = data3;

    delay(refreshRate); // delay between columns

    // reset the storage bytes
    data1 = 0;
    data2 = 0;
    data3 = 0;
  }

}

