#include <Wire.h>


#define BUFTXSIZE 20
#define BUFRXSIZE 20
char bufTx[BUFTXSIZE];
uint8_t posTx = 0;
volatile uint8_t lengthTx = 0;

// This pin will be used for notify to the master that there are a message pending to send in this slave
uint8_t noficationTxPin = 4;

volatile bool isRxReceived = false;
char bufRx[BUFRXSIZE];
uint8_t posRx = 0;

volatile bool isTxEvent = false;


void setup() {
  Serial.begin(9600);
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent); // register event
  pinMode(noficationTxPin, OUTPUT);
  digitalWrite(noficationTxPin, LOW);
}

void loop() {
  uint8_t data;
  // Puts pin in low by default
  digitalWrite(noficationTxPin, LOW);
  if (Serial.available() > 0) {
    data = Serial.read();
    if (posTx == BUFTXSIZE)
    {
      bufTx[posTx - 1] = data;
      lengthTx = BUFTXSIZE;
    }
    else {
      bufTx[posTx++] = data;
      lengthTx = posTx;
    }

    // End of message
    if (data == '\n') {
      // Disable interrupts
      noInterrupts();
      // Terminanting char string
      bufTx[posTx] = 0;
      Serial.print("Slave Says: ");
      Serial.print(bufTx);

      // Send notification to the master
      digitalWrite(noficationTxPin, HIGH);      
      
      posTx = 0;
      // Enable interrupts
      interrupts();      
    }
  }

  if (isRxReceived)
  {
    noInterrupts();  
    char* bufRxTmp;
    bufRxTmp = bufRx;
    interrupts();
    
    Serial.print("Master Says: ");
    Serial.print(bufRxTmp);

    noInterrupts();    
    isRxReceived = false;
    interrupts();
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent() {
  
  while (Wire.available())
  {
    uint8_t data = Wire.read();    // receive byte as an integer

    if (data == 12)
    {
      isTxEvent = true;
    }
    else
    {
      if (posRx == BUFRXSIZE) bufRx[posRx - 1] = data;
      else
      {
        bufRx[posRx++] = data;
      }
      
      // End of message
      if (data == '\n') {
        // Terminanting char string
        bufRx[posRx] = 0;
        posRx = 0;
        isRxReceived = true;
      }
    }    
  }  
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  
  if (isTxEvent)
  {
    Wire.write(lengthTx);
    // Resets the length variable after the read
    lengthTx = 0;
    isTxEvent = false;
  }
  else
  {
     Wire.write(bufTx); // respond with message of lengthTx bytes as expected by master
  }
 
}
