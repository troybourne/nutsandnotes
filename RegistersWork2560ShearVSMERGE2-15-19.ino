// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       RegistersWork2560ShearVSMERGE.ino
    Created:  2/11/2019 1:30PM
    Author:     ACORN\TROY
*/

// Define User Types below here or use a .h file
#include "IRremote.h"
//
//TO DO NOTES... limit movement of toStack and toConveyor to when carriage is up***********************
//TO DO NOTES... current sensor will need an analog so convert PORTF or K over to PORTC
// Define Function Prototypes that use User Types below here or use a .h file
void IR();
void exitingEStop();
void forwardChain();
void reverseChain();
void stopChain();
void toStack();
void toConveyor();
void stopBridgeMotor();
void raiseCarriage();
void lowerCarriage();
void leftVacEngage();
void rightVacEngage();
void bothVacEngage();
void bothVacRelease();
// Define Functions below here or use other .ino or cpp files
//
// The setup() function runs once each time the micro-controller starts
// NOTES TO KEEP IN MIND*********
//PORTF HAS at least 5 availble outputs
//PORT MAPPING
//PINL ON MEGA 44,45,46,47,48,49 49=LSB  L is proximity sensors INPUTS
//PORTF on MEGA analog A7, A6, A5, A4, A3, A2, A1, A0 ALL OUTPUTS
//A0 = DROP ON LOW, A1 = EXTEND ON LOW, 
// DDRC ON MEGA   30, 31, 32, 33, 34, 35, 36, 37
//IR with 3 LED for Auto(green), Manual(yellow), EStop(red) all on PORTD 
//PORTD is as followed 38,x,x,x,18,19,20,21 with 20 and 21 as inputs... 20 EStop Reset, 21 IRSignal pin
// Auto(green)19 bit 4, Manual(yellow) bit 8, EStop(red) bit 128 -- all on PORTD
//
/*NOTES
 * the six NPN proximity sensors are in PORTL 
 * correlate to binary values 32,16,8,4,2,1 respectively of PINL register
 */
#define VERIFY_PUSHER_STATE 0
#define DROP_PUSHER 1
#define EXTEND_PUSHER 2
#define RETRACT_PUSHERS 3
#define RAISE_PUSHERS 4
#define HOME_AND_VERIFY_PUSHERS 5

int pusherState = 0;
bool pusherCanRun = 0;
bool leftRetractTripped = 0;
bool rightRetractTripped = 0;
int pusherCanRunDelay = 2500; // delay to prevent restarting pusherStateMachine because of chainPosSensor
int forwardChainInvokedTime = 0;
int chainPosSensor = 4;
int returnStatePinReadF = 0;
int returnStatePinReadK = 0;
int bridgeStopDelay = 1000;
int intializeStopBridge = 0;

int receiver = 14; // pin 1 of IR receiver to Arduino digital pin 11
IRrecv irrecv(receiver); // create instance of 'irrecv'
decode_results results; // create instance of 'decode_results'

// DDR(bank) - 1 is to assign an output, 0 is an input
// PORT(bank) - if output then 0 is LOW and 1 is high; if input then then 1 is INPUT_PULLUP and 0 is no pullup enabled
// PIN(bank) - reads the value of that register

