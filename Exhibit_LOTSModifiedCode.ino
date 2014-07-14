
/*
 
 LOTS- Laser Operated Targeting System Main (TOP LEVEL) File

 Code By: Christian Butterfield, Jim Hunter and Jeffrey Childers
 
 Orlando Science Center in cooperation with Northrop Grumman
 
*/

//#include <AF_Wave.h>
//#include <avr/pgmspace.h>
#include "WaveHC.h" //#include "wave.h"
#include "WaveUtil.h" //#include "util.h"

#define FOG_TIMER_SECS           60

#define NO_HITS_TIMEOUT_SECS  90 
#define PR_DELTA_THRESHOLD    30
#define PLAYER_ONE             1
#define PLAYER_TWO             2

#define FOG_PWR               41
#define RED_LED               42


// Analog input pins
#define P1_PR1_EASY_PIN        8
#define P1_PR2_MED_PIN         9
#define P1_PR3_HARD_PIN       10
#define P2_PR4_EASY_PIN       11
#define P2_PR5_MED_PIN        12
#define P2_PR6_HARD_PIN       13

// Digital Pins
#define GREEN_LASER_PWR_PIN         24
#define RED_LASER_PWR_PIN           25
#define BLUE_STATUS_LED_PIN         43
#define P1_BUNK1_EASY_LED_PIN       44
#define P1_BUNK2_MED_LED_PIN        45
#define P1_BUNK3_HARD_LED_PIN       46
#define P2_BUNK1_EASY_LED_PIN       47
#define P2_BUNK2_MED_LED_PIN        48
#define P2_BUNK3_HARD_LED_PIN       49

#define MIN_TIME_ON_TARGET_MSECS  1000
typedef enum { GAME_INIT, GAME_READY, GAME_ON, P1_WINS, P2_WINS }GameStateType;
GameStateType GameState = GAME_INIT;
bool BIT_pass = true;
int i;
long int lastFogTime = 0; // *** JTC used to count time fog machine dispersed fog ***
long int timeSinceLastFog = 0;  //current time minus last fog time, converted to seconds
long int lastHitTime = 0; // used to timeout game if no hits in NO_HITS_TIMEOPUT_SECS amount of time
long int timeSinceLastHit = 0; // current time minus last hit time, converted to seconds
long int p1_timeOnTarget = 0;
long int p1_startedTimeOnTarget = 0;
long int p2_timeOnTarget = 0;
long int p2_startedTimeOnTarget = 0;
int target_sequence[3] = {0, 0, 0}; // initialized on game reset, not being used in code yet
//int p2_target_sequence[3] = {0, 0, 0};
bool p1_targets_hit[ 3 ] = { false, false, false };
bool p2_targets_hit[ 3 ] = { false, false, false };
int p1_sequence_ndx = -1; // MUST be set to -1 before every game start
int p1_tgt_ndx = 0;
int p2_sequence_ndx = -1; // MUST be set to -1 before every game start
int p2_tgt_ndx = 0;

// pin map arrays
int p1_tgtPRThresholds[ 3 ] = {  0,  0,  0 };
int p1_tgtSensorPinMap[ 3 ] = { P1_PR1_EASY_PIN, P1_PR2_MED_PIN, P1_PR3_HARD_PIN };
int p1_tgtLEDPinMap[ 3 ]    = { P1_BUNK1_EASY_LED_PIN, P1_BUNK2_MED_LED_PIN, P1_BUNK3_HARD_LED_PIN };
int p2_tgtPRThresholds[ 3 ] = {  0,  0,  0 };
int p2_tgtSensorPinMap[ 3 ] = { P2_PR4_EASY_PIN, P2_PR5_MED_PIN, P2_PR6_HARD_PIN };
int p2_tgtLEDPinMap[ 3 ]    = { P2_BUNK1_EASY_LED_PIN, P2_BUNK2_MED_LED_PIN, P2_BUNK3_HARD_LED_PIN };


/////Button section//////

const int buttonPin = 22;     // the number of the pushbutton pin
const int btnledPin =  23;      // the number of the test LED pin
int buttonState = 0;             // the current reading from the input pin
//int lastButtonState = LOW;   // the previous reading from the input pin

/////////////////////////

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader f;      // This holds the information for the file we're play
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

uint8_t dirLevel; // indent level for file/dir names    (for prettyprinting)
dir_t dirBuf;     // buffer for directory reads

