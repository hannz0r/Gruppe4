////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////      Biblioteker og instanser       ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Tlc5940.h>        // Inkluderer Tlc5940 biblioteket
#include <Servo.h>          // Inkluderer Servo biblioteket

Servo servo;                // Lager en instans av typen servo
TLC_CHANNEL_TYPE channel;   // Setter Tlc-ens channel type til "channel"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////         Variabler       //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Definerer Button Grounds
const int buttonYellow = 41;    // swt-gnd1
const int buttonRed = 42;       // swt-gnd2
const int buttonBlue = 43;      // swt-gnd3
const int buttonGreen = 44;     // swt-gnd4

// Definerer LED Grounds
const int ledYellow = 31;       // led-gnd1
const int ledRed = 25;          // led-gnd2
const int ledBlue = 33;         // led-gnd3
const int ledGreen = 24;        // led-gnd4

// Definerer RGB pins
const int redIN = 5;
const int greenIN = 6;
const int blueIN = 7;

// Pinner fra andre arduino
const int input1 = A1;
const int input2 = A2;
const int input3 = A3;
const int input4 = A4;
int inputs[4] = {input1, input2, input3, input4};

// Pinner fra back end via arduino
const int input5 = A12;

// Pinner til andre arduino
const int output = A0;


// Pin til servo
const int servoPin = 2;
// Teller for å holde kontroll på hvilken case som skal benyttes
int nextLed = 0;
// Liste som samler opp første bokstaven på hver farge som blir trykt og deretter sammenligner med listen fra kartet
char pushedColourOrder[4]; 
// Sjekker om det er kommet et tastetrykk og registrerer kun ett 
boolean flag = false;
// Flag for å passe på at det ikke blir registrert samme farge to ganger etter hverandre.
// Et flag for hver farge.
bool colourFlag[] = {true, true, true, true};
// Et flag for servoen
// bool servoState= false;           *kommentert ut*
// Et flag for statusen til pinnene fra andre arduino
bool input[] = {false, false, false, false};
// Tall som indikerer hvilken "kommunikasjons-kanal" som har blitt benytta
int route;
// Et flag for statusen til kommunikasjonen mellom arduinoene
bool aFlag=true;

// Liste med farge-fasitsvar 
char rightColourOrder[4][4]=
{
  {'r','y','b','r'},                // Rute 1
  {'b','g','y','r'},                // Rute 2
  {'y','g','b','r'},                // Rute 3
  {'g','y','b','r'}                 // Rute 4
};


// Definerer fargene
int red[] = {255, 0, 0};
int green[] = {0, 255, 0};
int blue[] = {0, 0, 255};
int purple[] = {255, 0, 150};
int yellow[] = {255, 200, 0};
int dark[] = {0, 0, 0};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////       Setup       //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() { 
  Tlc.init(0);
  // Switch Grounds 
  pinMode(buttonYellow, INPUT);     //swt-gnd1
  pinMode(buttonRed, INPUT);        //swt-gnd2
  pinMode(buttonBlue, INPUT);       //swt-gnd3
  pinMode(buttonGreen, INPUT);      //swt-gnd4

  // Led grounds
  pinMode(ledYellow, OUTPUT);       //led-gnd1
  pinMode(ledRed, OUTPUT);          //led-gnd2
  pinMode(ledBlue, OUTPUT);         //led-gnd3
  pinMode(ledGreen, OUTPUT);        //led-gnd4

  // RGB pins
  pinMode(redIN, OUTPUT);
  pinMode(greenIN, OUTPUT);
  pinMode(blueIN, OUTPUT);
  // Pinnen markert med "switch" på 2x2 button-pad går til 5v
  
  // Setter status til input pinnene fra andre arduino
  for (int i = 0; i < 4; i++)
     pinMode(inputs[i], INPUT_PULLUP);

  // Kommunikasjon fra back end via arduino 1
