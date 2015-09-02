// based on
// Dawn & Dusk controller. http://andydoz.blogspot.ro/2014_08_01_archive.html
// 16th August 2014.
// (C) A.G.Doswell 2014
// adapted sketch by niq_ro from http://nicuflorica.blogspot.ro & http://arduinotehniq.blogspot.com/
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
//
// Time is set using a rotary encoder with integral push button. The Encoder is connected to interrupt pins D2 & D3 (and GND), 
// and the push button to pin analogue 0 (and GND)
// The RTC is connections are: Analogue pin 4 to SDA. Connect analogue pin 5 to SCL.
// A 2 x 16 LCD display is connected as follows (NOTE. This is NOT conventional, as interrupt pins are required for the encoder)
// 
// Use: Pressing and holding the button will enter the clock set mode (on release of the button). Clock is set using the rotary encoder. 

#include <Wire.h>
#include "RTClib.h" // from https://github.com/adafruit/RTClib
#include <LiquidCrystal.h>
#include <Encoder.h> // from http://www.pjrc.com/teensy/td_libs_Encoder.html
#include <EEPROM.h>  // http://tronixstuff.com/2011/03/16/tutorial-your-arduinos-inbuilt-eeprom/


RTC_DS1307 RTC; // Tells the RTC library that we're using a DS1307 RTC
//Encoder knob(2, 3); //encoder connected to pins 2 and 3 (and ground)
Encoder knob(5, 6); //encoder connected to pins 5 and 6 (and ground)

// initialize the library with the numbers of the interface pins
 LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
/*                                    -------------------
                                      |  LCD  | Arduino |
                                      -------------------
 LCD RS pin to digital pin D7         |  RS   |   D7    |
 LCD Enable pin to digital pin D8     |  E    |   D8    |
 LCD D4 pin to digital pin D9         |  D4   |   D9    |
 LCD D5 pin to digital pin D10        |  D5   |   D10   |
 LCD D6 pin to digital pin D11        |  D6   |   D11   |
 LCD D7 pin to digital pin D12        |  D7   |   D12   |
 LCD R/W pin to ground                |  R/W  |   GND   |
                                      -------------------
*/

//the variables provide the holding values for the set clock routine
int setyeartemp; 
int setmonthtemp;
int setdaytemp;
int sethourstemp;
int setminstemp;
int setsecs = 0;
int maxday; // maximum number of days in the given month

// These variables are for the push button routine
int buttonstate = 0; //flag to see if the button has been pressed, used internal on the subroutine only
int pushlengthset = 2000; // value for a long push in mS
int pushlength = pushlengthset; // set default pushlength
int pushstart = 0;// sets default push value for the button going low
int pushstop = 0;// sets the default value for when the button goes back high

float knobval; // value for the rotation of the knob
boolean buttonflag = false; // default value for the button flag
byte pauza = 15;  // time beetween work at encoder

// http://arduino.cc/en/Reference/LiquidCrystalCreateChar
byte grad[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
};

#include <DHT.h>
#define DHTPIN A1     // what pin we're connected DHT11/22
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22

DHT dht(DHTPIN, DHTTYPE);
float t1;

// for DS18B20 - http://arduinoprojects.ru/2014/08/%D0%BF%D1%80%D0%BE%D1%81%D1%82%D0%BE%D0%B9-%D1%82%D0%B5%D1%80%D0%BC%D0%BE%D1%81%D1%82%D0%B0%D1%82-%D0%BD%D0%B0-arduino-%D0%B8-%D1%86%D0%B8%D1%84%D1%80%D0%BE%D0%B2%D0%BE%D0%BC-%D1%82%D0%B5%D1%80%D0%BC/
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

byte heat;  // is 1 for heater and 0 for cooler
byte dry;   // is 1 for dryer and 0 for wetter 

// define pins for relay or leds
#define temppeste 4
#define umidpeste 3

// define variable

float te, tunu, tes, dete, dete1;   // variable for temperature
int has, hass, dehas;        // variable for humiditiy

