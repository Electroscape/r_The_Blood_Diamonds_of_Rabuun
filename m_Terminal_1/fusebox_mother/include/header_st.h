#pragma once

#define StageCount 3
#define PasswordAmount 1
#define MaxPassLen 10
#define relayAmount 2
// may aswell move this into the Oled lib?
#define headLineMaxSize 20

#define open        0
#define closed      1

#define inputCnt 8

unsigned long resetTime = 5000;

enum relays {
    magnet,
    emergencyLight
};

enum relayInits {
    magnet_init = closed,
    emergencyLight_init = closed
};

enum IOpins {
    IO_1,
    IO_2,
    IO_3,
    logic0,
    logic1, 
    IO_6,
    IO_7,                       
    IO_8,                                            
};

int relayPinArray[relayAmount] = {
    magnet,
    emergencyLight
};

int relayInitArray[relayAmount] = {
    magnet_init,
    emergencyLight_init
};


enum stages {
    locked = 1,
    unlocked = 2,
    endgame = 3,
};

// the sum of all stages sprinkled with a bit of black magic
int stageSum = ~( ~0 << StageCount );


// could have multiple brains listed here making up a matrix
// for now its only an Access module mapped here
int flagMapping[StageCount] {
    keypadFlag
};

char passwords[PasswordAmount][MaxPassLen] = {
    "2429"
};

// defines what password/RFIDCode is used at what stage, if none is used its -1
int passwordMap[PasswordAmount] = {
    locked
};
// make a mapping of what password goes to what stage


char stageTexts[StageCount][headLineMaxSize] = {
    "Welcome",
};
