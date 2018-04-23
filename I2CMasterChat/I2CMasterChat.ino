#include <Wire.h>


#define BUFTXSIZE 20
#define BUFRXSIZE 20
char bufTx[BUFTXSIZE];
uint8_t posTx = 0;

// This pin will be used by slave for notify master about new messages
uint8_t noficationRxPin = 2;
volatile bool isRxEvent = false;
void rxEvent()
{
  isRxEvent = true;
}


void setup() {
  // Arduino have pull-up resistor by default in the i2c bus https://forum.arduino.cc/index.php?topic=150646.0
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output

  pinMode(noficationRxPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(noficationRxPin), rxEvent, RISING);
}

void loop() {
  // Receiving serial message
  uint8_t data;
  if (Serial.available() > 0) {
    data = Serial.read();
    if (posTx == BUFTXSIZE) bufTx[posTx - 1] = data;
    else {
      bufTx[posTx++] = data;
    }

    // End of message
    if (data == '\n') {
      // Disable interrupts
      noInterrupts();      
      // Terminanting char string
      bufTx[posTx] = 0;
      // lengthTx = posTx;
      posTx = 0;
      // Enable interrupts
      interrupts();
      
      Serial.print("Master Says: ");
      Serial.print(bufTx);

      // Sends message request command to the slave
      Wire.beginTransmission(8); // transmit to device #8
      Wire.write(bufTx);        // sends message command      
      Wire.endTransmission(true);    // stop transmitting        
    }
  }

  // Receiving slave message
  if (isRxEvent)
  {
    noInterrupts();
    isRxEvent = false;
    interrupts();
    
    // Sends message request command to the slave
    Wire.beginTransmission(8); // transmit to device #8
    Wire.write(12);        // sends request message command
    Wire.endTransmission(false);    // stop transmitting and sending an restart message

    Wire.requestFrom(8, 1, false);    // request 1 bytes from slave device #8 (the length of the message)
    uint8_t lengthRx = 0;
    if (Wire.available())
    {
      lengthRx = Wire.read();
    }

    // ALERT: If slave message is greater than 
    Serial.print("Slave Says: ");
    Wire.requestFrom(8, lengthRx, true); // request lengthRx bytes from slave device #8 (the message)
    while (Wire.available()) { // slave may send less than requested
      char c = Wire.read(); // receive a byte as character
      Serial.print(c);         // print the character
    }
  }  
}