/*
http://www.instructables.com/id/Custom-Large-Font-For-16x2-LCDs/?ALLSTEPS
A set of custom made large numbers for a 16x2 LCD using the 
 LiquidCrystal librabry. Works with displays compatible with the 
 Hitachi HD44780 driver. 
Made by Michael Pilcher - 2/9/2010
customised by niq_ro: http://nicuflorica.blogspot.ro/2014/01/afisaj-lcd16x2-cu-caractere-mari-fata.html
*/
// the 8 arrays that form each segment of the custom numbers
byte LT[8] = 
{
  B00111,
  B01111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte UB[8] =
{
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte RT[8] =
{
  B11100,
  B11110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
byte LL[8] =
{
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B01111,
  B00111
};
byte LB[8] =
{
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};
byte LR[8] =
{
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11110,
  B11100
};
byte UMB[8] =
{
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111
};
byte LMB[8] =
{
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};

int x = 0;




void setup () {
    //Serial.begin(57600); //start debug serial interface
    Wire.begin(); //start I2C interface
    RTC.begin(); //start RTC interface
dht.begin();  // DHT init 
sensors.begin();   // for DS18B20 sensor
  lcd.begin(20,4); //Start LCD (defined as 16 x 2 characters)
  lcd.createChar(0, grad);  // create custom symbol
  
    // assignes each segment a write number
  lcd.createChar(8,LT);
  lcd.createChar(1,UB);
  lcd.createChar(2,RT);
  lcd.createChar(3,LL);
  lcd.createChar(4,LB);
  lcd.createChar(5,LR);
  lcd.createChar(6,UMB);
  lcd.createChar(7,LMB);
  
  
  lcd.clear(); 
    pinMode(A0,INPUT);//push button on encoder connected to A0 (and GND)
    digitalWrite(A0,HIGH); //Pull A0 high
//    pinMode(A3,OUTPUT); //Relay connected to A3
//    digitalWrite (A3, HIGH); //sets relay off (default condition)
    
    //Checks to see if the RTC is runnning, and if not, sets the time to the time this sketch was compiled.
    if (! RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
 
// define outputs
 pinMode(temppeste, OUTPUT);  
 pinMode(umidpeste, OUTPUT);

// set the default state for outputs
  digitalWrite(temppeste, LOW);
  digitalWrite(umidpeste, LOW);


/*
// preset value for temperature and humidity
tes = 28.9;   
dete = 0.2;
hass = 58;
dehas = 2;
heat = 1;  // is 1 for heater and 0 for cooler
dry = 1;   // is 1 for dryer and 0 for wetter 
*/

/*
// just first time... after must put commnent (//)
EEPROM.write(201,0);    // tset1
EEPROM.write(202,245);  // tset2
EEPROM.write(203,5);    // dt x 10
EEPROM.write(204,45);   // hass
EEPROM.write(205,5);    // dehas
EEPROM.write(206,1);    // is 1 for heater and 0 for cooler
EEPROM.write(207,1);    // is 1 for dryer and 0 for wetter 
*/
  
byte tset1 = EEPROM.read(201);
byte tset2 = EEPROM.read(202);
tes = 256 * tset1 + tset2;  // recover the number
tes = tes/10;
dete = EEPROM.read(203);
dete = dete/10;
hass = EEPROM.read(204);
dehas = EEPROM.read(205);
heat = EEPROM.read(206);
dry = EEPROM.read(207);  
   
}
           

void loop () {

   int h = dht.readHumidity();
//  float t = dht.readTemperature();
  
   // 11 bit resolution by default 
  // Note the programmer is responsible for the right delay
  // we could do something usefull here instead of the delay
  int resolution = 11;
  sensors.setResolution(resolution);
//  delay(150/ (1 << (12-resolution)));
 
      pushlength = pushlengthset;
    pushlength = getpushlength ();
    delay (10);
    if (pushlength <pushlengthset) {
      ShortPush ();   
    }

 
 // using DS18B20 ( http://arduinoprojects.ru/2014/08/%D0%BF%D1%80%D0%BE%D1%81%D1%82%D0%BE%D0%B9-%D1%82%D0%B5%D1%80%D0%BC%D0%BE%D1%81%D1%82%D0%B0%D1%82-%D0%BD%D0%B0-arduino-%D0%B8-%D1%86%D0%B8%D1%84%D1%80%D0%BE%D0%B2%D0%BE%D0%BC-%D1%82%D0%B5%D1%80%D0%BC/ )
sensors.requestTemperatures(); // запрос на получение температуры
te = (sensors.getTempCByIndex(0)); 
//  delay(150/ (1 << (12-resolution)));
  
   
    DateTime now = RTC.now(); //get time from RTC
    
     byte ore = now.hour();
     byte minuti = now.minute();
     byte secunzi = now.second();
if (secunzi == 0) lcd.clear();

    //Display current time
    lcd.setCursor (0,2);
    lcd.print(now.day(), DEC);
    lcd.print('/');
    lcd.print(now.month());
    lcd.print('/');
    lcd.print(now.year(), DEC);
    lcd.print(" ");
//    lcd.setCursor (0,3);

 //  lcd.setCursor(11, 1);
     lcd.setCursor(13, 2);
  // lcd.print("t=");
 if (te < 0) 
 {t1=-te;}
 else t1=te;
  
    if ( t1 < 10)
   {
     lcd.print(" "); 
   }
   if (te>0) lcd.print("+"); 
   if (te==0) lcd.print(" "); 
   if (te<0) lcd.print("-");
 //  lcd.print(t1,0);
     lcd.print(t1,1);
   //lcd.print(",0");
   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("C");
    

   lcd.setCursor(15, 3);
  // lcd.print("H=");
   lcd.print(h);
   lcd.print("%RH"); 


/*    
//    lcd.print(now.hour(), DEC);
//    byte ore = now.hour();
    lcd.print(ore);
    lcd.print(':');
//    byte minuti = now.minute();
//    if (now.minute() <10) 
      if (minuti < 10)
      {
        lcd.print("0");
      }
  
//  lcd.print(now.minute(), DEC);
    lcd.print(minuti);
    lcd.print(':');

//    byte secunzi = now.second();
//    if (now.second() <10) 
    if (secunzi <10) 
      {
        lcd.print("0");
      }
//    lcd.print(now.second());
    lcd.print(secunzi);
//    lcd.print("     ");
    
//lcd.setCursor (0,3);
//temperaturi(has);
*/


lcd.setCursor (0,0);
lcd.print("                ");
lcd.setCursor (0,1);
lcd.print("                ");


if (ore >=10)
{
  x = 0;
  afisare(int(ore/10));
  
  x = 4;
  afisare(ore % 10);
}
else
{
  x = 4;
  afisare(ore);
}

x = 8;
custompp();


if (minuti >=10)
{
  x = 10;
  afisare(int(minuti/10));
  
  x = 14;
  afisare(minuti % 10);
}
else
{
  x = 10;
  afisare(0);
  x = 14;
  afisare(minuti);
}

lcd.setCursor(18, 0);
//    if (now.second() <10) 
    if (secunzi <10) 
      {
        lcd.print("0");
      }
//    lcd.print(now.second());
    lcd.print(secunzi);
//    lcd.print("     ");


lcd.setCursor(0, 3);
if (heat == 1) 
{
   lcd.print("HEATER/");
}
if (heat == 0) 
{
   lcd.print("COOLER/");
}

lcd.setCursor(7, 3);
if (dry == 1) 
{
   lcd.print("DRYER ");
}
if (dry == 0) 
{
   lcd.print("WETTER");
}


   
      if (te > tes) 
 {
if (heat == 1) digitalWrite(temppeste, LOW); 
if (heat == 0) digitalWrite(temppeste, HIGH); 
 } 
if (tes - dete > te)
  {
if (heat == 0) digitalWrite(temppeste, LOW); 
if (heat == 1) digitalWrite(temppeste, HIGH); 
 } 

 if (has > hass) 
 {
if (dry == 1) digitalWrite(umidpeste, HIGH); 
if (dry == 0) digitalWrite(umidpeste, LOW); 
 } 
 if (has < hass - dehas) 
 {
if (dry == 0) digitalWrite(umidpeste, HIGH); 
if (dry == 1) digitalWrite(umidpeste, LOW); 
 } 
    
     pushlength = pushlengthset;
    pushlength = getpushlength ();
    delay (10);
    if (pushlength <pushlengthset) {
      ShortPush ();   
    }
   
      
       //This runs the setclock routine if the knob is pushed for a long time
       if (pushlength >pushlengthset) {
         lcd.clear();
         DateTime now = RTC.now();
         setyeartemp=now.year(),DEC;
         setmonthtemp=now.month(),DEC;
         setdaytemp=now.day(),DEC;
         sethourstemp=now.hour(),DEC;
         setminstemp=now.minute(),DEC;
         setclock();
         pushlength = pushlengthset;
       };
}

//sets the clock
void setclock (){
   setyear ();
   lcd.clear ();
   setmonth ();
   lcd.clear ();
   setday ();
   lcd.clear ();
   sethours ();
   lcd.clear ();
   setmins ();
   lcd.clear();
   
   RTC.adjust(DateTime(setyeartemp,setmonthtemp,setdaytemp,sethourstemp,setminstemp,setsecs));
//   CalcSun ();
   delay (1000);
   
}

// subroutine to return the length of the button push.
int getpushlength () {
  buttonstate = digitalRead(A0);  
       if(buttonstate == LOW && buttonflag==false) {     
              pushstart = millis();
              buttonflag = true;
          };
          
       if (buttonstate == HIGH && buttonflag==true) {
         pushstop = millis ();
         pushlength = pushstop - pushstart;
         buttonflag = false;
       };
       return pushlength;
}
// The following subroutines set the individual clock parameters
int setyear () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Year");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setyeartemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setyeartemp=setyeartemp + knobval;
    if (setyeartemp < 2015) { //Year can't be older than currently, it's not a time machine.
      setyeartemp = 2015;
    }
    lcd.print (setyeartemp);
    lcd.print("  "); 
    setyear();
}
  
int setmonth () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Month");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setmonthtemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setmonthtemp=setmonthtemp + knobval;
    if (setmonthtemp < 1) {// month must be between 1 and 12
      setmonthtemp = 1;
    }
    if (setmonthtemp > 12) {
      setmonthtemp=12;
    }
    lcd.print (setmonthtemp);
    lcd.print("  "); 
    setmonth();
}

int setday () {
  if (setmonthtemp == 4 || setmonthtemp == 5 || setmonthtemp == 9 || setmonthtemp == 11) { //30 days hath September, April June and November
    maxday = 30;
  }
  else {
  maxday = 31; //... all the others have 31
  }
  if (setmonthtemp ==2 && setyeartemp % 4 ==0) { //... Except February alone, and that has 28 days clear, and 29 in a leap year.
    maxday = 29;
  }
  if (setmonthtemp ==2 && setyeartemp % 4 !=0) {
    maxday = 28;
  }
//lcd.clear();  
   lcd.setCursor (0,0);
    lcd.print ("Set Day");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setdaytemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setdaytemp=setdaytemp+ knobval;
    if (setdaytemp < 1) {
      setdaytemp = 1;
    }
    if (setdaytemp > maxday) {
      setdaytemp = maxday;
    }
    lcd.print (setdaytemp);
    lcd.print("  "); 
    setday();
}

int sethours () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Hours");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return sethourstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    sethourstemp=sethourstemp + knobval;
    if (sethourstemp < 1) {
      sethourstemp = 1;
    }
    if (sethourstemp > 23) {
      sethourstemp=23;
    }
    lcd.print (sethourstemp);
    lcd.print("  "); 
    sethours();
}

int setmins () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Mins");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setminstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setminstemp=setminstemp + knobval;
    if (setminstemp < 0) {
      setminstemp = 0;
    }
    if (setminstemp > 59) {
      setminstemp=59;
    }
    lcd.print (setminstemp);
    lcd.print("  "); 
    setmins();
}