int photocellPin = p1_tgtSensorPinMap[ p1_tgt_ndx ]; // the cell and 10K pulldown are connected to a7 ****************
int ledPin = p1_tgtLEDPinMap[ p1_tgt_ndx ];
int p2_photocellPin = p2_tgtSensorPinMap[ p2_tgt_ndx ]; // the cell and 10K pulldown are connected to a7 ****************
int p2_ledPin = p2_tgtLEDPinMap[ p2_tgt_ndx ];
int photocellReading;     // the analog reading from the analog resistor divider **********
int p2_photocellReading;     // the analog reading from the analog resistor divider **********
int PRbasevalue;          // Photoresistor base value (which is defined in setPRparameters in setup) set on startup
int runxtimes = 1;        // Play sound only once when bunker HIT
int PRdifference;
int p1_targets_hit_cnt = 0;
int p2_targets_hit_cnt = 0;

/*
 * Define macro to put error messages in flash memory
 */
 
#define error(msg) error_P(PSTR(msg))

// Function definitions (we define them here, but the code is below)

int PhotoResistor_Int; //Set variable for photoresistor
int fogState = LOW;             // fogState used to set the fog button
unsigned long fogPreviousMillis = 0;
unsigned long currentMillis = millis();
void play(FatReader &dir);

int debugging = 1;  // If true, sends Serial messages useful for debugging

// ~~~~SETUP~~~~ //

void setup() 
{

  pinMode( GREEN_LASER_PWR_PIN, OUTPUT);
  pinMode(   RED_LASER_PWR_PIN, OUTPUT);  
  pinMode( BLUE_STATUS_LED_PIN, OUTPUT);    // (Clear BLUE) - startup led
  
  pinMode(FOG_PWR, OUTPUT);
  
  pinMode (RED_LED,OUTPUT);
  
  pinMode( P1_BUNK1_EASY_LED_PIN, OUTPUT);    // LEDs for P1 bunkers
  pinMode( P1_BUNK2_MED_LED_PIN, OUTPUT);    
  pinMode( P1_BUNK3_HARD_LED_PIN, OUTPUT);
  pinMode( P2_BUNK1_EASY_LED_PIN, OUTPUT);    // LEDs for P2 bunkers
  pinMode( P2_BUNK2_MED_LED_PIN, OUTPUT);    
  pinMode( P2_BUNK3_HARD_LED_PIN, OUTPUT);
  
  //Temp in use
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);
  //
  
  pinMode(buttonPin, INPUT);
  pinMode(btnledPin, OUTPUT);
  
  Serial.begin(115200);           // set up Serial library at 9600 bps for debugging
  
  //// Blink to show startup is running
  digitalWrite( BLUE_STATUS_LED_PIN, HIGH);   // blink the startup led until startup is complete
  digitalWrite( P1_BUNK1_EASY_LED_PIN, HIGH);    // LEDs for P1 bunkers
  digitalWrite( P1_BUNK2_MED_LED_PIN, HIGH);    
  digitalWrite( P1_BUNK3_HARD_LED_PIN, HIGH);
  digitalWrite( P2_BUNK1_EASY_LED_PIN, HIGH);    // LEDs for P2 bunkers
  digitalWrite( P2_BUNK2_MED_LED_PIN, HIGH);    
  digitalWrite( P2_BUNK3_HARD_LED_PIN, HIGH); 
  delay(500);               //wait
  digitalWrite( BLUE_STATUS_LED_PIN, LOW);   // blink the startup led until startup is complete
  digitalWrite( P1_BUNK1_EASY_LED_PIN, LOW);    // LEDs for P1 bunkers  
  delay(500);               //wait
  digitalWrite( BLUE_STATUS_LED_PIN, HIGH );   // blink the startup led until startup is complete
  digitalWrite( P1_BUNK1_EASY_LED_PIN, HIGH );    // LEDs for P1 bunkers
  digitalWrite( P1_BUNK2_MED_LED_PIN,  LOW );    
   
  delay(500);               //wait
  digitalWrite(BLUE_STATUS_LED_PIN, LOW);   // blink the startup led until startup is complete
  digitalWrite( P1_BUNK2_MED_LED_PIN, HIGH);    
  digitalWrite( P1_BUNK3_HARD_LED_PIN,  LOW);  
  delay(500);               //wait
  digitalWrite(BLUE_STATUS_LED_PIN, HIGH);   // blink the startup led until startup is complete    
  digitalWrite( P1_BUNK3_HARD_LED_PIN, HIGH);
  digitalWrite( P2_BUNK1_EASY_LED_PIN,  LOW);  
  delay(500);               //wait
  digitalWrite(BLUE_STATUS_LED_PIN, LOW);   // blink the startup led until startup is complete
  digitalWrite( P2_BUNK1_EASY_LED_PIN, HIGH );    // LEDs for P1 bunkers
  digitalWrite( P2_BUNK2_MED_LED_PIN,  LOW );    
  delay(500);              //wait
  digitalWrite( P1_BUNK1_EASY_LED_PIN, LOW);    // LEDs for P1 bunkers
  digitalWrite( P1_BUNK2_MED_LED_PIN, LOW);    
  digitalWrite( P1_BUNK3_HARD_LED_PIN, LOW);
  digitalWrite( P2_BUNK1_EASY_LED_PIN, LOW);    // LEDs for P2 bunkers
  digitalWrite( P2_BUNK2_MED_LED_PIN, LOW);    
  digitalWrite( P2_BUNK3_HARD_LED_PIN, LOW);
  ////
   
  //LASER POWER ON/PULSE TEST
  
  digitalWrite(GREEN_LASER_PWR_PIN, HIGH);  
  digitalWrite(RED_LASER_PWR_PIN, HIGH);
  delay(2000);      
  digitalWrite(GREEN_LASER_PWR_PIN, LOW);  
  digitalWrite(RED_LASER_PWR_PIN, LOW);
  delay(2000);    
  
  for (i=0; i<2; i++)
  {
    digitalWrite(GREEN_LASER_PWR_PIN, HIGH);  
    digitalWrite(RED_LASER_PWR_PIN, HIGH);
    delay(100);      
    digitalWrite(GREEN_LASER_PWR_PIN, LOW);  
    digitalWrite(RED_LASER_PWR_PIN, LOW);
    delay(100);    
  }
  //


  
  
  // Ensure Depower lasers on start-up (redundant but keep)
  digitalWrite( GREEN_LASER_PWR_PIN, LOW);    
  digitalWrite( RED_LASER_PWR_PIN, LOW); 
  
