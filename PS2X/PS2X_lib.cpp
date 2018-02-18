#include "PS2X_lib.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//#include <avr/io.h>
#if ARDUINO > 22
  #include "Arduino.h"
#else
  //#include "WProgram.h"
  //#include "pins_arduino.h"
#endif

static byte enter_config[]={0x01,0x43,0x00,0x01,0x00};
static byte set_mode[]={0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00};
static byte set_bytes_large[]={0x01,0x4F,0x00,0xFF,0xFF,0x03,0x00,0x00,0x00};
static byte exit_config[]={0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A};
static byte enable_rumble[]={0x01,0x4D,0x00,0x00,0x01};
static byte type_read[]={0x01,0x45,0x00,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A};

#ifdef __RPI___
#include <sys/time.h>
double millis() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    double milliseconds = te.tv_sec*1000 + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
}
#endif

/****************************************************************************************/
boolean PS2X::NewButtonState() {
  return ((last_buttons ^ buttons) > 0);
}

/****************************************************************************************/
boolean PS2X::NewButtonState(unsigned int button) {
  return (((last_buttons ^ buttons) & button) > 0);
}

/****************************************************************************************/
boolean PS2X::ButtonPressed(unsigned int button) {
  return(NewButtonState(button) & Button(button));
}

/****************************************************************************************/
boolean PS2X::ButtonReleased(unsigned int button) {
  return((NewButtonState(button)) & ((~last_buttons & button) > 0));
}

/****************************************************************************************/
boolean PS2X::Button(uint16_t button) {
  return ((~buttons & button) > 0);
}

/****************************************************************************************/
unsigned int PS2X::ButtonDataByte() {
   return (~buttons);
}

/****************************************************************************************/
byte PS2X::Analog(byte button) {
   return PS2data[button];
}

/****************************************************************************************/
unsigned char PS2X::_gamepad_shiftinout (char data) {
   unsigned char tmp = 0;
   for(unsigned char i=0;i<8;i++) {
      if(CHK(data ,i)) CMD_SET();
      else CMD_CLR();
	  
      CLK_CLR();
      delayMicroseconds(CTRL_CLK);

      //if(DAT_CHK()) SET(tmp,i);
      if(DAT_CHK()) 
      {
		bitSet(tmp,i);
	}

      CLK_SET();
#if CTRL_CLK_HIGH
      delayMicroseconds(CTRL_CLK_HIGH);
#endif
   }
   CMD_SET();
   delayMicroseconds(CTRL_BYTE_DELAY);
   return tmp;
}