void ShortPush () {
  //This displays the calculated sunrise and sunset times when the knob is pushed for a short time.
lcd.clear();
for (long Counter = 0; Counter < 2000 ; Counter ++) { //returns to the main loop if it's been run 604 times 
                                                     //(don't ask me why I've set 604,it seemed like a good number)
//  lcd.setCursor (0,0);
//  lcd.print ("Sunrise ");

   lcd.setCursor(0, 0);
//   lcd.print("+");
   lcd.print(tes,1);
   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("C ");
   lcd.setCursor(1, 1);
   lcd.print(dete,1);
   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("C ");

   lcd.setCursor(7, 0);
   lcd.print(hass);
   lcd.print("%RH ");
   lcd.setCursor(8, 1);
   lcd.print(dehas);
   lcd.print("%RH ");

   lcd.setCursor(14, 0);
if (heat == 1) 
{
   lcd.print("HEATER");
}
if (heat == 0) 
{
   lcd.print("COOLER");
}

   lcd.setCursor(14, 1);
if (dry == 1) 
{
   lcd.print("DRYER");
}
if (dry == 0) 
{
   lcd.print("WETTER");
}



    
  //If the knob is pushed again, enter the mode set menu
  pushlength = pushlengthset;
  pushlength = getpushlength ();
  if (pushlength != pushlengthset) {
    lcd.clear ();
//    heat = setmode ();
   tehas();
  }
  
}
lcd.clear();
}