// setPRparameters(p1_tgt_ndx); //Set parameter for PhotoResistor


//////SOUND SETUP////////////

  putstring_nl("\nWavetest!");  // say we woke up!
  

  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) 
  {         //play with 8 MHz spi (default faster!)
    BIT_pass = false;  
    error("Card init. failed!");  // Something went wrong, lets print out why
  }
  
  
  if(  BIT_pass==true )
  {
    putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
    Serial.println(FreeRam());    
    
    // enable optimize read - some cards may timeout. Disable if you're having problems
    card.partialBlockRead(true);
    
    // Now we will look for a FAT partition!
    uint8_t part;
    
    for (part = 0; part < 5; part++) 
    {   // we have up to 5 slots to look in
      if (vol.init(card, part)) 
        break;                           // we found one, lets bail
    }
    
    if (part == 5) 
    {                     // if we ended up not finding one  :(
      error("No valid FAT partition!");  // Something went wrong, lets print out why
    }
    
    // Lets tell the user about what we found
    putstring("Using partition ");
    Serial.print(part, DEC);
    putstring(", type is FAT");
    Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
    
    // Try to open the root directory
    if (!root.openRoot(vol)) 
    {
      error("Can't open root dir!");      // Something went wrong,
    }
    
    // Whew! We got past the tough parts.
    putstring_nl("Files found (* = fragmented):");
  
    // Print out all of the files in all the directories.
    root.ls(LS_R | LS_FLAG_FRAGMENTED);
  }
  
   lastFogTime = millis(); //[cjb]
}


// ~~~~ LOOP ~~~~//

