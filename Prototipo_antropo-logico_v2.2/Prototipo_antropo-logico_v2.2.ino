/* 2022-09-02
 * PROTOTIPO ANTROPO-LOGICO V2
 * DA UN OPERA DI GIACOMO VERDE
 * https://www.verdegiac.org/reperto.htm
 * RESTAURO DI LORENZO ANTEI (2022)
 * PER LA MOSTRA "LIBERARE ARTE DA ARTISTI"
 * PRESSO CAMeC LA SPEZIA
 * DAL 25 GIUGNO 2022 AL 15 GENNAIO 2023
 * 
 * See documentation at 
 * See License information at root directory of this library
 * Author: Brendan Doherty (2bndy5)
 */

/*
 * DOCUMENTAZIONE MODULO RF24L01 https://nRF24.github.io/RF24
 * FOTOCELLULA TELCOMA TILBINO SINCRO http://www.tege-torantriebe.com/pdf/ILBINO_V_03_2007.pdf
 * 
 * PINOUT TX (LEONARDO)
 * CE 10
 * CSN 9
 * SCK 9-PB1 ICSP (3di6)
 * MOSI 10-PB2 ICSP (4di6)
 * MISO 11-PB3 ICSP (1di6)
 * 
 * (note ICSP)
 * leggendoli con riferimento il verso del logo
 * 1 - MISO 11-PB3  |   2 - X
 * 3 - SCK 9-PB1    |   4 - MOSI 10-PB2
 * 5 - X            |   6 - X
 * 
 * PINOUT RX (NANO)
 * CE   10
 * CSN   9
 * SCK  13
 * MOSI 11
 * MISO 12
 */

#define radioNumber 0
// 0 -> LeonardoTX
// 1 -> nanoRX

bool debugMode = true;
 
#include <SPI.h>
#include "printf.h"
#include "RF24.h"

RF24 radio(10,9);  // instantiate an object for the nRF24L01 transceiver using 10 for the CE pin, and 9 for the CSN pin
uint8_t address[][6] = { "1Node", "2Node" }; // Let these addresses be used for the pair. It is very helpful to think of an address as a path instead of as an identifying device destination
bool role = true;  // Used to control whether this node is sending or receiving. true = TX role, false = RX role - NEL SETUP VIENE IMPOSTATO A SECONDA di radioNumber

//ogni trasmissione manda un float payload che varia tra 0 (
float payload = 0.0;
float lastPayload = 0.0;

#include "Keyboard.h"

void setup() {
  Serial.begin(115200);
  
  if (debugMode) {Serial.println("SETUP");}
  if ( radioNumber == 0) { //TX - LEONARDO
    if (debugMode) {Serial.println("TX - LEONARDO");}
    #define statusLedPin 13
    #define fotoCellulaPin 11 //Leonardo

    role = true;
    pinMode(statusLedPin, OUTPUT);
    digitalWrite(statusLedPin, LOW);
    pinMode(fotoCellulaPin, INPUT_PULLUP); //PULLUP --> un digitalPin ed un GND

    Keyboard.begin();
    
  } else { //RX - NANO
    if (debugMode) {Serial.println("RX- NANO");}
    role = false;
    #define relayPin 2 //10k ohm
    #define statusLedPin 8
    
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);
  }

  

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) { //warning loop
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
      }
  }
  
  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.
  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes
  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0
  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1
  // additional setup specific to the node's role
  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    radio.startListening();  // put radio in RX mode
  }
}  // setup

void loop() {

  if (role) {
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //TX - LEONARDO

    payload = digitalRead(fotoCellulaPin);
    if (debugMode) {Serial.println(payload);}  // print payload sent

    if (payload == 1) {
      //if (debugMode) {Serial.println("rilevata fotocellula - invio dati");}
      bool report = radio.write(&payload, sizeof(float));  // transmit & save the report

    } else {
      delay(300);
      //if (debugMode) {delay(1000);}
    }
    changeSlide();

  } else {
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // RX - NANO

    uint8_t pipe;
    if (radio.available(&pipe)) {              // is there a payload? get the pipe number that recieved it
      if (debugMode) {Serial.println("ricevo dati da TX");}
      digitalWrite(statusLedPin, HIGH);
      uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
      radio.read(&payload, bytes);             // fetch payload from FIFO
      digitalWrite(statusLedPin, LOW);
      if (debugMode) {Serial.println("fine ricevimento dati");}
      
    } else {
      payload = 0;
      if (debugMode) {Serial.println("non sto ricevendo dati...");}
    }
    if (payload == 1) {
        //Serial.println("giro");
        if (debugMode) {Serial.println("You Spin Me Round🎵 ♫🎵🎵");}
        digitalWrite(statusLedPin, HIGH);
        digitalWrite(relayPin, HIGH);
      } else {
        if (debugMode) {Serial.println("non ho ricevuto dati e quindi non giro...");}
        digitalWrite(statusLedPin, LOW);
        digitalWrite(relayPin, LOW);
        delay(50);
      }
      if (debugMode) {Serial.println("payload è ");}
      if (debugMode) {Serial.print(payload);}
      
  }

}  // loop

void changeSlide() {
  if (payload != lastPayload) {
      Keyboard.press('KEY_LEFT_ARROW');
      //delay(10); //REGOLARE
      Keyboard.releaseAll();
      lastPayload = payload;
    }
}