void tehas (){
sette ();
// write value in eeprom
int tes2 = tes*10;  
byte tset1 = tes2 / 256;
byte tset2 = tes2 - tset1 * 256;
EEPROM.write(201, tset1);  // partea intreaga
EEPROM.write(202, tset2);   // rest
 lcd.clear ();
 
setdete ();
 EEPROM.write(203,dete*10);      
   lcd.clear ();

sethas ();
EEPROM.write(204,hass); 
   lcd.clear ();

setdehas ();
EEPROM.write(205,dehas); 
   lcd.clear ();
   
setheat ();
EEPROM.write(206,heat);      
   lcd.clear();

setdry();
EEPROM.write(207,dry);     
   lcd.clear();
}


// The following subroutines set thermostat and hygrostat parameters
// this subroutine set temperature point for thermostat
int sette () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set temperature");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return tes;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    tes = tes + knobval/10;
//    delay(10);
    if (tes < 10) { 
      tes = 10;
    }
    lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(tes,1);
   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("C  ");

   lcd.setCursor(8, 1);
   lcd.print("dt=");
   lcd.print(dete,1);
   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("C  ");

    sette();
}


// this subroutine set hysteresis temperature for thermostat
int setdete () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set hyst.temp.:");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return dete;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    dete = dete + knobval/10;
//    delay(10);
    if (dete < 0.1) { //Year can't be older than currently, it's not a time machine.
      dete = 0.1;
    }
    lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(tes,1);
   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("C  ");

   lcd.setCursor(8, 1);
   lcd.print("dt=");
   lcd.print(dete,1);
   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("C  ");

    setdete();
}