void setup()
{
	
	pinMode(chainPosSensor, INPUT_PULLUP);
	digitalWrite(chainPosSensor, HIGH);
  Serial.begin(115200);
  delay(250);
  Serial.println("IR Receiver Raw Data + Button Decode Test");
  irrecv.enableIRIn(); // Start the receiver
  //DDRD = DDRD | B1111110; //b128 redLED(pin38), b8 yellowLED(pin18), b4 greenLED(pin19), b2 inputEStopHardReset, b1 IRSignal
  //PORTD = B00000010; //b2 is PULLUP
  pinMode(38, OUTPUT);
  digitalWrite(38, LOW);
  DDRC = DDRC | B00000000; //30, 31, 32, 33, 34, 35, 36, 37 all inputs
  PORTC = B00000000; //supposed to assign C to LOW ****PORTC NOT BEING USED*********
  DDRB = DDRB | B00000000; //13, 12, 11, 10, 50, 51, 52 ,53 all inputs
  PORTB = B11111111; // red dip switches... 
  DDRF = B11111111; //analog A7, A6, A5, A4, A3, A2, A1, A0 ALL OUTPUTS
  PORTF = B11111111;
  DDRK = B11111111;
  PORTK = B11111111;
  DDRL = B11000000;
  PORTL = B00111111;
  //PORTC = PORTC | B111111;
  //pinMode(18, INPUT_PULLUP);
  //attachInterrupt(5, pusherStateMachine, FALLING);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  pinMode(20, INPUT_PULLUP);
  digitalWrite(20, HIGH);
  pinMode(21, INPUT_PULLUP);
  digitalWrite(21, HIGH);
  
  attachInterrupt(0, pusherCanRaiseFlagL, FALLING);
  attachInterrupt(1, pusherCanRaiseFlagR, FALLING);
  attachInterrupt(2, stopBridgeAtConveyor, FALLING);
  attachInterrupt(3, stopBridgeAtStack, FALLING);
}

void pusherCanRaiseFlagL() {
	leftRetractTripped = 1;
	Serial.println("interrupt worked");
	//pusherState = 3;
	//pusherStateMachine();
	return;
}
void pusherCanRaiseFlagR() {
	rightRetractTripped = 1;
	Serial.println("interrupt worked");
	//pusherState = 3;
	//pusherStateMachine();
	return;
}
void IR()
{
  //Serial.println("main loop has been reached");
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    Serial.print(F("result = 0x"));
    Serial.println(results.value, HEX); //UN Comment to see raw values
    translateIR();
    irrecv.resume(); // receive the next value
  }
}/* --(end IR loop )-- */



void reverseChain() {
	if (PINF == B11110111) {
		return;
	}
	PORTF = PORTF | B00001100;
	PORTF = PORTF & B11110111;
	pusherCanRun = 1;
	return;
}
void forwardChain() {
	PORTF = PORTF & B11111011; 
	forwardChainInvokedTime = millis();
	while(millis() - forwardChainInvokedTime <= pusherCanRunDelay){
		
	}
		pusherCanRun = 1;
		return;
}
//void forwardChain() {
	//PORTF = PORTF & B11111011;  // turn forward relay on
	//forwardChainInvokedTime = millis();
	//while (millis() < forwardChainInvokedTime + pusherCanRunDelay) {
	//	pusherCanRun = 0;
	//}
	//pusherCanRun = 1;
	//return;
//}
void stopChain() {
		if (PING >= 32 && pusherCanRun == 1) {
			pusherStateMachine();
		}
		PORTF = PORTF | B00001100;
		//delay(750);
		return;
}
void raiseCarriage() {
	PORTK = PORTK & B10111111;
	return;
}
void lowerCarriage() {
	PORTK = PORTK | B01000000;
	return;
}
void carriageToStack() {
	Serial.println(PORTK, DEC);
	if (PORTK == B01111111) {
		return;
	}
	stopBridge();
	//PORTK = PORTK | B00100000;
	PORTK = PORTK & B01111111;
	return;
}
void carriageToConveyor() { 
	Serial.println(PORTK, DEC);
	if (PORTK == B11011111) {
		return;
	}
	stopBridge();
	//PORTK = PORTK | B10000000;
	//PORTD = B00001000;
	PORTK = PORTK & B11011111;
	return;
}

/*
something started glitching when the while loop and millis was being used
or when the calls to stopBridge were made in toCoveyor and toStack
but i think mostly its the while loop*/
void stopBridgeAtStack() {
	PORTK = PORTK | B10000000;
	return;
}
void stopBridgeAtConveyor() {
	PORTK = PORTK | B00100000;
	return;
}
void stopBridge() {
	Serial.println(PORTD, DEC);
	intializeStopBridge = millis();
	PORTK = PORTK | B10100000;
	while (millis() - intializeStopBridge <= bridgeStopDelay) {
	}
	//if (digitalRead(21) == LOW) {
		//Serial.println("Bridge At Stack");
		//return;
	//}
	if (digitalRead(20) == LOW) {
		Serial.println("Bridge At Conveyor");
		return;
	}
	if (digitalRead(20) == LOW && digitalRead(21) == LOW) {
		Serial.println("Sensor Malfunction - adjust switch");
		return;
	}
	else {
		Serial.println("Bridge in transit");
	}
	return;
}

