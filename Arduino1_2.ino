//////////////////////////////////////////////////////////////////////////////////
///////////////////  Inkludering av forskjellige bibliotek  //////////////////////
//////////////////////////////////////////////////////////////////////////////////

#include <SPI.h>                        // Serial Pherifrial Interface bibliotek
#include <Tlc5940.h>                    // Tlc5940 extender bibliotek
#include <MFRC522.h>                    // RFID bibliotek
TLC_CHANNEL_TYPE channel;

//////////////////////////////////////////////////////////////////////////////////
/////////////////////  Definisjoner for RFID leseren  ////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#define SS_PIN 53                       // Definerer en Serial pin for RFID-leseren
#define RST_PIN 5                       // Drfinerer en Reset pin for  RFID-leseren
MFRC522 mfrc522(SS_PIN, RST_PIN);       // Lager instanse til RFID-leseren
byte cardPresent;                       // Mellomlagring for RFID avlesning


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////    Motstands RFID tags  ///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

byte ohm5[4]   = {0x50, 0xD4, 0xB3, 0x4F};
byte ohm10[4]  = {0xC0, 0xAE, 0xBB, 0x4F};
byte ohm20[4]  = {0xA0, 0xEE, 0xBF, 0x4F};
byte ohm25[4]  = {0x50, 0xEB, 0xBC, 0x4F};
byte ohm50[4]  = {0x40, 0x69, 0xBD, 0x4F};
byte ohm100[4] = {0x80, 0x96, 0xBF, 0x4F};


//////////////////////////////////////////////////////////////////////////////////
///////////////  Ruter betående av  en serie med RFID tags  //////////////////////
//////////////////////////////////////////////////////////////////////////////////

byte route_1[4][4] =
{
  {0x40, 0x69, 0xBD, 0x4F},   // 50 ohm
  {0xC0, 0xAE, 0xBB, 0x4F},   // 10 ohm
  {0xA0, 0xEE, 0xBF, 0x4F},   // 20 ohm
  {0x80, 0x96, 0xBF, 0x4F}    // 100 ohm
};

byte route_2[4][4] =
{
  {0x80, 0x96, 0xBF, 0x4F},   // 100 ohm
  {0x40, 0x69, 0xBD, 0x4F},   // 50 ohm
  {0xC0, 0xAE, 0xBB, 0x4F},   // 10 ohm
  {0xA0, 0xEE, 0xBF, 0x4F}    // 20 ohm
};

byte route_3[4][4] = 
{
  {0xA0, 0xEE, 0xBF, 0x4F},   // 20 ohm
  {0xC0, 0xAE, 0xBB, 0x4F},   // 10 ohm
  {0x80, 0x96, 0xBF, 0x4F},   // 100 ohm
  {0x40, 0x69, 0xBD, 0x4F}    // 50 ohm
};

byte route_4[4][4] =
{
  {0xC0, 0xAE, 0xBB, 0x4F},   // 10 ohm
  {0x80, 0x96, 0xBF, 0x4F},   // 100 ohm
  {0xA0, 0xEE, 0xBF, 0x4F},   // 20 ohm
  {0x40, 0x69, 0xBD, 0x4F}    // 50 ohm
};


///////////////////////////////////////////////////////////////////////////////////
///////////  Alle rutene kombinert til en 3-dimensjonell liste  ///////////////////
///////////////////////////////////////////////////////////////////////////////////

byte rightResistor[4][4][4]=
{
{
  {0x40, 0x69, 0xBD, 0x4F},   // 50 ohm
  {0xC0, 0xAE, 0xBB, 0x4F},   // 10 ohm
  {0xA0, 0xEE, 0xBF, 0x4F},   // 20 ohm
  {0x80, 0x96, 0xBF, 0x4F}    // 100 ohm
},
{
  {0x80, 0x96, 0xBF, 0x4F},   // 100 ohm
  {0x40, 0x69, 0xBD, 0x4F},   // 50 ohm
  {0xC0, 0xAE, 0xBB, 0x4F},   // 10 ohm
  {0xA0, 0xEE, 0xBF, 0x4F}    // 20 ohm
},
{
  {0xA0, 0xEE, 0xBF, 0x4F},   // 20 ohm
  {0xC0, 0xAE, 0xBB, 0x4F},   // 10 ohm
  {0x80, 0x96, 0xBF, 0x4F},   // 100 ohm
  {0x40, 0x69, 0xBD, 0x4F}    // 50 ohm
},
{
  {0xC0, 0xAE, 0xBB, 0x4F},   // 10 ohm
  {0x80, 0x96, 0xBF, 0x4F},   // 100 ohm
  {0xA0, 0xEE, 0xBF, 0x4F},   // 20 ohm
  {0x40, 0x69, 0xBD, 0x4F}    // 50 ohm
}
};


