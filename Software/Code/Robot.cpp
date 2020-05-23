
#include <Arduino.h>
#include <String.h>
#include <EEPROM.h>
#include <SPI.h>
#include "SPIFlash.h"
#include "IO_Communication.h"
#include "Motor_Route.h"

void receive();
char *strtolower(char *s);
void save();
void read();
void saveInteger(uint32_t addr, int data);
int readInteger(uint32_t addr);

SPIFlash flash(31);

/*
 * 32 Bit Input + 32 Bit Output
 *
 * Input:
 * 0		= Motor Finished
 * 1		= Motor Error
 * 2-15		= Driven_L/Driven_R
 * 16-25	= Ultrasonic 1-5
 * 26		= Button (for reset after error)
 *
 * Output:
 * 0		= RST_Drive
 * 1		= Set Drive_L
 * 2		= Set Drive_R
 * 3-17		= Drive_L/R Distance
 * 18-25	= Drive_Speed
 * 26		= Driven_L/R switch
 * 27-29	= Ultrasonic select
 * 30		= LED (blinks if error)
 * 31		= Flash Enable
 */

void setup() {
	Serial.begin(9600);

	if (flash.initialize()) Serial.println("Init OK!");
	else Serial.println("Init FAIL!");
}

int routeNumber = 0;
int routesL [16][64];
int routesR [16][64];
int lengths [16];

char received [64];

int route;

int curRouteL [128];
int curRouteR [128];
int curRouteStep;
bool recordRoute;

void loop() {
	Serial.println("Ready\n\n\"Read\" = Read saved routes\n\"Record\" = Record new route\n\"Route\" = Drive route\n\"Home\" = Return to home\n\"Delete\" = Delete route\n\"Turn\" = Set length to turn 90deg\n1 = 10cm Forward, 2 = Left, 3 = 10cm Backward, 4 = Right");

	//Receive UART Data

	receive();

	if(!strncmp(strtolower(received), "record", strlen("record"))){ //TODO: Mehrere gleiche Richtungen zu einer Strecke zusammenfassen
		Serial.println("Ok = Finish route\n100,100 = 10cm forward\n-100,100 = Turn left\n-100,-100 = 10cm backward\n100,-100 = Turn right");
		routeNumber ++;

		int routeStep = 0;
		receive();
		//Recieve route steps
		while (strncmp(strtolower(received), "ok", strlen("ok"))){
			//Convert [number],[number] to int
			int comma;
			int i;
			char firstNum  [64];
			char secondNum [64];
			bool second = false;
			for(i = 0; received[i] != '\0'; i++){
				if (received[i] == ','){
					firstNum[i] = '\0';
					comma = i;
					second = true;
				}else if (!second){
					firstNum[i] = received[i];
				}else{
					secondNum[i-comma-1] = received[i];
				}
			}
			secondNum[i-comma-1] = '\0';

			Serial.println(firstNum);
			Serial.println(secondNum);

			int lenL,lenR;
			//Save in array
			sscanf(firstNum,  "%d", &lenL);
			sscanf(secondNum, "%d", &lenR);
			//Check if routes can be combined
			if ((((routesL[routeNumber-1][routeStep-1]>0) == (lenL>0)) == ((routesR[routeNumber-1][routeStep-1]>0) == (lenR>0)))
					&& abs(routesL[routeNumber-1][routeStep-1]) == abs(routesR[routeNumber-1][routeStep-1]) && abs(lenL) == abs(lenR)
					&& !(routesR[routeNumber-1][routeStep-1] == 0 || routesL[routeNumber-1][routeStep-1] == 0 || lenL == 0 || lenR == 0)){
				Serial.println("combine");
				routesL[routeNumber-1][routeStep-1] += lenL;
				routesR[routeNumber-1][routeStep-1] += lenR;
			}
			else{
				Serial.println("new");
				routesL[routeNumber-1][routeStep] = lenL;
				routesR[routeNumber-1][routeStep] = lenR;
				routeStep ++;
			}
			//Drive route step
			startRoute(lenL,lenR,255);
			while (!routeFinished());

			receive();
		}
		lengths[routeNumber-1] = routeStep;

		save();
	}
	else if(!strncmp(strtolower(received), "route", strlen("route"))){
		Serial.print("Select route 1-");
		Serial.println(routeNumber);

		receive();

		sscanf(received,  "%d", &route);
		route--;

		recordRoute = true;
		for (int i = 0; i < lengths[route]; i ++){
			if (!driveRoute(routesL[route][i],routesR[route][i],255)){
				while(!digitalRead(26)){
					digitalWrite(30,1);
					delay(250);
					digitalWrite(30,0);
					delay(250);
				}
			}
		}
		recordRoute = false;
	}
	else if(!strncmp(strtolower(received), "home", strlen("home"))){ //Besser: beim Fahren aufnehmen um auch Umfahren zu tracken
		if (!driveTurn(true) || !driveTurn(true)) {
			while(!digitalRead(26)){
				digitalWrite(30,1);
				delay(250);
				digitalWrite(30,0);
				delay(250);
			}
		}
		for (int i = lengths[route]-1; i >= 0; i --){
			if (!driveRoute(routesR[route][i],routesL[route][i],255)){
				while(!digitalRead(26)){
					digitalWrite(30,1);
					delay(250);
					digitalWrite(30,0);
					delay(250);
				}
			}
		}
	}
	else if(!strncmp(strtolower(received), "delete", strlen("delete"))){
		Serial.print("Select route 1-");
		Serial.println(routeNumber);

		receive();

		int i;
		sscanf(received,  "%d", &i);

		for (; i < routeNumber; i ++){
			for (int j = 0; j < lengths[i]; j ++){
				routesL[i-1][j] = routesL[i][j];
			}
			lengths[i-1] = lengths[i];
		}

		if (routeNumber > 0)
			routeNumber--;

		save();
	}
	else if(!strncmp(strtolower(received), "turn", strlen("turn"))){
		Serial.print("90deg Turn Length:");
		receive();
		sscanf(received,  "%d", &rightAngleTurnDistance);
		startRoute(rightAngleTurnDistance,-rightAngleTurnDistance,255);
		while (!routeFinished());

		save();
	}
	else if(!strncmp(strtolower(received), "read", strlen("read"))){
		read();
	}
	else{
		switch (received[0]){
		case '1':
			Serial.println("Forward");
			startRoute(100,100,255);
			break;
		case '2':
			Serial.println("Left");
			startRoute(-100,100,255);
			break;
		case '3':
			Serial.println("Backward");
			startRoute(-100,-100,255);
			break;
		case '4':
			Serial.println("Right");
			startRoute(100,-100,255);
			break;
		default:
			delay(1000);
		}

		while (!routeFinished()){
			bool leftCollision   = getUltrasonicDistance(1) < minDist;
			bool middleCollision = getUltrasonicDistance(2) < minDist;
			bool rightCollision  = getUltrasonicDistance(3) < minDist;

			digitalWrite(30,leftCollision || middleCollision || rightCollision);
		}
	}
}