/****************************************************************************************/
double PS2X::read_gamepad(boolean motor1, byte motor2) {
    double btns = 0;
   double temp = millis() - last_read;

   if (temp > 1500) //waited to long
      reconfig_gamepad();

   if(temp < read_delay)  //waited too short
      delay(read_delay - temp);

#ifndef __RPI__
   if(motor2 != 0x00)
      motor2 = map(motor2,0,255,0x40,0xFF); //noting below 40 will make it spin
#endif

   char dword[9] = {0x01,0x42,0,motor1,motor2,0,0,0,0};
   byte dword2[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

   // Try a few times to get valid data...
   for (byte RetryCnt = 0; RetryCnt < 5; RetryCnt++) 
   {
      CMD_SET();
      CLK_SET();
      ATT_CLR(); // low enable joystick

      delayMicroseconds(CTRL_BYTE_DELAY);
      //Send the command to send button and joystick data;
      for (int i = 0; i<9; i++) {
         PS2data[i] = _gamepad_shiftinout(dword[i]);
      }
/*
      if(PS2data[1] == 0x79) 
      {  //if controller is in full data return mode, get the rest of data
         for (int i = 0; i<12; i++) 
         {
            PS2data[i+9] = _gamepad_shiftinout(dword2[i]);
         }
      }
      */

      ATT_SET(); // HI disable joystick
      // Check to see if we received valid data or not.  
	  // We should be in analog mode for our data to be valid (analog == 0x7_)
      if ((PS2data[1] & 0xf0) == 0x70)
         break;

      // If we got to here, we are not in analog mode, try to recover...
      reconfig_gamepad(); // try to get back into Analog mode.
      delay(read_delay);
   }

   // If we get here and still not in analog mode (=0x7_), try increasing the read_delay...
   if ((PS2data[1] & 0xf0) != 0x70) {
      if (read_delay < 10)
         read_delay++;   // see if this helps out...
   }

#ifdef PS2X_COM_DEBUG
   printf(">> ");
   for(int i=0; i<9; i++)
   {
      printf("%02x ", PS2data[i]);
   }
/*   
    if ((PS2data[1] & 0xf0) == 0x70)
        {
        for (int i = 0; i<12; i++) 
                {
                  printf("%02x ",  PS2data[i]);
                }
        }   
    */
printf("\n");

#endif

   last_buttons = buttons; //store the previous buttons states

#if defined(__AVR__)
   buttons = *(uint16_t*)(PS2data+3);   //store as one value for multiple functions
#else
   buttons =  (uint16_t)(PS2data[4] << 8) + PS2data[3];   //store as one value for multiple functions
#endif
   last_read = millis();
   memcpy(&btns, &PS2data[1], sizeof(btns));
   return btns;
}

void PS2X::setupPins(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat)
{
	_clk = clk;
	_cmd = cmd;
	_att = att;
	_dat = dat;
	
	  pinMode(clk, OUTPUT); //configure ports
	  pinMode(att, OUTPUT);
	  pinMode(cmd, OUTPUT);
	  pinMode(dat, INPUT);
}
/****************************************************************************************/
byte PS2X::config_gamepad(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat, bool pressures, bool rumble) 
{
	byte temp[sizeof(type_read)];
	
	delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
	if (wiringPiSetup () == -1)
	{
		printf ("Unable to start wiringPi:\n");
		return 1 ;
	}
	
	setupPins(clk, cmd, att, dat);

  CMD_SET(); // SET(*_cmd_oreg,_cmd_mask);
  CLK_SET();

  //new error checking. First, read gamepad a few times to see if it's talking
  read_gamepad();
  read_gamepad();

  //see if it talked - see if mode came back. 
  //If still anything but 41, 73 or 79, then it's not talking
  if(PS2data[1] != 0x41 && PS2data[1] != 0x73 && PS2data[1] != 0x79){ 
#ifdef PS2X_DEBUG
    printf("Controller mode not matched or no controller found");
    printf("Expected 0x41, 0x73 or 0x79, but got ");
    printf("%02x", PS2data[1]);
#endif
    return 1; //return error code 1
  }

  //try setting mode, increasing delays if need be.
  read_delay = 1;

  for(int y = 0; y <= 10; y++) {
    sendCommandString(enter_config, sizeof(enter_config)); //start config run

    //read type
    delayMicroseconds(CTRL_BYTE_DELAY);

    CMD_SET();
    CLK_SET();
    ATT_CLR(); // low enable joystick

    delayMicroseconds(CTRL_BYTE_DELAY);

    for (int i = 0; i<9; i++) {
      temp[i] = _gamepad_shiftinout(type_read[i]);
    }

    ATT_SET(); // HI disable joystick

    controller_type = temp[3];

    sendCommandString(set_mode, sizeof(set_mode));
    if(rumble){ sendCommandString(enable_rumble, sizeof(enable_rumble)); en_Rumble = true; }
    if(pressures){ sendCommandString(set_bytes_large, sizeof(set_bytes_large)); en_Pressures = true; }
    sendCommandString(exit_config, sizeof(exit_config));

    read_gamepad();

    if(pressures){
      if(PS2data[1] == 0x79)
        break;
      if(PS2data[1] == 0x73)
        return 3;
    }

    if(PS2data[1] == 0x73)
      break;

    if(y == 10){
#ifdef PS2X_DEBUG
      printf("Controller not accepting commands");
      printf("mode stil set at");
      printf("%02x", PS2data[1]);
#endif
      return 2; //exit function with error
    }
    read_delay += 1; //add 1ms to read_delay
  }
  return 0; //no error if here
}

/****************************************************************************************/
void PS2X::sendCommandString(byte string[], byte len) {
#ifdef PS2X_COM_DEBUG
  byte temp[len];
  ATT_CLR(); // low enable joystick
  delayMicroseconds(CTRL_BYTE_DELAY);

  for (int y=0; y < len; y++)
    temp[y] = _gamepad_shiftinout(string[y]);

  ATT_SET(); //high disable joystick
  delay(read_delay); //wait a few

  printf("OUT:IN Configure");
  for(int i=0; i<len; i++) {
    printf("%02x:%02x ", string[i], temp[i]);
  }
  printf("\n");
#else
  ATT_CLR(); // low enable joystick
  delayMicroseconds(CTRL_BYTE_DELAY);
  for (int y=0; y < len; y++)
    _gamepad_shiftinout(string[y]);
  ATT_SET(); //high disable joystick
  delay(read_delay);                  //wait a few
#endif
}

/****************************************************************************************/
byte PS2X::readType() {
/*
  byte temp[sizeof(type_read)];

  sendCommandString(enter_config, sizeof(enter_config));

  delayMicroseconds(CTRL_BYTE_DELAY);

  CMD_SET();
  CLK_SET();
  ATT_CLR(); // low enable joystick

  delayMicroseconds(CTRL_BYTE_DELAY);

  for (int i = 0; i<9; i++) {
    temp[i] = _gamepad_shiftinout(type_read[i]);
  }

  sendCommandString(exit_config, sizeof(exit_config));

  if(temp[3] == 0x03)
    return 1;
  else if(temp[3] == 0x01)
    return 2;

  return 0;
*/

  if(controller_type == 0x03)
    return 1;
  else if(controller_type == 0x01)
    return 2;
  else if(controller_type == 0x0C)  
    return 3;  //2.4G Wireless Dual Shock PS2 Game Controller
	
  return 0;
}

/****************************************************************************************/
void PS2X::enableRumble() {
  sendCommandString(enter_config, sizeof(enter_config));
  sendCommandString(enable_rumble, sizeof(enable_rumble));
  sendCommandString(exit_config, sizeof(exit_config));
  en_Rumble = true;
}

/****************************************************************************************/
bool PS2X::enablePressures() {
  sendCommandString(enter_config, sizeof(enter_config));
  sendCommandString(set_bytes_large, sizeof(set_bytes_large));
  sendCommandString(exit_config, sizeof(exit_config));

  read_gamepad();
  read_gamepad();

  if(PS2data[1] != 0x79)
    return false;

  en_Pressures = true;
    return true;
}

/****************************************************************************************/
void PS2X::reconfig_gamepad(){
  sendCommandString(enter_config, sizeof(enter_config));
  sendCommandString(set_mode, sizeof(set_mode));
  if (en_Rumble)
    sendCommandString(enable_rumble, sizeof(enable_rumble));
  if (en_Pressures)
    sendCommandString(set_bytes_large, sizeof(set_bytes_large));
  sendCommandString(exit_config, sizeof(exit_config));
}

/****************************************************************************************/

// On pic32, use the set/clr registers to make them atomic...
 void  PS2X::CLK_SET(void) {
digitalWrite(_clk, HIGH); 
}

 void  PS2X::CLK_CLR(void) {
  digitalWrite(_clk, LOW); 
}

 void  PS2X::CMD_SET(void) {
  digitalWrite(_cmd, HIGH);
}

 void  PS2X::CMD_CLR(void) {
  digitalWrite(_cmd, LOW); 
}

 void  PS2X::ATT_SET(void) {
  digitalWrite(_att, HIGH); 
}

 void PS2X::ATT_CLR(void) {
  digitalWrite(_att, LOW); 
}

 bool PS2X::DAT_CHK(void) {
  if (digitalRead(_dat))
	return true;
	return false;
}

bool PS2X::setup(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat)
{
    int error = 0;
    byte type = 0;
    
 	printf("start setup\n");  
    error = config_gamepad(clk, cmd, att, dat);
    printf("ps2 setup:%d\n",  error);
  
  if(error == 0)
  {
    printf("Found Controller, configured successful ");
    printf("Try out all the buttons, X will vibrate the controller, faster as you press harder;\n");
    printf("holding L1 or R1 will print out the analog stick values.\n");
    printf("Note: Go to www.billporter.info for updates and to report bugs.\n");
  }  
  else if(error == 1)
    printf("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips\n");
   
  else if(error == 2)
    printf("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips\n");

  else if(error == 3)
    printf("Controller refusing to enter Pressures mode, may not support it. \n");
    
    if (error)
        return false;
  
  type = readType(); 
  switch(type) {
    case 0:
      printf("Unknown Controller type found ");
      break;
    case 1:
      printf("DualShock Controller found ");
      break;
    case 2:
      printf("GuitarHero Controller found ");
      break;
	case 3:
      printf("Wireless Sony DualShock Controller found ");
      break;
   }
   
   return true;
}

double PS2X::read()
{
    double buttons = read_gamepad(); //read controller and set large motor to spin at 'vibrate' speed
    return buttons;
    
    for (int i=0; i<sizeof(buttons); i++)
    {
        printf("%02x ", ((char*)&buttons)[i]);
    }
    printf("\n");
    
    if(Button(PSB_START))         //will be TRUE as long as button is pressed
      printf("Start is being held\n");
    if(Button(PSB_SELECT))
      printf("Select is being held\n");      

    if(Button(PSB_PAD_UP)) {      //will be TRUE as long as button is pressed
      printf("Up held this hard: ");
    }
    if(Button(PSB_PAD_RIGHT)){
      printf("Right held this hard: \n");
    }
    if(Button(PSB_PAD_LEFT)){
      printf("LEFT held this hard: \n");
    }
    if(Button(PSB_PAD_DOWN)){
      printf("DOWN held this hard: \n");
    }   

    int vibrate = Analog(PSAB_CROSS);  //this will set the large motor vibrate speed based on how hard you press the blue (X) button
    if (NewButtonState()) {        //will be TRUE if any button changes state (on to off, or off to on)
      if(Button(PSB_L3))
        printf("L3 pressed\n");
      if(Button(PSB_R3))
        printf("R3 pressed\n");
      if(Button(PSB_L2))
        printf("L2 pressed\n");
      if(Button(PSB_R2))
        printf("R2 pressed\n");
      if(Button(PSB_TRIANGLE))
        printf("Triangle pressed\n");        
    }

    if(ButtonPressed(PSB_CIRCLE))               //will be TRUE if button was JUST pressed
      printf("Circle just pressed\n");
    if(NewButtonState(PSB_CROSS))               //will be TRUE if button was JUST pressed OR released
      printf("X just changed\n");
    if(ButtonReleased(PSB_SQUARE))              //will be TRUE if button was JUST released
      printf("Square just released\n");     

    if(Button(PSB_L1) || Button(PSB_R1)) { //print stick values if either is TRUE
      printf("Stick Values:...\n");
    }     
    delay(50);
}