///////////////////////////////////////////////////////////////////////////////////
///////////////  Variabler brukt i forbindelse med blinking  //////////////////////
///////////////////////////////////////////////////////////////////////////////////

unsigned long previousMillis = 0;
unsigned long interval = 500;
int state = 0;
int tick = 0;
unsigned long currentMillis;

// bool ghettoFlag = false;

//////////////////////////////////////////////////////////////////////////////////
////////////////////////   Liste med fargekombinasjoner   ////////////////////////
//////////////////////////////////////////////////////////////////////////////////

char rightColourOrder[4][4]=
{
  {'r','y','b','r'},                // Rute 1
  {'b','g','y','r'},                // Rute 2
  {'y','g','b','r'},                // Rute 3
  {'g','y','b','r'}                 // Rute 4
};

int routeChoice;
int preventSameRoute;

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////   Kommunikasjons-pins   //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

const int trondheim = 49;           // Definerer en pin for "utgangspunkt" LED lyset 

const int output1 = A1;
const int output2 = A2;
const int output3 = A3;
const int output4 = A4;
int outputs[] = {output1, output2, output3, output4};
const int input = A0;               // Kommunikasjonspin fra Arduino 2

const int backEndReset = A15;       // Kommunikasjonspin til back end systemet
const int backEndClosed = A14;      // Kommunikasjonspin til hvorvidt skuffen er åpen fra back end
const int backEndSuccess = A13;     // Kommuniksajonspin til back end ved løst oppgave
const int output5 = A12;            // Kommiunikasjon mellom arduinoene


//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////   Setup    /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void setup() {
  Tlc.init();                       // Initialiserer Tlc extender
  Serial.begin(9600);               // Initialiserer serial kommunikasjon med PCen
  SPI.begin();                      // Initialiserer SPI bus
  mfrc522.PCD_Init();               // Initialiserer MFRC522 card (RFID leseren)
  randomSeed(analogRead(8));
  pinMode (trondheim, OUTPUT);
  digitalWrite(trondheim, HIGH);
  for (int i = 0; i < 4; i++){
    pinMode(outputs[i], OUTPUT);
    digitalWrite(outputs[i], HIGH);
  }
  pinMode(input, INPUT_PULLUP);

  
//  pinMode(backEndReset, INPUT);  
//  pinMode(backEndClosed, INPUT);
//  pinMode(backEndSuccess, OUTPUT);
//  pinMode(output5, OUTPUT, HIGH);
  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////        Selve kode-loopen       //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {


  currentMillis=millis();
  
  Serial.print(routeChoice);
  Serial.println("choice før");
  Serial.print(preventSameRoute);
  Serial.println("prevensjon før");
  
  pickRandomRoute();
  Serial.print(routeChoice);
  Serial.println("choice etter");
  Serial.print(preventSameRoute);
  Serial.println("prevensjon etter");
  
if((unsigned long)(currentMillis-previousMillis)>=interval){
  
  journey(rightResistor,routeChoice,rightColourOrder);
  
  previousMillis=currentMillis;
 }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////        Funksjoner        ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
///////////////////   Kode for sjekking av RFID-tags   /////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool SjekkRFID(byte rightTag[4][4][4],int choice,int ledNr){
  
