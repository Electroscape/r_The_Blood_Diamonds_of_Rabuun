#pragma once

#define StageCount 1
#define PasswordAmount 2
#define MaxPassLen 10
// may aswell move this into the Oled lib?
#define headLineMaxSize 16
#define thresold 190 
// depending on the volt coming from the D+ signal in the USB

#define relayAmount 3

#define open        0
#define closed      1


enum brains {
    accessBrain,      
};


enum IOpins {
    USB_INPUT_PIN,                                         
};


enum inputValues {  
    usbStick = 1
};


enum outputValues {};

#define outputCnt 0
#define inputCnt 1


int intputArray[inputCnt] = {
    USB_INPUT_PIN,
};

int outputArray[outputCnt] = {};


// -- relays
enum relays {
    lid,
    gate,
    alarmlight,
    whitelight
};

enum relayInits {
    lidInit = closed,
    gateInit = closed,
    alarmlightInit = closed,
};

int relayPinArray[relayAmount] = {
    lid,
    gate,
    alarmlight,
 
};

int relayInitArray[relayAmount] = {
    lidInit,
    gateInit,
    alarmlightInit,
    
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