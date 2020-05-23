#include "IO_Communication.h"

int readNatural(int startPin, int length){
	int num = 0;
	for (int i = startPin+length-1; i >= startPin; i --){
		num *= 2;
		num += digitalRead(i);
	}
	return num;
}

int readInteger(int startPin, int length){
	int num = readNatural(startPin, length);
	if (num >= 1<<(length-1))
		num -= (1<<length);
	return num;
}

void writeNatural(int startPin, int length, int value){
	for (int i = startPin; i < startPin+length; i ++){
		digitalWrite(i, value%2);
		value /= 2;
	}
}

void writeInteger(int startPin, int length, int value){
	if (value < 0)
		value += (1<<length);
	writeNatural(startPin, length, value);
}