// this subroutine set humidity point for hygrostat
int sethas () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set humidity:");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return hass;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    hass = hass + knobval;
//    delay(10);
   if (has < 10) { 
      has = 10;
    }
   if (has > 90) { 
      has = 90;
    }
    
   lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(hass,1);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH  ");

   lcd.setCursor(7, 1);
   lcd.print("dh=");
   lcd.print(dehas);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH");

    sethas();
}


// this subroutine set hysteresis humidity for hygrostat
int setdehas () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set hyst.humidit");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return dehas;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    dehas = dehas + knobval;
//    delay(10);
    if (dehas < 1) { //Year can't be older than currently, it's not a time machine.
      dehas = 1;
    }
    lcd.setCursor(0, 1);
//   lcd.print("+");
   lcd.print(hass,1);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH  ");

   lcd.setCursor(7, 1);
   lcd.print("dh=");
   lcd.print(dehas);
//   lcd.write(0b11011111);
//   lcd.write(byte(0));
   lcd.print("%RH");

    setdehas();
}

// this subroutine set type of thermostat
int setheat () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("thermostat type:");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return heat;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    heat = heat + knobval;
//    delay(10);
    if (heat < 0) { 
      heat = 0;
    }
    if (heat > 1) { 
      heat = 1;
    }
       
    lcd.setCursor(0, 1);
if (heat == 1) 
{
   lcd.print("HEATER ");
}
if (heat == 0) 
{
   lcd.print("COOLER ");
}
    setheat();
}

// this subroutine set type of hygrostat
int setdry () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("hygrostat type:");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return dry;
    }
  
    knob.write(0);
    delay (pauza);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    dry = dry + knobval;
//    delay(10);
    if (dry < 0) { 
      dry = 0;
    }
    if (dry > 1) { 
      dry = 1;
    }
       
    lcd.setCursor(0, 1);
if (dry == 1)     
{
   lcd.print("DRYER ");
}
if (dry == 0) 
{
   lcd.print("WETTER ");
}
    setdry();
}

