
#include "PS2X_lib.h"  //for v1.6

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original 
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        8  //14    This pin will need a pull up resistor 
#define PS2_CMD        9  //15
#define PS2_SEL        7  //16
#define PS2_CLK        15  //17

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/


//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you connect the controller, 
//or call config_gamepad(pins) again after connecting the controller.
extern "C" {
PS2X* new_ps2()
{
    PS2X *ps2x = new PS2X;
    return ps2x;
}

int setup_ps2(PS2X* ps2x)
{
	if (ps2x->setup(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT))
        return 0;
    return -1;
}

double read_ps2(PS2X* ps2)
{
    if (!ps2)
        return 0;
    return ps2->read();
}
    
int main()
{
    double buttons  = 0;
    PS2X *ps2x = new_ps2();
    if (setup_ps2(ps2x))
        return 1;

	while (1)
	{
        buttons = read_ps2(ps2x);
    }
	return 0;
}
}
