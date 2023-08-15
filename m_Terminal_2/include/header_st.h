#pragma once

#define StageCount 11
#define PasswordAmount 2
#define MaxPassLen 10
// may aswell move this into the Oled lib?
#define headLineMaxSize 16

#define relayAmount 2

#define open        0
#define closed      1

#define doorOpen    0
#define doorClosed  1

#define ledCnt 13

// how long the system remains in unlock and accepts RFID cards 
unsigned long deconRFIDTimeout = 12000;
// how long the signal of RFID identity remains active 
unsigned long rfidTxDuration = 5000;
unsigned long displayFailedUnlock = 8000;
unsigned long rfidTimeout = 3000;
unsigned long lightStartDuration = 20000;

// FL lamps shall be some industrial reddish light 
//static constexpr int clrLight[3] = {255,200,120};
static constexpr int clrLight[3] = {255,0,0};

enum brains {
    airlockAccess,      // access module on the outside
    labAccess,          // access module on the inside of the lab
    ledLaserBrain,
    ledCeilBrain
};


// 3 IO therefore 1+2+4 = 7 values 
// 4 IO = 15 Values
enum IOpins {
    IO_1,
    IO_2,
    IO_3,
    IO_4,
    reedDoor, 
    IO_6,
    IO_7,                       
    IO_8,                                            
};

// 15 values 4 IOs
enum inputValues {  
    roomBoot = 1,            
    elancellEnd,            
    rachelEnd,            
    lightOff,            
    cleanupLight,            
    failedBootTrigger,        
    bootupTrigger,    
    rachelAnnouncement,    
    skipToSeperation
};

// 7 Values 3 IOs
enum outputValues {
    david = 1,
    rachel,
    seperationEnd,
    davidSeperated,     // status for T1
    rachelSeperated,     // status for T1
    timeout,            // when card was not presented in time
};



#define outputCnt 3
#define inputCnt 5


int intputArray[inputCnt] = {
    IO_1,
    IO_2,
    IO_3,
    IO_4,
    reedDoor
};

int outputArray[outputCnt] = {
    IO_6,
    IO_7,                       
    IO_8,                       
};


// -- relays
enum relays {
    door,
    outerDoor
};

enum relayInits {
    doorInit = doorClosed,
    outerDoorInit = doorClosed,
};

int relayPinArray[relayAmount] = {
    door, 
    outerDoor,
};

int relayInitArray[relayAmount] = {
    doorInit,
    outerDoorInit
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
    "SD",   // David
};



char stageTexts[StageCount][headLineMaxSize] = {
    "Present to Open" ,                     // setupStage
    "Present to Open" ,                     // idle 
};