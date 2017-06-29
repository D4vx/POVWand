// PHYSICAL COMPUTING PROJECT //
// Purpose    : Takes manual input from a user in the form of hand writing
//              Converts to a bit pattern and sends to arduino 
//              Displays on the POV wand
// Author     : Dave Brown
// Student ID : 1393595
// Date       : 29/10/2013



// include libraries
import processing.serial.*;
import javax.swing.*;

// characters that get transmitted to the Arduino
public static final char ROWTRANSMIT = '-';
public static final char COLUMNTRANSMIT = '|';
public static final char RECEIPT = '<';

Serial serial;

int videoScale = 10;     // factor to scale the video down
int patternWidth = 90;   // width of the pattern
int patternHeight = 16;  // height of the pattern

// store the 100 x 20 bitpattern in this array
byte[][] bitPattern = new byte[patternWidth][patternHeight];

// logic variables
boolean rowDone = false;
boolean bitRecieved = false;
boolean ready = false;

// counter 
int index = 0;



void setup() {
 size(patternWidth*videoScale, patternHeight*videoScale);
 //size(900, 160);
 background(0);
 
 // initialize the serial port
 initPort();
}


void draw() {
 stroke(255);
 strokeWeight(1);
 
 // Draw if mouse is pressed
 if (mousePressed) 
 {
   line(pmouseX, pmouseY, mouseX, mouseY);
 } 
 
 if(ready) 
 {
   PImage img = createImage(width, height, ALPHA); 
   loadPixels();
   img.pixels = pixels;                               // screen capture to PImage
   img.resize(width/videoScale, height/videoScale);   // resize to 90 x 16 for easy pixel calculation
   img.updatePixels();
   
   // convert image to bit pattern 
   println("width : " + img.width + " height : " + img.height);
   for(int x = 0; x < img.width; x++) 
   {
     for(int y = 0; y < img.height; y++) 
     {
       int loc = x + y*img.width;  // standard x,y traversal through linear array
       
       // TEST THE PIXELS //
       
       // every black pixel is converted to a 0
       if(img.pixels[loc] == color(0)) 
       { 
         bitPattern[x][y] = 0;
       }
       else 
       // every white pixel is converted to a 1
       {
         bitPattern[x][y] = 1;
       }       
     }
   }
   
   // ECHO OUTPUT TO CONSOLE
   
   for(int y = 0; y < img.height; y++) 
   {
      for(int x = 0; x < img.width; x++) 
      {
          print(bitPattern[x][y]);
      }
      println();        
   }
     
   // BLAST BIT PATTERN OFF TO ARDUINO POV WAND

   blastPattern();
   
   ready = !ready;
   }   
}

// KEYPRESS WILL BLAST THE PATTERN 

void keyPressed()
{
  if(!ready){
    ready = !ready;
  } 
}
  