void loop() 
{   
  
  
  if (debugging){
    Serial.print("FOG_PWR: ");
    Serial.println(fogState);
  
    Serial.print("currentMillis: ");
    Serial.println(currentMillis);
  
    Serial.print("fogPreviousMillis: ");
    Serial.println(fogPreviousMillis);
  
    Serial.print("currentMillis - fogPreviousMillis: ");
    Serial.println(currentMillis - fogPreviousMillis);
    
    delay(50);
  }
  
  currentMillis = millis();
  
  digitalWrite(FOG_PWR, LOW); //redundant but keep 
  
  timeSinceLastFog = (int)( (float)(millis() - lastFogTime) / 1000.0 );  // millis() - lastFogTime gets us the number of milliseconds since the machine fogged last. Dividing that by 1000.0 converts it to the number of seconds since the machine fogged last. 1000.0 should not be changed, as there will be 1000 milliseconds in a second at least until the end of time, if not longer.
   
   if(currentMillis - fogPreviousMillis > 30 * 1000 * 60) { // every 30 Minutes
    // save the last time you fogged the machine
    fogPreviousMillis = currentMillis;   
  }

  if (currentMillis - fogPreviousMillis < 20000) {fogState = HIGH;} else {fogState = LOW;}  // Every x often runs for 20 seconds
  digitalWrite(FOG_PWR, fogState);

  if ( timeSinceLastFog > FOG_TIMER_SECS )
    {
        delay(50);
        digitalWrite(FOG_PWR, HIGH);
        delay(2000);
        digitalWrite(FOG_PWR, LOW);
        delay(50);
	lastFogTime = millis(); //[cjb]
    }                                // JTC 1/12/13
  
  switch (GameState)
  {
    case GAME_INIT:
    
      digitalWrite(BLUE_STATUS_LED_PIN, HIGH);   // blink the startup led until startup is complete
      delay(500);               //wait
      digitalWrite(BLUE_STATUS_LED_PIN, LOW);   // blink the startup led until startup is complete
      delay(500);               //wait
      digitalWrite(BLUE_STATUS_LED_PIN, HIGH);   // blink the startup led until startup is complete
      delay(500);               //wait
      
      //depower lasers
      digitalWrite( GREEN_LASER_PWR_PIN, LOW);    
      digitalWrite( RED_LASER_PWR_PIN, LOW); 
      
    
      // get random target sequence
      randomSeed (analogRead (10));  // so you don't get the same tgt sequence every time      
      target_sequence[0] = random(3);
      target_sequence[1] = ( target_sequence[0] + 1 + random(2)) % 3;
      target_sequence[2] = 3 - ( target_sequence[0] + target_sequence[1] ); 
      
      //p2_target_sequence[0] = random(3);
      //p2_target_sequence[1] = ( p2_target_sequence[0] + 1 + random(2)) % 3;
      //p2_target_sequence[2] = 3 - ( p2_target_sequence[0] + p2_target_sequence[1] );
      
      p1_sequence_ndx = -1; // MUST be set to -1 before every game start
      p2_sequence_ndx = -1; // MUST be set to -1 before every game start
      
      // turn all LEDs off between game rounds
      digitalWrite( P1_BUNK1_EASY_LED_PIN, LOW);    // LEDs for P2 bunkers
      digitalWrite( P1_BUNK2_MED_LED_PIN,  LOW);    
      digitalWrite( P1_BUNK3_HARD_LED_PIN, LOW);

      digitalWrite( P2_BUNK1_EASY_LED_PIN, LOW);    // LEDs for P2 bunkers
      digitalWrite( P2_BUNK2_MED_LED_PIN,  LOW);    
      digitalWrite( P2_BUNK3_HARD_LED_PIN, LOW);
      
      for( p1_tgt_ndx = 0; p1_tgt_ndx < 3; p1_tgt_ndx++ ) //Ensure index is cleared on startup
      {
        photocellPin = p1_tgtSensorPinMap[ p1_tgt_ndx ];
        ledPin = p1_tgtLEDPinMap[ p1_tgt_ndx ];
        setPRparameters(PLAYER_ONE, p1_tgt_ndx);  // get baseline PR levels

        p1_targets_hit[ p1_tgt_ndx ] = false; // clear all hit flags        
      }

      for( p2_tgt_ndx = 0; p2_tgt_ndx < 3; p2_tgt_ndx++ ) //Ensure index is cleared on startup
      {
        p2_photocellPin = p2_tgtSensorPinMap[ p2_tgt_ndx ];
        p2_ledPin = p2_tgtLEDPinMap[ p2_tgt_ndx ];
        setPRparameters(PLAYER_TWO, p2_tgt_ndx);  // get baseline PR levels

        p2_targets_hit[ p2_tgt_ndx ] = false; // clear all hit flags        
      }
      
      GameState = GAME_READY;
      break;
        
    case GAME_READY:
      
      //chk_p1button();//If button pressed, do ledshow and break to GAME_ON.
      chk_button();
      //Serial.println(buttonState);
      
      if(buttonState == 1)
      {
  
//        delay(50);
//        digitalWrite(FOG_PWR, HIGH);
//        delay(100);
//        digitalWrite(FOG_PWR, LOW);    JTC 1/12/13
        delay(50);
        ledshowSTART();
        delay(50);
        playcomplete("intro.wav");
        delay(2000); //select targets 2 seconds after saying instructions
        
        randomtgt();  
        p2_randomtgt();       

        //// Blink to show in Gamestate transition to GAME_ON /////
        digitalWrite(BLUE_STATUS_LED_PIN, LOW);   // blink the startup led until startup is complete
        delay(500);               //wait
        digitalWrite(BLUE_STATUS_LED_PIN, HIGH);   // blink the startup led until startup is complete
        delay(500);               //wait
        
        // Power Lasers
        digitalWrite(GREEN_LASER_PWR_PIN, HIGH);
        digitalWrite(RED_LASER_PWR_PIN, HIGH);
        
        GameState = GAME_ON;
        lastHitTime = millis();        
        break;
        
      }
      else // buttonState=0
      {
        GameState = GAME_READY;//Do nothing, remain in gamestate GAME_READY
        break;
      }
    
    case GAME_ON:
    
    
      digitalWrite(GREEN_LASER_PWR_PIN, HIGH);
      digitalWrite(RED_LASER_PWR_PIN, HIGH);
        
      timeSinceLastHit = (int)( (float)(millis() - lastHitTime) / 1000.0 );    
      
      if( timeSinceLastHit > NO_HITS_TIMEOUT_SECS )
      {
        // time out and reset to init state
        GameState = GAME_INIT;
        break; 
      }
      
      photocellReading = analogRead(photocellPin);
      p2_photocellReading = analogRead(p2_photocellPin);
      
      if (photocellReading > p1_tgtPRThresholds[ p1_tgt_ndx ] )
      { 
        p1_timeOnTarget = millis() - p1_startedTimeOnTarget;
        if( p1_timeOnTarget >= MIN_TIME_ON_TARGET_MSECS )
        {
          //if (runxtimes > 0)
          {
            Serial.println(" - HIT");
            
            p1_targets_hit[ p1_tgt_ndx ] = true; // indicate hit
            digitalWrite(p1_tgtLEDPinMap[ p1_tgt_ndx ], LOW);
        
            //Bunker Light Show
            //ledshowHIT(); 
         
            //Pulse Green Laser
            for (i=0; i<3; i++)
            {
              digitalWrite(GREEN_LASER_PWR_PIN, HIGH);  
              delay(100);      
              digitalWrite(GREEN_LASER_PWR_PIN, LOW);
              delay(100);   
            }
              
            p1_targets_hit_cnt++;
            
            p1_tgt_ndx = target_sequence[ p1_targets_hit_cnt ];
            
        
            if( -1 == randomtgt() )
            {
              playcomplete("g_wins.wav");  //selects the sound file in SD card that is titled "_____.wave"
              GameState = GAME_INIT; // Game WON, start over
            }
            else
            {
              playcomplete("g_tgtacq.wav");  //selects the sound file in SD card that is titled "_____.wave" 
            }
          }
        }
      }
      else
      {
        p1_startedTimeOnTarget = millis(); // always capture new time on target reference time if not above threshold
      }
      
      if (p2_photocellReading > p2_tgtPRThresholds[ p2_tgt_ndx ] )
      {
       
        
        {
          Serial.println(" - HIT");
          
          p2_targets_hit[ p2_tgt_ndx ] = true; // indicate hit
          digitalWrite(p2_tgtLEDPinMap[ p2_tgt_ndx ], LOW);
      
          //ledshowHIT();  
        
          //Pulse Red Laser
          for (i=0; i<3; i++)
          { 
            digitalWrite(RED_LASER_PWR_PIN, HIGH);
            delay(100);        
            digitalWrite(RED_LASER_PWR_PIN, LOW);
            delay(100);    
          }  
          
          p2_targets_hit_cnt++;
          
          p2_tgt_ndx = target_sequence[ p2_targets_hit_cnt ];
      
          if( -1 == p2_randomtgt() )
          {
            playcomplete("r_wins.wav");  //selects the sound file in SD card that is titled "_____.wave"
            GameState = GAME_INIT; // Game WON, start over
          }
          else
          {
            playcomplete("r_tgtacq.wav");  //selects the sound file in SD card that is titled "_____.wave" 
          }
        }
      }
   }   
}