void EStop()
{
	returnStatePinReadF = (PINF);  //saves state of motors, relays for returning after EStop
	returnStatePinReadK = (PINK);  //saves state of motors, relays for returning after EStop
	PORTF = B01111111; // EStop relay engaged
	PORTK = PORTK | B10100000; // Stop forward or reverse bridge motor
	//PORTD = B10000000; // EStop beacon LIT
	digitalWrite(38, HIGH);
	while (PINB < 128) {
		Serial.println("EStop hard button entered EStop Mode");
	}
	while (PINB >= 128) {
		Serial.println("EStop remote button exiting EStop Mode");
		//PORTD = B00000000; // EStop beacon out for exitingEStop flashing
		digitalWrite(38, LOW);
		Serial.println("breaking out of EStop");
		exitingEStop();
		return;
	}
}
void exitingEStop() // flashed EStop beacon and chirp alarm for 5 seconds
{
	//PORTD = B10000000;
	digitalWrite(38, HIGH);
	delay(25);
	Serial.println("hard e stop reached");
	Serial.println(PIND);
		while ((PINB) <128) {
			Serial.println("waiting for e-stop reset button to be engaged pressed");
			delay(10);
			Serial.println(PIND);
			delay(15);
		}
  while ((PINB) >=128) {
		Serial.println("waiting for e-stop reset button to be released pulled");
		Serial.println(PIND);
		delay(15);
  }
  Serial.println("exiting E Stop mode");
  //below for loop can display strobe and beeper for exiting EStop mode notification
  for (int i = 0; i <= 10; i++) {
    //PORTD = B10000000;
	  digitalWrite(38, HIGH);
    delay(250);
    //PORTD = B00000000;
	digitalWrite(38, LOW);
    delay(250);
  }
  // PORTD = B00100000;
  /*
  PORTD = PORTD ^ 4; //This will toggle only the bit in the 4 column
  delay(1500);
  Serial.println(PIND);
  delay(250);
  PORTD = PORTD ^ 28; //this will toggle the bits that add to 28, namely 4, 8 and 16 turning on 8 and 16
  delay(1500);
  Serial.println(PIND);
  PORTD = PORTD ^ 24; //this will flip bits for 8 and 16 off
  */
  PORTF = returnStatePinReadF;  // These return the relays, motors, to their state before EStop
  PORTK = returnStatePinReadK;  // These return the relays, motors, to their state before EStop
  PORTF = PORTF | B10000000; // EStop relay DIS engaged
  return;
}

