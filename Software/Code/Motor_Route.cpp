#include "Motor_Route.h"


////Main Route Drive//////////////////////////////////////////////////////////////

int depth = 0;

bool driveRoute(int distanceLeft, int distanceRight, int speed){
	for (int i = 0; i < depth; i ++) Serial.print("  ");
	Serial.print("Start route ");
	Serial.print(distanceLeft);
	Serial.print(", ");
	Serial.println(distanceRight);

	depth ++;

	//Start Driving
	startRoute(distanceLeft,distanceRight,speed);

	//Check for collision while driving
	while(!routeFinished() && !routeStop)
	{
		serialEvent();

		bool forward = distanceLeft > 0 && distanceRight > 0;

		//Check ultrasonic sensors
		bool leftCollision   = getUltrasonicDistance(1) < minDist && forward;
		bool middleCollision = getUltrasonicDistance(2) < minDist && forward;
		bool rightCollision  = getUltrasonicDistance(3) < minDist && forward;

		//Check for collision
		if (leftCollision || middleCollision || rightCollision || routeError())
		{
			for (int i = 0; i < depth; i ++) Serial.print("  ");
			Serial.print("Collision ");
			Serial.print(leftCollision);
			Serial.print(" | ");
			Serial.print(middleCollision);
			Serial.print(" | ");
			Serial.print(rightCollision);
			Serial.print(" | ");
			Serial.print(routeError());

			if (forward){  //surrounding only works when driving forward
				if (!surround(distanceLeft, distanceRight, speed)){
					startRoute(0,0,0, false);
					Serial.println("Error!");
					return false;
				}
			}
			else{
				startRoute(0,0,0, false);
				Serial.print("Error!");
				return false;
			}
		}
	}

	depth --;

	for (int i = 0; i < depth; i ++) Serial.print("  ");
	Serial.println("Finished route");
	return !routeStop;
}

bool surround(int distanceLeft, int distanceRight, int speed){
	//Save left over distance for both motors
	int leftOverDistanceLeft = abs(distanceLeft)-getDrivenLengthLeft();
	if (distanceLeft < 0) {leftOverDistanceLeft *= -1;}
	int leftOverDistanceRight = abs(distanceRight)-getDrivenLengthRight();
	if (distanceRight < 0) {leftOverDistanceRight *= -1;}

	//Drive Back
	if (!driveBack(backDistance,backDistance,surroundObjectSpeed,&leftOverDistanceLeft,&leftOverDistanceRight)) return false;

	//Turn to the side with more space
	bool driveRight = getUltrasonicDistance(0) < getUltrasonicDistance(4);
	if (!driveTurn(driveRight)) return false;

	int horizontalOffsetLeft = 0;
	int horizontalOffsetRight = 0;
	int maxLength = leftOverDistanceLeft > leftOverDistanceRight ? leftOverDistanceLeft : leftOverDistanceRight;

	//Drive until there is no object on the side
	if (!driveAround(sideCheckMaxDistance,sideCheckMaxDistance,surroundObjectSpeed,&horizontalOffsetLeft,&horizontalOffsetRight, !driveRight, maxLength/10, sideCheckDistance_1)){
		//drive back horizontal offset
		if (!driveRouteWithoutSurrounding(horizontalOffsetLeft*(-1), horizontalOffsetRight*(-1), surroundObjectSpeed, false)) return false;

		//Turn to the other side
		if (!driveTurn(!driveRight) || !driveTurn(!driveRight)) return false;

		driveRight = not driveRight;

		if (!driveAround(sideCheckMaxDistance,sideCheckMaxDistance,surroundObjectSpeed,&horizontalOffsetLeft,&horizontalOffsetRight, !driveRight, maxLength/10, sideCheckDistance_1)){
			Serial.println("Could not find way to surround!");
			return false;
		}
	}

	//Turn back in drive direction
	if (!driveTurn(!driveRight)) return false;

	//Drive left over route
	//if (!driveRoute(leftOverDistanceLeft,leftOverDistanceRight,speed)) return false;
	int drivenL = 0;
	int drivenR = 0;
	if (!driveAround(leftOverDistanceLeft,leftOverDistanceRight,speed,&drivenL,&drivenR, !driveRight, horizontalOffsetLeft/10, sideCheckDistance_2, true)) return false;
	distanceLeft  = leftOverDistanceLeft - drivenL;
	distanceRight = leftOverDistanceRight - drivenR;

	//Turn in other direction to fix horizontal offset
	if (!driveTurn(!driveRight)) return false;
	if (!driveRoute(horizontalOffsetLeft,horizontalOffsetRight,speed)) return false;

	//Drive streight
	if (!driveTurn(driveRight)) return false;

	startRoute(distanceLeft,distanceRight,speed);

	return !routeStop;
}

////Drive functions//////////////////////////////////////////////////////////////

