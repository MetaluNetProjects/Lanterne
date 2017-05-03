/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
//#include <softpwm.h>

t_delay mainDelay;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- softpwm setup ----------------
//	softpwmInit();		// init analog module

//-----------------
	T2CONbits.T2CKPS1=1; // Timer2 prescaler 16
	analogWrite(K4,0);
	analogWrite(K6,0);
	analogWrite(K10,0);
	pinModeAnalogOut(K4);
	pinModeAnalogOut(K6);
	pinModeAnalogOut(K10);
}

// ---------- Main loop ------------
void loop() {
	fraiseService();	// listen to Fraise events
}

// ---------- Interrupts ------------
/*void highInterrupts()
{
	softpwmHighInterrupt();
}
void lowInterrupts()
{
	softpwmLowInterrupt();
}*/

// Receiving

void fraiseReceive() // receive raw bytes
{
	unsigned char c=fraiseGetChar();
	unsigned int i;
	/*if(c==50) softpwmReceive(); // if first byte is 50, then call softpwm receive function.
	else*/ if(c==51) {
		i = fraiseGetInt();
		analogWrite(K4,i);
	}
	else if(c==52) {
		i = fraiseGetInt();
		analogWrite(K6,i);
	}
	else if(c==53) {
		i = fraiseGetInt();
		analogWrite(K10,i);
	}
}

