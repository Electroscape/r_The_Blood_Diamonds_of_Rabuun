#pragma once

#define StageCount 1
#define PasswordAmount 2
#define MaxPassLen 10
// may aswell move this into the Oled lib?
#define headLineMaxSize 16

#define relayAmount 3

#define open        0
#define closed      1


enum brains {
    accessBrain,      
};


enum IOpins {
    IO_1,                                         
};


enum inputValues {  
    usbStick = 1
};


enum outputValues {};

#define outputCnt 0
#define inputCnt 1


int intputArray[inputCnt] = {
    IO_1,
};

int outputArray[outputCnt] = {};


// -- relays
enum relays {
    lid,
    gate,
    light
};

enum relayInits {
    lidInit = closed,
    gateInit = closed,
    lightInit = closed
};

int relayPinArray[relayAmount] = {
    lid,
    gate,
    light
};

int relayInitArray[relayAmount] = {
    lidInit,
    gateInit,
    lightInit,
};


enum stages {
    setupStage = 1, 
};

// the sum of all stages sprinkled with a bit of black magic
int stageSum = ~( ~0 << StageCount );


// could have multiple brains listed here making up a matrix
// for now its only an Access module mapped here
int flagMapping[StageCount] {
    rfidFlag,          // setupStage
};

char passwords[PasswordAmount][MaxPassLen] = {
    "SD",
};

int passwordMap[PasswordAmount] = {
    setupStage
};


char stageTexts[StageCount][headLineMaxSize] = {
    "Present to Open",                     // setupStage
};