bool driveRouteWithoutSurrounding(int distanceLeft, int distanceRight, int speed, bool checkDistance){
	startRoute(distanceLeft,distanceRight,speed);

	bool success = true;
	while(!routeFinished() && success && !routeStop)
	{
		serialEvent();

		bool leftCollision   = getUltrasonicDistance(1) < minDist && checkDistance;
		bool middleCollision = getUltrasonicDistance(2) < minDist && checkDistance;
		bool rightCollision  = getUltrasonicDistance(3) < minDist && checkDistance;

		if (leftCollision || middleCollision || rightCollision || routeError()) success = false;
		delay(100);
	}

	return success && !routeStop;
}

bool driveTurn(bool driveRight){
	if (driveRight){
		return driveRouteWithoutSurrounding(rightAngleTurnDistance, rightAngleTurnDistance*(-1), surroundObjectSpeed, false);
	}
	else{
		return driveRouteWithoutSurrounding(rightAngleTurnDistance*(-1), rightAngleTurnDistance, surroundObjectSpeed, false);
	}
}

bool driveBack(int distanceLeft, int distanceRight, int speed, int *leftOverDistanceLeft, int *leftOverDistanceRight){
	bool success = true;

	if (leftOverDistanceLeft > 0){distanceLeft *= -1;}
	if (leftOverDistanceRight > 0){distanceRight *= -1;}

	//Start Driving
	success = driveRouteWithoutSurrounding(distanceLeft,distanceRight,speed, false);

	int drivenLeft  = getDrivenLengthLeft();
	int drivenRight = getDrivenLengthRight();

	if (distanceLeft < 0)  *leftOverDistanceLeft   += drivenLeft;
	else 				   *leftOverDistanceLeft   -= drivenLeft;
	if (distanceRight < 0) *leftOverDistanceRight  += drivenRight;
	else 				   *leftOverDistanceRight  -= drivenRight;

	return success;
}

bool driveAround(int maxDistanceLeft, int maxDistanceRight, int speed, int *offsetDistanceLeft, int *offsetDistanceRight, bool checkRight, int checkDistance, int sideCheckDistance, bool waitForObject){
	bool success = true;
	bool stop = false;

	//Start Driving
	startRoute(maxDistanceLeft,maxDistanceRight,speed);

	bool sideEnoughtSpace = false;
	int  sideStartDrivenLeft = 0;

	bool checkSideEna = !waitForObject;

	while(!routeFinished() && success && !stop && !routeStop)
	{
		serialEvent();

		//Problem with motor
		if (routeError()) success = false;
		//Collosion
		if (maxDistanceLeft > 0 && maxDistanceRight > 0){
			bool leftCollision   = getUltrasonicDistance(1) < minDist;
			bool middleCollision = getUltrasonicDistance(2) < minDist;
			bool rightCollision  = getUltrasonicDistance(3) < minDist;
			if (leftCollision || middleCollision || rightCollision) success = false;
		}
		//Enought space + driven sideCheckDistance
		int sideDistance;
		if (checkRight) sideDistance = getUltrasonicDistance(4);
		else sideDistance = getUltrasonicDistance(0);

		int drivenLeft  = getDrivenLengthLeft();
		if (sideDistance > checkDistance || sideDistance > maxSideDistance){
			if (checkSideEna){
				if (!sideEnoughtSpace){
					sideStartDrivenLeft  = drivenLeft;
					sideEnoughtSpace     = true;
				}

				if (drivenLeft-sideStartDrivenLeft > sideCheckDistance) {
					stop = true;
				}
			}
		}
		else checkSideEna = true;

		delay(100);
	}

	int drivenLeft  = getDrivenLengthLeft();
	int drivenRight = getDrivenLengthRight();
	*offsetDistanceLeft  += drivenLeft;
	*offsetDistanceRight += drivenRight;

	return stop && !routeStop;
}

////General Functions//////////////////////////////////////////////////////////////

void startRoute(int distanceLeft, int distanceRight, int speed, bool waitFinished){
	digitalWrite(0, 1);  //Reset driven length
	writeInteger(3, 15, distanceLeft);
	digitalWrite(1, 1);
	delayMicroseconds(1);
	digitalWrite(1, 0);  //Set left distance
	writeInteger(3, 15, distanceRight);
	digitalWrite(2, 1);
	delayMicroseconds(1);
	digitalWrite(2, 0);  //Set right distance
	writeNatural(18, 8, speed);
	digitalWrite(0, 0);  //Start driving
	while (waitFinished && routeFinished());
}

bool routeFinished(){
	return digitalRead(0);
}

bool routeError(){
	return digitalRead(1);
}

int getDrivenLengthRight(){
	digitalWrite(26, 1);
	return readNatural(2, 14);
}

int getDrivenLengthLeft(){
	digitalWrite(26, 0);
	return readNatural(2, 14);
}

int getUltrasonicDistance(int sensor){
	writeNatural(27, 3, sensor); //Select sensor (0-4)
	return readNatural(16, 10);
}