void receive(){
	received[0] = '\0';
	while (Serial.available() == 0);
	while (Serial.available() > 0){
		sprintf(received,"%s%c",received,Serial.read());
		delay(10);
	}
}

char *strtolower(char *p)
{
	char* r = p;
	for ( ; *p; ++p) *p = tolower(*p);
	return r;
}

void save(){
	flash.chipErase();
	while(flash.busy()){
		Serial.print(".");
		delay(1000);
	}
	Serial.println("Flash Erased");

	int addr = 1000;
	//Save rightAngleTurnDistance
	saveInteger(addr, rightAngleTurnDistance);
	addr+=4;
	//Save routeNumber
	saveInteger(addr, routeNumber);
	addr+=4;
	//Save lengths
	for (int i = 0; i < routeNumber; i ++){
		saveInteger(addr, lengths[i]);
		addr+=4;
	}
	//Save routesL
	for (int i = 0; i < routeNumber; i ++){
		for (int j = 0; j < lengths[i]; j ++){
			saveInteger(addr, routesL[i][j]);
			addr+=4;
		}
	}
	//Save routesR
	for (int i = 0; i < routeNumber; i ++){
		for (int j = 0; j < lengths[i]; j ++){
			saveInteger(addr, routesR[i][j]);
			addr+=4;
		}
	}
}

void read(){
	int addr = 1000;
	//Read rightAngleTurnDistance
	rightAngleTurnDistance = readInteger(addr);
	addr+=4;
	//Read routeNumber
	routeNumber = readInteger(addr);
	addr+=4;
	//Read lengths
	for (int i = 0; i < routeNumber; i ++){
		lengths[i] = readInteger(addr);
		addr+=4;
	}
	//Save routesL
	for (int i = 0; i < routeNumber; i ++){
		for (int j = 0; j < lengths[i]; j ++){
			routesL[i][j] = readInteger(addr);
			addr+=4;
		}
	}
	//Save routesR
	for (int i = 0; i < routeNumber; i ++){
		for (int j = 0; j < lengths[i]; j ++){
			routesR[i][j] = readInteger(addr);
			addr+=4;
		}
	}
}

void saveInteger(uint32_t addr, int data){
	byte d1 = data;
	byte d2 = data >> 8;
	byte d3 = data >> 16;
	byte d4 = data >> 24;

	flash.writeByte(addr, d1);
	flash.writeByte(addr+1, d2);
	flash.writeByte(addr+2, d3);
	flash.writeByte(addr+3, d4);
}

int readInteger(uint32_t addr){
	byte d1 = flash.readByte(addr);
	byte d2 = flash.readByte(addr+1);
	byte d3 = flash.readByte(addr+2);
	byte d4 = flash.readByte(addr+3);

	return d1 | d2 << 8 | d3 << 16  | d4 << 24;
}