void translateIR() // takes action based on IR code received
{
  switch (results.value)
  {
  case 0xE318261B: {
	  Serial.println(" E-Stop");
	  PORTD = B10000000;
	  EStop();
	  return;
  case 0xFFA25D: Serial.println(" E-Stop");
	  PORTD = B10000000;
	  EStop();
	  return;
  case 0xFFA25: Serial.println(" E-Stop");
	  PORTD = B10000000;
	  EStop();
	  return; }
  case 0x511DBB: {Serial.println(" Manual Mode"); break;
  case 0xFF629D: Serial.println(" Manual Mode"); break;
  case 0xFF629: Serial.println(" Manual Mode"); break; }
  case 0xEE886D7F: { Serial.println(" Auto Mode"); break;
  case 0xFFE21D: Serial.println(" Auto Mode"); break;
  case 0xFFE21: Serial.println(" Auto Mode"); break; }
  case 0x52A3D41F: {Serial.println(" Stop Bridge");
	  stopBridge();
	  break;
  case 0xFF22DD: Serial.println(" Stop Bridge");
	  stopBridge();
	  break;
  case 0xFF22D: Serial.println(" Stop Bridge");
	  stopBridge();
	  break; }
  case 0xD7E84B1B:{ Serial.println(" To Conveyor");
	carriageToConveyor();
    break;
  case 0xFF02FD: Serial.println(" To Conveyor");
	  carriageToConveyor();
	  break;
  case 0xFF02F: Serial.println(" To Conveyor");
	  carriageToConveyor();
	  break; }
  case 0x20FE4DBB: {Serial.println(" To Stack");
	  carriageToStack();
	  PORTD = B00000100;
	  break;
  case 0xFFC23D: Serial.println(" To Stack");
	  carriageToStack();
	  PORTD = B00000100;
	  break;
  case 0xFFC23: Serial.println(" To Stack");
	  carriageToStack();
	  PORTD = B00000100;
	  break; }
  case 0xF076C13B: Serial.println(" Home the Pusher"); 
	  PORTF = PORTF | B00000010;
	  break;
  case 0xFFE01F: Serial.println(" Home the Pusher"); 
	  PORTF = PORTF | B00000010;
	  break;
  case 0xFFE01: Serial.println(" Home the Pusher"); 
	  PORTF = PORTF | B00000010;
	  break;
  case 0xA3C8EDDB: {Serial.println(" LOWER Carriage");
	  lowerCarriage();
	  break;
  case 0xFFA857: Serial.println(" LOWER Carriage");
	  lowerCarriage();
	  break;
  case 0xFFA85: Serial.println(" LOWER Carriage");
	  lowerCarriage();
	  break; }
  case 0xE5CFBD7F: { Serial.println(" RAISE Carriage");
	  raiseCarriage();
	  break;
  case 0xFF906F: Serial.println(" RAISE Carriage");
	  raiseCarriage();
	  break;
  case 0xFF906: Serial.println(" RAISE Carriage");
	  raiseCarriage();
	  break; }
  case 0xC101E57B: Serial.println(" PushPlate To"); break;
  case 0xFF6897: Serial.println(" PushPlate To"); break;
  case 0xFF689: Serial.println(" PushPlate To"); break;
  case 0x97483BFB: Serial.println(" Release Vacuum"); break;
  case 0xFF9867: Serial.println(" Release Vacuum"); break;
  case 0xFF986: Serial.println(" Release Vacuum"); break;
  case 0xF0C41643: Serial.println(" Activate All Vacuum"); break;
  case 0xFFB04F: Serial.println(" Activate All Vacuum"); break;
  case 0xFFB04: Serial.println(" Activate All Vacuum"); break;
  case 0x9716BE3F: Serial.println(" Drop Pusher"); break;
  case 0xFF30CF: Serial.println(" Drop Pusher"); break;
  case 0xFF30C: Serial.println(" Drop Pusher"); break;
  case 0xFF18E7: Serial.println(" Activate Left Vacuum"); break;
  case 0xFF18E: Serial.println(" Activate Left Vacuum"); break;
  case 0x3D9AE3F7: Serial.println(" Activate Left Vacuum"); break;
  case 0x6182021B: Serial.println(" Activate Right Vacuum"); break;
  case 0xFF7A85: Serial.println(" Activate Right Vacuum"); break;
  case 0xFF7A8: Serial.println(" Activate Right Vacuum"); break;
  case 0x8C22657B: {
	  Serial.println(" stop chain");
	  //stopChain();
	  PORTF = PORTF | B00001100;
	  break;
  case 0xFF10EF:
	  Serial.println(" stop chain");
	  //stopChain();
	  PORTF = PORTF | B00001100;
	  break;
  case 0xFF10E:
	  Serial.println(" stop chain");
	  //stopChain();
	  PORTF = PORTF | B00001100;
	  break; }
  case 0x488F3CBB: {
	  Serial.println(" forward chain");
	  forwardChain();
	  break;
  case 0xFF38C7:
	  Serial.println(" forward chain");
	  forwardChain();
	  break;
  case 0xFF38C:
	  Serial.println(" forward chain");
	  forwardChain();
	  break; }
  case 0x449E79F: { Serial.println(" reverse chain");
	  reverseChain();
	  break;
  case 0xFF5AA5: Serial.println(" reverse chain");
	  reverseChain();
	  break;
  case 0xFF5AA: Serial.println(" reverse chain");
	  reverseChain();
	  break; }
  case 0x32C6FDF7: Serial.println(" Diagnostics"); break;
  case 0xFF42BD: Serial.println(" Diagnostics"); break;
  case 0xFF42B: Serial.println(" Diagnostics"); break;
  case 0x1BC0157B: Serial.println(" Reset Microcontroller"); break;
  case 0xFF4AB5: Serial.println(" Reset Microcontroller"); break;
  case 0xFF4AB: Serial.println(" Reset Microcontroller"); break;
  case 0x3EC3FC1B: Serial.println(" Enter Manual Pusher Window"); break;
  case 0xFF52AD: Serial.println(" Enter Manual Pusher Window"); break;
  case 0xFF52A: Serial.println(" Enter Manual Pusher Window"); break;
    // case 0xFF52AD: Serial.println(" #"); break;
    // case 0xFFFFFFFF: Serial.println(" REPEAT");break; 
  default:
    Serial.println(" other button ");
  }
  //delay(150); 
} //END translateIR

