/*********************************************************************
 *
 *                Lyre for 8X2A + 2 VNH
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  aug 28 2015     Original.
                     apr 11 2016     Update to latest Fraise & change mag hmc5883 for incremental sensor + zero
 ********************************************************************/

/*
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
*/
#define BOARD 8X2A

#include <fruit.h>


#include <pid.h>
#include <ramp.h>
#include <dcmotor.h>
#include <analog.h>
#include <switch.h>

//-------------  Timer1 macros :  ---------------------------------------- 
//prescaler=PS fTMR1=FOSC/(4*PS) nbCycles=0xffff-TMR1init T=nbCycles/fTMR1=(0xffff-TMR1init)*4PS/FOSC
//TMR1init=0xffff-(T*FOSC/4PS) ; max=65536*4PS/FOSC : 
//ex: PS=8 : T=0.01s : TMR1init=0xffff-15000
//Maximum 1s !!
#define	TMR1init(T) (0xffff-((T*FOSC)/32000)) //ms ; maximum: 8MHz:262ms 48MHz:43ms 64MHz:32ms
#define	TMR1initUS(T) (0xffff-((T*FOSC)/32000000)) //us ; 
#define InitTimer(T) do{ TMR1H=TMR1init(T)/256 ; TMR1L=TMR1init(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define InitTimerUS(T) do{ TMR1H=TMR1initUS(T)/256 ; TMR1L=TMR1initUS(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define TimerOut() (PIR1bits.TMR1IF)

DCMOTOR_DECLARE(D);

long int TestVar,TestVar2;

void highInterrupts()
{
	if(PIR1bits.TMR1IF) {
		DCMOTOR_CAPTURE_SERVICE(D);
		InitTimerUS(10);
	}
}

t_delay mainDelay;

unsigned char PERIOD=25;
unsigned char t=25,t2;

char disablePotSecurity = 0;

// --------- Fraise Watchdog : ----------------//
unsigned int wdC = 0; //watchdog count

void wdReset(void)
{
	wdC = 0;
}

#define wdOK() (wdC < (200*2)) // 2 seconds
#define wdService() do {if(wdOK()) wdC++;} while(0)
//----------------------------------------------//

void setup(void)
{
	fruitInit();
	switchInit();
	
// ------- init PWM1 to pulse MOTC_IN1
	//PSTR1CON=0;
	//PSTR1CONbits.STR1D=1;

// ------- init PWM2 to pulse MOTD_EN

	//PSTR2CON=0;
	//PSTR2CONbits.STR2B=1;

// ---------- capture timer : TMR1 ------------
	T1CON=0b00110011;//src=fosc/4,ps=8,16bit r/w,on.
	PIE1bits.TMR1IE=1;  //1;
	IPR1bits.TMR1IP=1;
	
	dcmotorInit(D);
#define HW_PARAMS	
#ifdef HW_PARAMS
	//DCMOTOR(C).Setting.PosWindow = 1;
	DCMOTOR(D).Setting.PwmMin = 50;
	//DCMOTOR(D).Setting.PosErrorGain = 6;
	//DCMOTOR(D).Setting.onlyPositive = 0;
	
	DCMOTOR(D).PosRamp.maxSpeed = 900;
	DCMOTOR(D).PosRamp.maxAccel = 1200;
	DCMOTOR(D).PosRamp.maxDecel = 1200;
	//rampSetPos(&DCMOTOR(C).PosRamp, 0);

	DCMOTOR(D).PosPID.GainP = 150; //90
	DCMOTOR(D).PosPID.GainI = 2;
	DCMOTOR(D).PosPID.GainD = 0;
	DCMOTOR(D).PosPID.MaxOut = 1023;

	//DCMOTOR(C).VolVars.homed = 0;
#endif
	analogInit();
	analogSelect(0, MOTD_POT);
	
	switchInit();
    //switchSelect(0,K7); 
#ifndef HW_PARAMS
	EEreadMain();
#endif
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms
}

// ---------- Main loop ------------
int pot0;
#define POT_PERCENT(x)  ((int)((x*32768UL)/100))

void loop() {
	fraiseService();
	analogService();
	fraiseService();
	switchService();

	if(delayFinished(mainDelay)) // when mainDelay triggers 
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay

		//DCMOT_GETPOS(X) = Analog_Get(0);
		if(!switchSend()) analogSend();
		pot0 = analogGet(0);
		wdService();
		if(!disablePotSecurity) {
			if(!wdOK() || (pot0 < POT_PERCENT(4)) || (pot0 > POT_PERCENT(96))) {
				DCMOTOR(D).Vars.PWMConsign = 0;
				DCMOTOR(D).Setting.Mode = 0;
			}
		}

		DCMOTOR_COMPUTE(D, ASYM);

		fraiseService();
		
		if(!t--){
			t=PERIOD;
			printf("C MC %ld %ld %d %d\n",DCMOTOR_GETPOS(D),(long)(DCMOTOR(D).PosRamp.destPos>>RAMP_UINCPOW), DCMOTOR(D).Vars.PWMConsign,DCMOTOR(D).VolVars.homed);
		}
	}
}


void fraiseReceiveChar()
{
	unsigned char c;
	unsigned char l = fraiseGetLen();	
	c=fraiseGetChar();
	if(c=='L'){	
		c=fraiseGetChar();
		/*if(c=='0') 
			{LED=0;}
		else LED=1;*/
	}
	else if(c=='E') {
		printf("C");
		for(c=1;c<l;c++) printf("%c",fraiseGetChar());
		putchar('\n');
	}
	else if(c=='W') { //watchdog
		wdReset();
	}
	else if(c=='S') { //EEPROM save
		if((fraiseGetChar() == 'A')
		&& (fraiseGetChar() == 'V')
		&& (fraiseGetChar() == 'E'))
			EEwriteMain();
	}	
	else if(c=='F') { //force motor
		if((fraiseGetChar() == 'O')
		&& (fraiseGetChar() == 'R')
		&& (fraiseGetChar() == 'C')
		&& (fraiseGetChar() == 'E'))
		disablePotSecurity = (fraiseGetChar()!='0');
	}	
}


void fraiseReceive()
{
	unsigned char c;//,c2;
	
	c=fraiseGetChar();

	switch(c) {
		PARAM_CHAR(1,t2); break;
		PARAM_CHAR(2,PERIOD); break;
		//case 20 : Servo_Input(); break;
		case 120 : DCMOTOR_INPUT(D); break;
		//case 121 : DCMOTOR_INPUT(D); break;
	}
}

void EEdeclareMain()
{
	DCMOTOR_DECLARE_EE(D);
	//DCMOTOR_DECLARE_EE(D);
}