void custom0O()
{ // uses segments to build the number 0
  lcd.setCursor(x, 0); 
  lcd.write(8);  
  lcd.write(1); 
  lcd.write(2);
  lcd.setCursor(x, 1); 
  lcd.write(3);  
  lcd.write(4);  
  lcd.write(5);
}

void custom1()
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(2);
  lcd.write(254);
  lcd.setCursor(x+1,1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(254);
}

void custom2()
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(8);
  lcd.write(7);
  lcd.write(4);
}

void custom3()
{
  lcd.setCursor(x,0);
  lcd.write(6);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(7);
  lcd.write(7);
  lcd.write(5); 
}

void custom4()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(4);
  lcd.write(255);
  lcd.setCursor(x, 1);
  lcd.write(1);
  lcd.write(1);
  lcd.write(255);
}

void custom5()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.write(6);
  lcd.write(1);
  lcd.setCursor(x, 1);
  lcd.write(7);
  lcd.write(7);
  lcd.write(5);
}

void custom6()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x, 1);
  lcd.write(3);
  lcd.write(7);
  lcd.write(5);
}

void custom7()
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(1);
  lcd.write(2);
  lcd.setCursor(x+1, 1);
  lcd.write(8);
  lcd.write(254);
  lcd.write(254);
}

void custom8()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(3);
  lcd.write(7);
  lcd.write(5);
}

void custom9()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(4);
  lcd.write(7);
  lcd.write(5);
}

void customA()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(255);
}

void customB()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.write(6);
  lcd.write(5);
  lcd.setCursor(x, 1);
  lcd.write(255);
  lcd.write(7);
  lcd.write(2);
}

void customC()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(1);
  lcd.write(1);
  lcd.setCursor(x,1);
  lcd.write(3);
  lcd.write(4);
  lcd.write(4);
}

void customD()
{
  lcd.setCursor(x, 0); 
  lcd.write(255);  
  lcd.write(1); 
  lcd.write(2);
  lcd.setCursor(x, 1); 
  lcd.write(255);  
  lcd.write(4);  
  lcd.write(5);
}

void customE()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x, 1);
  lcd.write(255);
  lcd.write(7);
  lcd.write(7); 
}

void customF()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x, 1);
  lcd.write(255);
}

void customG()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(1);
  lcd.write(1);
  lcd.setCursor(x,1);
  lcd.write(3);
  lcd.write(4);
  lcd.write(2);
}

void customH()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.write(4);
  lcd.write(255);
  lcd.setCursor(x, 1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(255); 
}

void customI()
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(255);
  lcd.write(1);
  lcd.setCursor(x,1);
  lcd.write(4);
  lcd.write(255);
  lcd.write(4);
}

void customJ()
{
  lcd.setCursor(x+2,0);
  lcd.write(255);
  lcd.setCursor(x,1);
  lcd.write(4);
  lcd.write(4);
  lcd.write(5);
}

void customK()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.write(4);
  lcd.write(5);
  lcd.setCursor(x,1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(2); 
}

void customL()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.setCursor(x,1);
  lcd.write(255);
  lcd.write(4);
  lcd.write(4);
}

void customM()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(3);
  lcd.write(5);
  lcd.write(2);
  lcd.setCursor(x,1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(254);
  lcd.write(255);
}

void customN()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(2);
  lcd.write(254);
  lcd.write(255);
  lcd.setCursor(x,1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(3);
  lcd.write(5);
}

void customP()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(255);
}

void customQ()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(1);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(3);
  lcd.write(4);
  lcd.write(255);
  lcd.write(4);
}

void customR()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x,1);
  lcd.write(255);
  lcd.write(254);
  lcd.write(2); 
}

void customS()
{
  lcd.setCursor(x,0);
  lcd.write(8);
  lcd.write(6);
  lcd.write(6);
  lcd.setCursor(x, 1);
  lcd.write(7);
  lcd.write(7);
  lcd.write(5);
}

void customT()
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(255);
  lcd.write(1);
  lcd.setCursor(x,1);
  lcd.write(254);
  lcd.write(255);
}

void customU()
{
  lcd.setCursor(x, 0); 
  lcd.write(255);  
  lcd.write(254); 
  lcd.write(255);
  lcd.setCursor(x, 1); 
  lcd.write(3);  
  lcd.write(4);  
  lcd.write(5);
}