/* 
 *  SjekkRFID leser RFID-tags og sjekker de opp mot en 3-dimensjonell liste (GoodTag[4][4][4]) 
 *  med RFID-kombinasjoner, og returnerer "true" dersom den leste RFIDen stemmer med listen.
 *  
 *  
 *  int choice   -      Tall som bestemmer hvilken ytterste 
 *  
 *  bool AllGood1 & 2 - True/false variabeler som brukes til å si hvorvidt løsningen 
 *                      er riktig eller ikke.
 *                      AllGood settes til usann inntil det motsatte er bevist.
 */
  bool AllGood1=false;
  bool AllGood2=false;
  
// Ser etter nye RFID brikker
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

// Velger en RFID brikke
if ( ! mfrc522.PICC_ReadCardSerial()) {
  return false;
}
// Viser UIDen til RFID brikken (til debugging)
        for (byte i = 0; i < mfrc522.uid.size; i++) {
                //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                //Serial.print(mfrc522.uid.uidByte[i],HEX);
                
//                    ghettoFlag=true;
                
// går igjennom brikkens ID, bit for bit og sjekker den opp mot en "fasit"
                for(int j=0;j<3;j++){
                  if ((rightResistor[choice][ledNr][j]!=mfrc522.uid.uidByte[j])){
                Serial.print(rightResistor[choice][ledNr][j]);
                Serial.print(" ");
                Serial.print(mfrc522.uid.uidByte[j]);
                Serial.println("");
                    break;
                    }
                
// hvis hele brikkens ID stemmer med fasiten, sett AllGood1 flagget til "true"
                  if(j==2){
                    AllGood1=true;                     
                  }
              }
// går igjennom brikkens ID motsatt vei og sjekker den opp mot en "fasit"
                for(int j=0;j<3;j++){
                  if ((rightResistor[choice][ledNr][2-j]!=mfrc522.uid.uidByte[j])){
                    break;
                    }
                
// hvis hele brikkens ID stemmer med fasiten, sett AllGood2 flagget til "true"
                  if(j==2){
                    AllGood2=true; 
                  }
              }

        }
//        ghettoFlag=false; 
//        Serial.println(AllGood1);            // Serial printing for debugging
        return AllGood1;
        
 
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void pickRandomRoute(){
   routeChoice=random(4);
   if (routeChoice==preventSameRoute){
    Serial.println("fikser rute");
    pickRandomRoute();
   }
   preventSameRoute=routeChoice;
}


///////////////////////////////////////////////////////////////////////////////
/////////////  Kode for å bestemme LED, farge og intensitet  //////////////////
///////////////////////////////////////////////////////////////////////////////

