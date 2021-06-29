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


/*
//Sets the output pins to a number
void setInt (int val, int length, int start)
{
    for (int i = 0; i < length; i ++){
        digitalWrite(i+start, val%2);
        val /= 2;
    }
}

//Reads a number from the input pins
int readInt (int length, int start)
{
    uint32_t val = IORD_ALTERA_AVALON_PIO_DATA(PIO_1_BASE);
    return (val >> (start)) & (int)pow(2.0, (double)length);
    
    int result = 0;
    for (int j = length+start-1; j >= start; j --){
        result += digitalRead(j);
        result *= 2;
    }
    return result;
}
*/
