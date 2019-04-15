#include <Servo.h> 

/* Diyode CodeShield Constants */
#define SERVO1 4 //Pushes A
#define SERVO2 3 //Pushes Start
#define LED 8 
#define PIEZO 6
#define PHOTOCELL 1
#define BUTTON 9

#define SCREEN_DARK 0
#define SCREEN_BRIGHT 1
#define CHECK_SCRREN_DELAY 150
#define WALKING_BUTTONPRESS_DURATION 1500
#define TIME_AFTER_BATTLE 7000
#define DARK 0
#define BRIGHT 1
#define ERR_WUE_PREMATURE_RETURN 1

#define STARTING 1
#define WALKING 2
#define WATCHING 3
#define FOUND 4
Servo servo1;
Servo servo2;

// Unit A Prefs
int servo1Flat = 150;
int servo1Deflect = 25;
int servo2Flat = 140;
int servo2Deflect = 25;
String name = "A";

int photocellPin = 1;     // the cell and 10K pulldown are connected to a1
int photocellReading;     // the analog reading from the sensor divider

// Delayed blackout

int SHINY_GLITCH_LOW = 13500;
int SHINY_GLITCH_HIGH = 21000;
int SHINY_LOW1 = 11800;
int SHINY_LOW2 = 13300;
int SHINY_ABIL1 = 15000;
int SHINY_ABIL2 = 22999;
int SHINY_HIGH = 23000;

/*
// Unit B Prefs
int servo1Flat = 88;
int servo1Deflect = 15;
int servo2Flat = 90;
int servo2Deflect = 10;
String name = "B";
*/

// Immediate blackout
/*
int SHINY_GLITCH_LOW = 5800;
int SHINY_GLITCH_HIGH = 10000;
int SHINY_LOW1 = 11200;
int SHINY_LOW2 = 20300;
int SHINY_ABIL1 = 20300;
int SHINY_ABIL2 = 20301;
int SHINY_HIGH = 24500;
*/

unsigned int mode = 1;

unsigned int encounter_count = 0;
unsigned char dark_threshold = 250;
unsigned int bike_count = 0;
long last_blackout = 0;
//unsigned long start_time;

void setup() {
  Serial.begin (115200);
  Serial.print("Start Unit ");
  Serial.println(name);

  servo1.attach(SERVO1);
  servo1.write(servo1Flat);

  servo2.attach(SERVO2);
  servo2.write(servo2Flat);
  
}

void loop() {
  switch (mode) {
    case STARTING:
      watchButton();
      break;
    case WALKING:
      doWalking();
      break;
    case WATCHING:
      break;
    case FOUND:
      break;
  }
}

void watchButton() {
  if (digitalRead(BUTTON) == HIGH) {
    digitalWrite(LED, LOW);
    mode = WALKING;
  }  
}

void doWalking()
{
//    start_time = millis();
    while(1) //While the screen is bright
    {
        long blackout_duration = walk_until_encounter();
        encounter_count++;
        if(is_shiny(blackout_duration)) //If found pokemon is shiny
        {
         //   unsigned long shiny_time = millis() - start_time;
            Serial.print("Shiny: ");
            Serial.println(encounter_count);
         //   Serial.print("Elapsed time at Shiny: ");
         //   Serial.println(shiny_time);
            encounter_count = 0;
            beep();
            digitalWrite(LED, HIGH);
            mode = FOUND;
            return;
        }
        else
        {
            Serial.print("Encounter count: ");
            Serial.println(encounter_count);            
            tapScreen(); //Leaves the battle
            delay(TIME_AFTER_BATTLE); //Waits the predetermined time before resuming search
        }
    }
}

int is_shiny(long blackout_duration)
{
    last_blackout = blackout_duration;
    long shiny_threshold = SHINY_HIGH;
    Serial.print("Blackout Duration: ");
    Serial.println(blackout_duration);

   if(is_between(blackout_duration, SHINY_GLITCH_LOW, SHINY_GLITCH_HIGH) )
    {
        return 1;
    }
    else
        return 0;
}


int is_between(long val, long lower, long upper)
{
    return (val > lower && val < upper);
}

//Leaves a none shiny encounter
void tapScreen() {
  delay(100);
  servo2.write(servo2Flat + servo2Deflect); //Presses Start to soft reset
  delay(500);
  servo2.write(servo2Flat);
  delay(11000); //Waits for game to resume
  servo1.write(servo1Flat + servo1Deflect); //Presses A to skip opening animation
  delay(1000);
  servo1.write(servo1Flat); 
  delay(1500);
  servo1.write(servo1Flat + servo1Deflect); //Presses A open save files
  delay(1000);
  servo1.write(servo1Flat); 
  delay(3500);
  servo1.write(servo1Flat + servo1Deflect); //Presses A to open save file
  delay(1000);
  servo1.write(servo1Flat); 
  delay(3500);
  servo1.write(servo1Flat + servo1Deflect); //Presses A to open bag
  delay(1000);
  servo1.write(servo1Flat); 
  
}

void beep() {
 analogWrite( PIEZO, 128);
 delay(400);
 digitalWrite(PIEZO, LOW);
}

//Presses A until the screen goes dark
 int walk_until_encounter()
{
    unsigned long dark_start = 0;
    unsigned char screen_state = SCREEN_BRIGHT;
     
    while(1)
    {
        delay(CHECK_SCRREN_DELAY);
        if(screen_state == SCREEN_BRIGHT)
        {
            servo1.write(servo1Flat);
            if(millis() % WALKING_BUTTONPRESS_DURATION * 2 < WALKING_BUTTONPRESS_DURATION)
            {
              servo1.write(servo1Flat + servo1Deflect);
            }
            else
            {
              
              servo1.write(servo1Flat);
              
            }
            
      }
        int curr = analogRead(PHOTOCELL); 
         
        curr < dark_threshold ? curr = DARK : curr = BRIGHT;
         
        if(screen_state == SCREEN_BRIGHT && curr == DARK)
        {
            screen_state = SCREEN_DARK;
            servo1.write(servo1Flat);
            servo2.write(servo2Flat);
            dark_start = millis();
        }
       
        else if(screen_state == SCREEN_DARK && curr == BRIGHT)
        {
            unsigned long ret = millis() - dark_start;
             return ret;
        }
    }
}