void setLedColour (int led, char colour, int intensity){

/* 
 * int led -    Tall som brukes i bestemmelsen av hvilken LED som skal lyse 
 * char colour -   Bokstav som bestemmer hvilken farge den skal ha (r,g,b,y,w,d)
 *                 hvor r = rød
 *                      b = blå
 *                      g = grønn
 *                      y = gul  (yellow)
 *                      w = hvit (white)
 *                      d = mørk (ingen farge) (dark)
 *                      
 * int intensity - Tall som angir intensiteten til LED på en skala fra 0-4095
 * int finish -    Tall for å korrigere for plasseringen på LED i forhold,
 *                 til extenderen. 
 *                 Brukes også til slutt til å angi hvilken LED som skal lyse.
 */
 
  int finish = led*3;
  switch(colour){
    case 'r':
    break;
    
    case 'g':
    finish=finish+1;
    setLedColour(led,'r',0);
    break;

    case 'b':
    finish=finish+2;
    setLedColour(led,'r',0);
    intensity=intensity*6;
    break;

    case 'y':
      setLedColour(led,'r', intensity);
      setLedColour(led, 'g', intensity/3);
    break;

    case 'w':
      setLedColour(led, 'r', intensity*2);
      setLedColour(led, 'g', intensity*2);
      setLedColour(led, 'b', intensity/7);
      break;
    case 'd':
      setLedColour(led, 'r', 0);
      setLedColour(led, 'g', 0);
      setLedColour(led, 'b', 0);
      break;

    }
/*
 * Disse brukes utelukkende til å korrigere for plasseringen,
 * av LED lysene i forhold til extenderene
 */
    if (led>4)
      finish+=4;
    if (led>8)
      finish+=4;
    if (led>12)
      finish+=4;
/*
 * Sender selve informasjonen til biblioteksfunksjonen,
 * Tlc.set, som får LED-lysene til å lyse
 */
    Serial.println(finish-2);
    Tlc.set(finish-2,intensity);

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
////////////////////  Kode for å blinke LED i hvitt  //////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ledBlink(int route, int intensity, int onOff, int start){
/*
 * int start     - Tall som brukes til å iterere lysblinkene fra en led til neste
 * 
 * int route     - Tall som sier hvilken extender å starte fra
 * 
 * int number    - Tall som brukes i bestemmelsen av hvilken LED som skal lyse
 * 
 * int intensity - Tall som brukes for å definere intensiteten til LED-lyset
 * 
 * int onOff     - Tall som brukes som en på/av status i samarbeid med den
 *                 globale variablen (int state). 
 * 
 * switch funksjonen bruker "setLedColour"-funksjonen i samarbeid med (int onOff)
 * og (int state) til å alternere en LED mellom  å lyse hvitt (w) og mørkt (d)
 */
 //if (!ghettoFlag){
    currentMillis=millis();
 
    if((unsigned long)(currentMillis-previousMillis)>=interval){
     int number=1+start;
     number += route*4;
      switch (onOff){
        case 0:
          state+=1;
          setLedColour(number,'w',intensity);
         break;
       case 1:
         state-=1;
         setLedColour(number,'d',0);
         break;
      }
      previousMillis=currentMillis; 
    }

// }
 }
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
void closedDrawer(){
  if (digitalRead(backEndClosed==HIGH){
    digitalWrite(output5, LOW);
  }
}
*/
///////////////////////////////////////////////////////////////////////////////
///////////////////  Kode for å tilfeldiggjøre farger  ////////////////////////
///////////////////////////////////////////////////////////////////////////////

char randColour(){
/*
 *char randColour() - En bokstav som tilfeldiggjøres ved hjelp av en innebygget
 *                    randomize-funksjon.
 *                    hvor r = rød
 *                         b = blå
 *                         g = grønn
 *                         y = gul (yellow)
 *
 *int choice - Tall som benyttes i switch-funksjonen for å bestemme fargeutfallet.
 *             Den valgte fargen blir så returnert til hovedloopen.
 */
 
  int choice=random(4);
  switch (choice){
    case 0:
    return 'r';
    break;

    case 1:
    return 'g';
    break;

    case 2:
    return 'b';
    break;

    case 3:
    return 'y';
    break;
  }
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void partyTime(){
  Tlc.setAll(0);
  Tlc.update();
  for(int i=1;i<17;i++){
    setLedColour(i,randColour(),128);
    delay(50);
    Tlc.update();
    delay(50);
  }
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////  Kode for valg av rute  /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool journey(byte route[4][4][4],int extender,char list[4][4]){ 
/*
 * byte route[4][4][4] - 3-dimensjonell liste med hexadecimale RFID-verdier som 
 *                       angir hvilke RIFD-verdier SjekkRFID-funksjonen skal 
 *                       benytte som fasit ved RFID sjekking
 *                       
 * int extender        - Tall (0-4) som angir hvilken PWM extender som skal benyttes
 * 
 * char list[4][4]     - 2-dimensjonell liste med char bokstaver
 *                       Brukes som predefinert fargekombinasjon til fargesetting av 
 *                       Led-lysene i setLedColour-funksjonen                      
 */
 
  for (byte j = 0; j <4;) {     
    ledBlink(extender,128,state,j);
      if (SjekkRFID(rightResistor[extender][4][4],extender,j)==true){ 
        j++;
        setLedColour(extender*4+j,'d',128);
        Tlc.update();
        setLedColour(extender*4+j,list[extender][j-1],127);
        } 
    if (j>3){
      Serial.println(" Gratulerer!!");
      digitalWrite(outputs[extender], LOW);
      while (digitalRead(input)){
        ;
      }
     }
   Tlc.update(); 
   }  
 for (int i=0;i<5;i++){
  partyTime();
 }

Tlc.setAll(0);
Tlc.update();
digitalWrite(outputs[extender], HIGH);
} 
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
