#include <Arduino.h>
#include <String.h>
#include "IO_Communication.h"

extern int rightAngleTurnDistance;  //mm to drive for a 90° turn
extern int sideCheckMaxDistance;    //maximum distance to drive while checking if the object can be surrounded
extern int sideCheckDistance_1;     //Overshoot while driving to the side
extern int sideCheckDistance_2;     //Overshoot while driving forward
extern int maxSideDistance;         //Distance from Ultrasonic sensor after that seen as clear
extern int surroundObjectSpeed;     //speed for surrounding
extern int backDistance;            //Distance to drive back after collision
extern int minDist;                 //distance in front of ultrasonic sensors for collision (in cm)

extern bool routeStop;

int getUltrasonicDistance(int sensor);
int getDrivenLengthLeft();
int getDrivenLengthRight();
bool routeError();
bool routeFinished();
void startRoute(int distanceLeft, int distanceRight, int speed, bool waitFinished = true);
bool driveRoute(int distanceLeft, int distanceRight, int speed);
bool driveBack(int distanceLeft, int distanceRight, int speed, int *leftOverDistanceLeft, int *leftOverDistanceRight);
bool driveAround(int maxDistanceLeft, int maxDistanceRight, int speed, int *offsetDistanceLeft, int *offsetDistanceRight, bool checkRight, int checkDistance, int sideCheckDistance, bool waitForObject = false);
bool driveTurn(bool driveRight);
bool driveRouteWithoutSurrounding(int distanceLeft, int distanceRight, int speed, bool checkDistance);
bool surround(int distanceLeft, int distanceRight, int speed);
