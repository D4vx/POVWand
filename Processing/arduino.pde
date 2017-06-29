

// INITIALIZE SERIAL PORT 
void initPort()
{
  String[] ports = Serial.list();
  JComboBox cbxPortList = new JComboBox(ports);
  JOptionPane.showMessageDialog(this, cbxPortList, "Select a Serial Communication Port", JOptionPane.PLAIN_MESSAGE);
  String selectedPort = (String) cbxPortList.getSelectedItem();
  serial = new Serial(this, selectedPort, 19200);
  //serial = new Serial(this, "/dev/tty.usbmodem1411", 19200);  
}


// FUNCTION TO BLAST THE PATTERN TO THE ARDUINO

void blastPattern()
{
  // blast the bit pattern to the arduino
  // send it one row at a time
  println("DATA OUTPUTTING TO ARDUINO");
  index = 0;
  // start by sending header character 
  // begin sending one column at a time... wait for confirmation of each column before 
  // moving onto the next 

  for(int x = 0 ; x < patternWidth; x++)
  {
    serial.write(ROWTRANSMIT);
    for(int y= patternHeight - 1; y>=0; y--)
    {
      serial.write(bitPattern[x][y]);
      delay(5);
    }
    serial.write(COLUMNTRANSMIT);
    while(!rowDone) { }
  }

}


// Will wait for a serial event to occur before executing the code here
void serialEvent(Serial p)
{
  // wait for the confirmation of reciept from Arduino before moving to next row
  char readString = p.readChar();
  print(readString); // echo to the console
  if(readString == RECEIPT)
  {
    rowDone = !rowDone;
    println();
  }
}

  