void customV()
{
  lcd.setCursor(x, 0); 
  lcd.write(3);  
  lcd.write(254);
  lcd.write(254); 
  lcd.write(5);
  lcd.setCursor(x+1, 1); 
  lcd.write(2);  
  lcd.write(8);
}

void customW()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.write(254);
  lcd.write(254);
  lcd.write(255);
  lcd.setCursor(x,1);
  lcd.write(3);
  lcd.write(8);
  lcd.write(2);
  lcd.write(5);
}

void customX()
{
  lcd.setCursor(x,0);
  lcd.write(3);
  lcd.write(4);
  lcd.write(5);
  lcd.setCursor(x,1);
  lcd.write(8);
  lcd.write(254);
  lcd.write(2); 
}

void customY()
{
  lcd.setCursor(x,0);
  lcd.write(3);
  lcd.write(4);
  lcd.write(5);
  lcd.setCursor(x+1,1);
  lcd.write(255);
}

void customZ()
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(6);
  lcd.write(5);
  lcd.setCursor(x, 1);
  lcd.write(8);
  lcd.write(7);
  lcd.write(4);
}

void customqm()
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(6);
  lcd.write(2);
  lcd.setCursor(x, 1);
  lcd.write(254);
  lcd.write(7);
}

void customsm()
{
  lcd.setCursor(x,0);
  lcd.write(255);
  lcd.setCursor(x, 1);
  lcd.write(7);
}

void customplus()  // made by niq_ro
{
  lcd.setCursor(x,0);
  lcd.write(4);
  lcd.write(255);
  lcd.write(4);
  lcd.setCursor(x, 1);
  lcd.write(1);
  lcd.write(255);
  lcd.write(1);
}

void customminus() // made by niq_ro
{
  lcd.setCursor(x,0);
  lcd.write(4);
  lcd.write(4);
  lcd.write(4);
  lcd.setCursor(x, 1);
  lcd.write(1);
  lcd.write(1);
  lcd.write(1);
}

void customgrad()
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.write(8);
  lcd.write(1);
  lcd.write(1);
  lcd.setCursor(x, 1);
  lcd.write(254);
  lcd.write(3);
  lcd.write(4);
  lcd.write(4);
}

void custompp()
{
  lcd.setCursor(x,0);
  lcd.write(1);
  lcd.setCursor(x,1);
//  lcd.write(1);
  lcd.write(4);
}


// subrutina de afisare a numerelor
void afisare(int numar)
{
  switch (numar)
  {
    case 0:
    custom0O();
    break;

    case 1:
    custom1();
    break;

    case 2:
    custom2();
    break;

    case 3:
    custom3();
    break;

    case 4:
    custom4();
    break;

    case 5:
    custom5();
    break;

    case 6:
    custom6();
    break;

    case 7:
    custom7();
    break;

    case 8:
    custom8();
    break;

    case 9:
    custom9();
    break;
  }
}

void temperaturi(int t)
{
if (t>0)
{
if (t>=10)
{
  x = 0;
//  lcd.clear();
  customplus();
  
  x = x + 4;
  afisare(int(t/10));
  
  x = x + 4;
  afisare(t % 10);
  
  x = x + 4;
  customgrad();
 // delay(500);
}
else
if (t<10)
{

x = 4;
//  lcd.clear();
  customplus();
  
   x = x + 4;
  afisare(t);
  
  x = x + 4;
  customgrad();
 // delay(500);
}
}

if (t==0)
{
  x = 8;
//  lcd.clear();
  afisare(0);
  x = x + 4;
  customgrad();
 // delay(500);
}

if (t<0)
{
t=-t;

if (t<10)
{
  x = 4;
//  lcd.clear();
  customminus();
  
   x = x + 4;
  afisare(t);
  
  x = x + 4;
  customgrad();
  //delay(500);
}

else
{
  x = 0;
//  lcd.clear();
  customminus();
  
  x = x + 4;
  afisare(int(t/10));
  
  x = x + 4;
  afisare(t % 10);
  
  x = x + 4;
  customgrad();
  //delay(500);
 }
}
}


void ceas(int c)
{
if (c>=10)
{
  x = 0;
  afisare(int(c/10));
  
  x = x + 4;
  afisare(c % 10);
}
else
{
x = 4;
  afisare(c);
}
}
