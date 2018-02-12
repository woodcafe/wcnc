
#include "PS2X_lib.h"  //for v1.6

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original 
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        12  //14    This pin will need a pull up resistor 
#define PS2_CMD        13  //15
#define PS2_SEL        14  //16
#define PS2_CLK        10  //17

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false

PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you connect the controller, 
//or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte type = 0;
byte vibrate = 0;

void setup(){
 

  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
   
  //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  printf("ps2 setup:%d\n", error);
  
  if(error == 0){
    printf("Found Controller, configured successful ");
    printf("pressures = ");
	if (pressures)
	  printf("true ");
	else
	  printf("false");
	printf("rumble = ");
	if (rumble)
	  printf("true)");
	else
	  printf("false");
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
  
//  printf(ps2x.Analog(1), HEX);
  
  type = ps2x.readType(); 
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
}

void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
  if(error == 1) //skip loop if no controller found
    return; 
  
  if(type == 2){ //Guitar Hero Controller
    ps2x.read_gamepad();          //read controller 
   
    if(ps2x.ButtonPressed(GREEN_FRET))
      printf("Green Fret Pressed");
    if(ps2x.ButtonPressed(RED_FRET))
      printf("Red Fret Pressed");
    if(ps2x.ButtonPressed(YELLOW_FRET))
      printf("Yellow Fret Pressed");
    if(ps2x.ButtonPressed(BLUE_FRET))
      printf("Blue Fret Pressed");
    if(ps2x.ButtonPressed(ORANGE_FRET))
      printf("Orange Fret Pressed"); 

    if(ps2x.ButtonPressed(STAR_POWER))
      printf("Star Power Command");
    
    if(ps2x.Button(UP_STRUM))          //will be TRUE as long as button is pressed
      printf("Up Strum");
    if(ps2x.Button(DOWN_STRUM))
      printf("DOWN Strum");
 
    if(ps2x.Button(PSB_START))         //will be TRUE as long as button is pressed
      printf("Start is being held");
    if(ps2x.Button(PSB_SELECT))
      printf("Select is being held");
    
    if(ps2x.Button(ORANGE_FRET)) {     // print stick value IF TRUE
      printf("Wammy Bar Position:");
      printf("%d\n", ps2x.Analog(WHAMMY_BAR)); 
    } 
  }
  else { //DualShock Controller
    ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed
    
    if(ps2x.Button(PSB_START))         //will be TRUE as long as button is pressed
      printf("Start is being held");
    if(ps2x.Button(PSB_SELECT))
      printf("Select is being held");      

    if(ps2x.Button(PSB_PAD_UP)) {      //will be TRUE as long as button is pressed
      printf("Up held this hard: ");
      printf("%d\n", ps2x.Analog(PSAB_PAD_UP));
    }
    if(ps2x.Button(PSB_PAD_RIGHT)){
      printf("Right held this hard: ");
      printf("%d\n", ps2x.Analog(PSAB_PAD_RIGHT));
    }
    if(ps2x.Button(PSB_PAD_LEFT)){
      printf("LEFT held this hard: ");
      printf("%d\n", ps2x.Analog(PSAB_PAD_LEFT));
    }
    if(ps2x.Button(PSB_PAD_DOWN)){
      printf("DOWN held this hard: ");
      printf("%d\n", ps2x.Analog(PSAB_PAD_DOWN));
    }   

    vibrate = ps2x.Analog(PSAB_CROSS);  //this will set the large motor vibrate speed based on how hard you press the blue (X) button
    if (ps2x.NewButtonState()) {        //will be TRUE if any button changes state (on to off, or off to on)
      if(ps2x.Button(PSB_L3))
        printf("L3 pressed");
      if(ps2x.Button(PSB_R3))
        printf("R3 pressed");
      if(ps2x.Button(PSB_L2))
        printf("L2 pressed");
      if(ps2x.Button(PSB_R2))
        printf("R2 pressed");
      if(ps2x.Button(PSB_TRIANGLE))
        printf("Triangle pressed");        
    }

    if(ps2x.ButtonPressed(PSB_CIRCLE))               //will be TRUE if button was JUST pressed
      printf("Circle just pressed");
    if(ps2x.NewButtonState(PSB_CROSS))               //will be TRUE if button was JUST pressed OR released
      printf("X just changed");
    if(ps2x.ButtonReleased(PSB_SQUARE))              //will be TRUE if button was JUST released
      printf("Square just released");     

    if(ps2x.Button(PSB_L1) || ps2x.Button(PSB_R1)) { //print stick values if either is TRUE
      printf("Stick Values:");
      printf("%d\n",  ps2x.Analog(PSS_LY)); //Left stick, Y axis. Other options: LX, RY, RX  
      printf(",");
      printf("%d\n", ps2x.Analog(PSS_LX)); 
      printf(",");
      printf("%d\n", ps2x.Analog(PSS_RY)); 
      printf(",");
      printf("%d\n", ps2x.Analog(PSS_RX)); 
    }     
  }
  delay(50);  
}

int main()
{
	if (wiringPiSetup () == -1)
	{
		printf ("Unable to start wiringPi:\n");
		return 1 ;
	}
	
	setup();
	while (1)
	{
			loop();
	}
	return 0;
}


int main1()
{
	if (wiringPiSetup () == -1)
	{
		printf ("Unable to start wiringPi:\n");
		return 1 ;
	}
	ps2x.setupPins(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT);
	if (ps2x.DAT_CHK())
		printf("data on\n");
	else
		printf("data off\n");
	ps2x.CMD_SET();
	ps2x.CLK_SET();
	ps2x.ATT_SET();
	sleep(10);
	ps2x.CMD_CLR();
	ps2x.CLK_CLR();
	ps2x.ATT_CLR();
	return 0;
}
