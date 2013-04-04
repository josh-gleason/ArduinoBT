/*
    Bluetooth communication code

    Author: Josh Gleason
*/


/* Upload this sketch into Seeeduino and press reset*/

#include <SoftwareSerial.h>   //Software Serial Port

const int BTN_PIN = 4;
const int DC_BTN_PIN = 5;
const int BT_RXD = 6;
const int BT_TXD = 7;
const int BT_IO1 = A1;
const int LED_PIN = 12;
const int ACCEL_LED_PIN = 13;
const int ACCEL_PIN = A0;

// Blink interval in milliseconds
const int interval = 250;

#define DEBUG_ENABLED  1

// Global variables
SoftwareSerial blueToothSerial(BT_RXD, BT_TXD);

int btnState;
int prevBtnState;

int dcBtnState;
int prevDcBtnState;

double refVolt;
const double voltThresh = 0.016 * 65; // 30g
void setup() 
{
    Serial.begin(9600);
   
    // Set pin modes
    pinMode(BT_RXD, INPUT);
    pinMode(BT_TXD, OUTPUT);
    pinMode(BT_IO1, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(ACCEL_LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT);
    pinMode(DC_BTN_PIN, INPUT);
    
    digitalWrite(BT_IO1, LOW);

    // initialize bluetooth
    setupBlueToothConnection();

    // Get initial states of buttons
    btnState = prevBtnState = digitalRead(BTN_PIN);
    dcBtnState = prevDcBtnState = digitalRead(DC_BTN_PIN);

    // rely on delay from bluetooth setup
    int temp = analogRead(ACCEL_PIN);
    refVolt = temp / 1024.0 * 5;
}

void loop() 
{ 
    static long previousMillis = 0;
    static int ledState = LOW;
    static bool ledEnabled = false;

    unsigned long currentMillis = millis();

    // Process Bluetooth ///////////////////////////////
    char recvChar;
    if(blueToothSerial.available()){//check if there's any data sent from the remote bluetooth shield
        recvChar = blueToothSerial.read();
        // remove leading bit (dont know why it's there)
        unsigned char x = 0x7F;
        char val = (char)((unsigned char)(recvChar) & x);
        Serial.print(val);

        if ( val == 'a' ) {
            ledEnabled = !ledEnabled;
            // turn off LED
            if ( ledState == HIGH ) {
                ledState = LOW;
                digitalWrite(LED_PIN, ledState);
            }
        }
    }
    if(Serial.available()){//check if there's any data sent from the local serial terminal, you can add the other applications here
        recvChar  = Serial.read();
        unsigned char x = 0x80;
        char y = (char)((unsigned char)recvChar | x);
        blueToothSerial.print(recvChar);
    }

    // Process button presses ////////////////////////////
    btnState = digitalRead(BTN_PIN);
    dcBtnState = digitalRead(DC_BTN_PIN);

    if( prevBtnState == LOW && btnState == HIGH ) { // button pressed
        blueToothSerial.println("Button 1 Pressed");
    }

    if ( prevDcBtnState == LOW && dcBtnState == HIGH ) { // D/C Bluetooth button pressed
        blueToothSerial.println("Button 2 Pressed");
        digitalWrite(BT_IO1, HIGH);
    } else if ( prevDcBtnState == HIGH && dcBtnState == LOW ) {
        digitalWrite(BT_IO1, LOW);
    }

    prevDcBtnState = dcBtnState;
    prevBtnState = btnState;

    // Process LED blinking ///////////////////////////////
    if ( ledEnabled ) {
        // blink the LED.
        if(currentMillis - previousMillis > interval) {
            // save the last time you blinked the LED 
            previousMillis = currentMillis;   

            // if the LED is off turn it on and vice-versa:
            if (ledState == LOW)
                ledState = HIGH;
            else
                ledState = LOW;

            // set the LED with the ledState of the variable:
            digitalWrite(LED_PIN, ledState);
        }
    }

    // Read Accelerometer
    int temp = analogRead(ACCEL_PIN);
    double volts = (temp / 1024.0 * 5.0);
    if ( fabs(volts - refVolt) > voltThresh ) {
        Serial.println("Impact detected!");
        delay(200);
    }
    
} 

void setupBlueToothConnection()
{
    blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
    blueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
    blueToothSerial.print("\r\n+STNA=ARCNS\r\n"); //set the bluetooth name as "SeeedBTSlave"
    blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
    blueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
    delay(2000); // This delay is required.
    blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
    Serial.println("The slave bluetooth is inquirable!");
    delay(2000); // This delay is required.
    Serial.println("Two second delay!");
    blueToothSerial.flush();
}