/*
void checkChainMotor() {
  if (pusherCanRun == 0) {
    if (PINB == 0) {
      Serial.println("pusherCanRun=true 1");
      pusherCanRun = 1;
    }
    else {
      Serial.println("pusherCanRun=false 0");
      return;
    }
  }
}
*/


void pusherStateMachine()
{
	if (pusherCanRun != 1) {
		return;
	}
	else {
		PORTF = PORTF | B00001100;  //stops the chain whether in forward or reverse
	}

	switch (pusherState)
	{

		Serial.println(PING);
	case VERIFY_PUSHER_STATE:
		if (pusherState == 0) {
			pusherState++;
			PORTF = PORTF | B00110000; //stop chain???

			Serial.print("0 pusherState was verified..... State is---  ");
			Serial.println(pusherState);
		}
		else {
		}
	case DROP_PUSHER:
		PORTF = PORTF & B11111110; //drop valve triggered
		while (PINL != 60) {
		}
		if (PINL == 60) {
			pusherState++;
			Serial.print("Pushers are DROPPED... PusherState is... ");
			Serial.println(pusherState);
		}
		else {
			return;
		}
	case EXTEND_PUSHER:
		PORTF = PORTF & B11111100;
		Serial.print("Pushers are Extending... Pushers are Extending");
		Serial.println(PINL);
		while (PINL != 48) {  //probably NOT NEEDED

			if (irrecv.decode(&results)) // have we received an IR signal?
			{
				Serial.println(results.value);
				if (results.value == 0xF076C13B) {
					pusherState + 2;
					PORTF = PORTF | B00000010;
					//return;
				}
			}
		}
		if (PINL == 48) { //48 means down inputs&extract inputs all simultaneously LOW
			PORTF = PORTF | B00000010;  // this sets retraction to happen
			Serial.print("Pushers are Retracting... Pushers are Retracting");
			pusherState++;
			Serial.print("PusherState is... ");
			Serial.println(pusherState);
		}
	case RETRACT_PUSHERS:
		Serial.println("retract was reached");
		if ((leftRetractTripped + rightRetractTripped) == 2) {
			Serial.println("exiting retract");
			pusherState++;
		}
		else {
			while ((leftRetractTripped + rightRetractTripped) != 2) {
				Serial.println(leftRetractTripped);
				Serial.println(rightRetractTripped);
			}
			Serial.println("exiting retract");
			pusherState++;
		}

	case RAISE_PUSHERS:
		Serial.println("Raise Pushers REACHED");
		leftRetractTripped = 0;
		rightRetractTripped = 0;
		PORTF = PORTF | B00000011;
		pusherState++;

	case HOME_AND_VERIFY_PUSHERS:
		pusherCanRun = 0;
		while (PINL != 63) {
			Serial.println("waiting for pushers to HOME");
			Serial.println(PINL);
		}
		pusherState = 0;
		//if (steelOnChainsL == 0 && steelonChainsR == 0) {
			//forwardChain();
		//}
		//else {
			forwardChain();
		//}
		//checkChainMotor();
		Serial.println("breaking out of pusherStateMachine");
		Serial.println("pusherCanRun equals ");
		Serial.print(pusherCanRun);
		Serial.println(pusherCanRun);
		loop();

	}
}