//  pinMode(input5, INPUT_PULLUP);
     
  // Setter status til output pinnene til andre arduino 
  pinMode(output, OUTPUT);
  digitalWrite(output, HIGH);

  // Initialiserer servo-modulen
  servo.attach(servoPin);
  servo.write(180);

  // Initialiserer serial monitoring
  Serial.begin(9600);
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////     Loop     ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {


  for (int i = 0; i < 4; i ++){                  // Leter etter signal fra en av kommunikasjonspinnene
    input[i] = !digitalRead(inputs[i]);
  }

  for (int i = 0; i < 4; i ++){                  // Endrer flagstatus når det oppdages signal fra en av kommunikasjonspinnene
                                                 // Søker gjennom hvilken pin kommunikasjonen kommer fra og passerer informasjonen
                                                 // videre til "route" som peker på hvilken ytterste array som skal benyttes
                                                 // til farge-fasit i funksjonen "checkPassword"
    if (input[i] and aFlag){
      route = i;
      aFlag=false;
      break;
    }
  }

  if ( input[0] or input[1] or input[2] or input[3]){ // Hvis det blir oppdaget signal gjennom en av kommunikasjonspinnene,
                                                      // lyser button-pad opp, og er klar til å registrere tastetrykk 
  
    buttonColour(yellow, red, blue, green);       // Pass in the colour of your buttons 1, 2, 3, and 4.
  
    yellowButtonPushed();                         // Registrerer trykk på gul knapp og får en av ledsene til å lyse gult
    redButtonPushed();                            // Registrerer trykk på rød knapp og får en av ledsene til å lyse rødt
    blueButtonPushed();                           // Registrerer trykk på blå knapp og får en av ledsene til å lyse blått
    greenButtonPushed();                          // Registrerer trykk på grønn knapp og får en av ledsene til å lyse grønt
  
    //checkLastLed();                             // Sjekker om siste led lyser og om passordet er riktig
    if (nextLed == 4) {                           // så lenge siste trykket ikke har blitt gjort
    

    
      if (checkPassword()){                       // Bestemmer hva som skal skje når alle tastene har blitt trykket
                                                  // Hvis kombinasjonen er riktig åpnes sendes et signal til servoen
                                                  // og et signal tilbake til arduino 1
        Serial.println ("hurra!");
        
        digitalWrite (output,LOW);
        changeServoState();
        digitalWrite (output, HIGH);
        aFlag=true;
        }

      else{                                       // Dersom kombinasjonen er feil resetes flaggene, LED-lysene
                                                  // blir slått av og button-pad er igjen klar til å registrere nye trykk
        Serial.println("fail");
        delay(500);
        }
     
         
      ledsOff();
      for (int i=0; i<4; i++)
      pushedColourOrder [i]= ' ';
      resetColourFlag(); 
      }
  
    if(!(digitalRead(buttonGreen) or digitalRead(buttonBlue) or digitalRead(buttonRed) or digitalRead(buttonYellow))){
      flag = true;
     }
   }
  else {
    ledsOff();
  }
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////     Funksjoner     ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kontroll av individuelle LED
// Passer inn en fargeliste for hver LED
void buttonColour(int led1[], int led2[], int led3[], int led4[])
{
  analogWrite(redIN, led1[0]);
  analogWrite(greenIN, led1[1]);
  analogWrite(blueIN, led1[2]);
  // Flicker kontroll
  delay(2);
  digitalWrite(ledYellow, LOW);
  // Flicker kontroll
  delayMicroseconds(1100);
  digitalWrite(ledYellow, HIGH);

  analogWrite(redIN, led2[0]);
  analogWrite(greenIN, led2[1]);
  analogWrite(blueIN, led2[2]);
  // Flicker kontroll
  delay(2);
  digitalWrite(ledRed, LOW);
  // Flicker kontroll
  delayMicroseconds(1100);
  digitalWrite(ledRed, HIGH);

  analogWrite(redIN, led3[0]);
  analogWrite(greenIN, led3[1]);
  analogWrite(blueIN, led3[2]);
  // Flicker kontroll
  delay(2);
  digitalWrite(ledBlue, LOW);
  // Flicker kontroll
  delayMicroseconds(1100);
  digitalWrite(ledBlue, HIGH);

  analogWrite(redIN, led4[0]);
  analogWrite(greenIN, led4[1]);
  analogWrite(blueIN, led4[2]);
  // Flicker kontroll
  delay(2);
  digitalWrite(ledGreen, LOW);
  // Flicker kontroll
  delayMicroseconds(1100);
  digitalWrite(ledGreen, HIGH);

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funksjoner for hvert lys i hver farge
// Led 1
void led1yellow(){
  Tlc.set(1,128);
  Tlc.set(2,32);
  Tlc.update();
  nextLed ++;
}
void led1red(){
  Tlc.set(1,128);
  Tlc.update();
  nextLed ++;
}
void led1blue(){
  Tlc.set(3,128);
  Tlc.update();
  nextLed ++;
}
void led1green(){
  Tlc.set(2,128);
  Tlc.update();
  nextLed ++;
}

// Led 2
void led2yellow(){
  Tlc.set(4,128);
  Tlc.set(5,32);
  Tlc.update();
  nextLed ++;
}
void led2red(){ 
  Tlc.set(4,128);
  Tlc.update();
  nextLed ++;
}
void led2blue(){
  Tlc.set(6,128);
  Tlc.update();
  nextLed ++;
}
void led2green(){
  Tlc.set(5,128);
  Tlc.update();
  nextLed ++;
}

// Led 3
void led3yellow(){
  Tlc.set(7,128);
  Tlc.set(8,32);
  Tlc.update();
  nextLed ++;
}
void led3red(){ 
  Tlc.set(7,128);
  Tlc.update();
  nextLed ++;
}
void led3blue(){
  Tlc.set(9,128);
  Tlc.update();
  nextLed ++;
}
void led3green(){
  Tlc.set(8,128);
  Tlc.update();
  nextLed ++;
}

// Led 4
void led4yellow(){
  Tlc.set(10,128);
  Tlc.set(11,32);
  Tlc.update();
  nextLed ++;
}
void led4red(){ 
  Tlc.set(10,128);
  Tlc.update();
  nextLed ++;
}
void led4blue(){
  Tlc.set(12,128);
  Tlc.update();
  nextLed ++;
}
void led4green(){
  Tlc.set(11,128);
  Tlc.update();
  nextLed ++;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funksjon som slår av alle RGB-LED 
void ledsOff (){
  Tlc.setAll(0);
  Tlc.update();
  nextLed = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// For å resette flaggene for farger.
void resetColourFlag(){
  for (int i = 0; i < 4; i++)
    colourFlag[i] = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funksjoner for å registrere knappetrykk og skifte farge på LEDene
void yellowButtonPushed (){
  if (digitalRead(buttonYellow) and flag and colourFlag[0]){
    resetColourFlag();
    colourFlag[0] = false;
    flag = false;
    Serial.println("gul");    
    switch (nextLed){
      case 0:
        led1yellow();
        pushedColourOrder[0]='y';
        break;
      case 1:
        led2yellow();
        pushedColourOrder[1]='y';
        break;
      case 2:
        led3yellow();
        pushedColourOrder[2]='y';
        break;
      case 3:
        led4yellow();
        pushedColourOrder[3]='y';
        break;
    }
  }
}

void redButtonPushed(){
  if (digitalRead(buttonRed) and flag and colourFlag[1]){
    resetColourFlag();
    colourFlag[1] = false;
    flag = false;
    Serial.println("rod");    
    switch (nextLed){
      case 0:
        led1red();
        pushedColourOrder[0]='r';
        break;
      case 1:
        led2red();
        pushedColourOrder[1]='r';
        break;
      case 2:
        led3red();
        pushedColourOrder[2]='r';
        break;
      case 3:
        led4red();
        pushedColourOrder[3]='r';
        break;
    }
  }
}

void blueButtonPushed(){
  if (digitalRead(buttonBlue) and flag and colourFlag[2]){
    resetColourFlag();
    colourFlag[2] = false;
    flag = false;
    Serial.println("bla");    
    switch (nextLed){
      case 0:
        led1blue();
        pushedColourOrder[0]='b';
        break;
      case 1:
        led2blue();
        pushedColourOrder[1]='b';
        break;
      case 2:
        led3blue();
        pushedColourOrder[2]='b';
        break;
      case 3:
        led4blue();
        pushedColourOrder[3]='b';
        break;
    }
  }
}

void greenButtonPushed(){
  if (digitalRead(buttonGreen) and flag and colourFlag[3]){
    resetColourFlag();
    colourFlag[3] = false;
    flag = false;
    Serial.println("gronn");    
    switch (nextLed){
      case 0:
        led1green();
        pushedColourOrder[0]='g';
        break;
      case 1:
        led2green();
        pushedColourOrder[1]='g';
        break;
      case 2:
        led3green();
        pushedColourOrder[2]='g';
        break;
      case 3:
        led4green();
        pushedColourOrder[3]='g';
        break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funksjon for å lukke kisten når skuffen lukkes
/*
void closeChest(){
  if(digitalRead(!input5){
    servo.write(180);
  }
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funksjon for å sjekke om riktig kode er trykket
boolean checkPassword (){
  for( int i = 0; i < 4; i++){
    if(pushedColourOrder[i] != rightColourOrder[route][i]){
      return false;      
    }    
  }
  return true;
}  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funksjon for å endre servo-statusen og dermed hvorvidt "kista" er åpen eller ikke
void changeServoState() {
    servo.write(0);
    delay(9000);
    servo.write(180);
}
