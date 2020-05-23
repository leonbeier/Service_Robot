#include <Arduino.h>
#include <String.h>
#include "IO_Communication.h"

static int rightAngleTurnDistance = 475;  //mm to drive for a 90° turn
static int sideCheckMaxDistance = 10000;  //maximum distance to drive while checking if the object can be surrounded
static int sideCheckDistance_1  = 200;    //Overshoot while driving to the side
static int sideCheckDistance_2  = 500;    //Overshoot while driving to the side
static int surroundObjectSpeed = 255;     //speed for surrounding
static int backDistance = 50;
static int minDist = 10;                   //distance in front of ultrasonic sensors for collision (in cm)

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
