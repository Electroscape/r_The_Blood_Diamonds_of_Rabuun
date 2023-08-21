/**
 * @file m_ter2.cpp
 * @author Martin Pek (martin.pek@web.de)
 * @brief  
 * @version 0.1
 * @date 2023-08-16
 * 
 * @copyright Copyright (c) 2023
 * 
*/


#include <stb_mother.h>
#include <stb_mother_IO.h>
#include <stb_keypadCmds.h>
#include <stb_oledCmds.h>
#include <stb_mother_ledCmds.h>


#include "header_st.h"

// using the reset PCF for this
PCF8574 inputPCF;
STB_MOTHER Mother;
STB_MOTHER_IO MotherIO;


int stage = setupStage;
// since stages are single binary bits and we still need to d some indexing
int stageIndex = 0;
// doing this so the first time it updates the brains oled without an exta setup line
int lastStage = -1;
int inputTicks = 0;

// general timestamp going to use this to timeout the card repsentation in unlocked and RFIDoutput
unsigned long timestamp = millis();

// cardsPreset adding bits of present cards with | 
int cardsPresent = 0;
int brainsPresent = 0;
bool lightOn = false;

int validBrainResult = 0;


void enableWdt() {
    wdt_enable(WDTO_8S);
}


/**
 * @brief Set the Stage Index object
 * @todo safety considerations
*/
void setStageIndex() {
    for (int i=0; i<StageCount; i++) {
        if (stage <= 1 << i) {
            stageIndex = i;
            Serial.print("stageIndex:");
            Serial.println(stageIndex);
            delay(1000);
            return;
        }
    }
    Serial.println(F("STAGEINDEX ERRROR!"));
    wdt_reset();
    delay(16000);
}


/**
 * @brief  consider just using softwareReset
*/
void gameReset() {
    // stage = setupStage;
    for (int relayNo=0; relayNo < relayAmount; relayNo++) {
        Mother.motherRelay.digitalWrite(relayNo, relayInitArray[relayNo]);
    }
    lightOn = false;
}


/**
 * @brief  
 * @param result 
 * @param brainNo can be set to > labAccess so it sends to both brains
*/
void sendResult(bool result, int brainNo=Mother.getPolledSlave()) {
    // prepare return msg with correct or incorrect
    char msg[10] = "";
    char noString[3] = "";
    strcpy(msg, keypadCmd.c_str());
    strcat(msg, KeywordsList::delimiter.c_str());

    // can i make an integer oneliner?
    if (result) {
        sprintf(noString, "%d", KeypadCmds::correct);
    } else {
        sprintf(noString, "%d", KeypadCmds::wrong);
    }
    strcat(msg, noString);

    Mother.sendCmdToSlave(msg, brainNo);
}


/**
 * @brief handles room specific actions
 * @param passNo 
*/
void handleCorrectPassword(int passNo) {
    Mother.motherRelay.digitalWrite(lid, open);
}


bool passwordInterpreter(char* password) {
    // Mother.STB_.defaultOled.clear();
    Serial.print("Handling pw");
    Serial.println(password);
    for (int passNo=0; passNo < PasswordAmount; passNo++) {
        if (passwordMap[passNo] & stage) {
            // strlen(passwords[passNo]) == strlen(password) &&
            if ( strncmp(passwords[passNo], password, strlen(passwords[passNo]) ) == 0) {
                handleCorrectPassword(passNo);
                Serial.println("Correct PW");
                return true;
            }
        }
    }
    return false;
}


/**
 * @brief handles evalauation of codes and sends the result to the access module
 * @param cmdPtr 
*/
void handleResult(char *cmdPtr) {
    cmdPtr = strtok(NULL, KeywordsList::delimiter.c_str());

    // prepare return msg with correct or incorrect
    char msg[10] = "";
    char noString[3] = "";
    strcpy(msg, keypadCmd.c_str());
    strcat(msg, KeywordsList::delimiter.c_str());
    if (passwordInterpreter(cmdPtr) && (cmdPtr != NULL)) {
        sprintf(noString, "%d", KeypadCmds::correct);
        strcat(msg, noString);
    } else {
        sprintf(noString, "%d", KeypadCmds::wrong);
        strcat(msg, noString);
    }
  
    Mother.sendCmdToSlave(msg);
}



bool checkForRfid() {
    Serial.println(Mother.STB_.rcvdPtr);
    if (strncmp(KeywordsList::rfidKeyword.c_str(), Mother.STB_.rcvdPtr, KeywordsList::rfidKeyword.length() ) != 0) {
        return false;
    } 
    char *cmdPtr = strtok(Mother.STB_.rcvdPtr, KeywordsList::delimiter.c_str());
    handleResult(cmdPtr);
    wdt_reset();
    return true;
}

void interpreter() {
    while (Mother.nextRcvdLn()) {
        checkForRfid();
    }
}


void oledUpdate(int brainIndex = 0) {
    char msg[32] = "";
    strcpy(msg, oledHeaderCmd.c_str());
    strcat(msg, KeywordsList::delimiter.c_str());
    strcat(msg, stageTexts[stageIndex]); 
    Mother.sendCmdToSlave(msg, brainIndex);
}



void stageActions() {
    wdt_reset();
    switch (stage) {
        default: break;
    }
}


/**
 * @brief  triggers effects specific to the given stage, 
 * room specific excecutions can happen here
 * TODO: avoid reposts of setflags, but this is an optimisation
*/
void stageUpdate() {
    if (lastStage == stage) { return; }
    Serial.print("Stage is:");
    Serial.println(stage);
    setStageIndex();
        
    // check || stageIndex >= int(sizeof(stages))
    if (stageIndex < 0) {
        Serial.println(F("Stages out of index!"));
        delay(15000);
        
    }
    // important to do this before stageActions! otherwise we skip stages
    lastStage = stage;

    // for now no need to make it work properly scaling, need to build afnc repertoir anyways
    for (int brainNo=0; brainNo < Mother.getSlaveCnt(); brainNo++) {
        Mother.setFlags(brainNo, flagMapping[stageIndex]);
        oledUpdate(brainNo);
        delay(5);
    }
    MotherIO.outputReset();
    stageActions();
}



void handleInputs() {
    
    int result = MotherIO.getInputs();

    if (result & usbStick) {
        Mother.motherRelay.digitalWrite(gate, open);
    }

    wdt_reset();
}


void setup() {

    Mother.begin();
    // starts serial and default oled
    Serial.println("relay init");
    Mother.relayInit(relayPinArray, relayInitArray, relayAmount);
    Serial.println("IO init");
    MotherIO.ioInit(intputArray, sizeof(intputArray), outputArray, sizeof(outputArray));

    Mother.setFlags(0, flagMapping[stageIndex]);

    Serial.println("WDT endabled");
    enableWdt();

    Mother.rs485SetSlaveCount(1);

    setStageIndex();

    wdt_reset();
}


void loop() {
    validBrainResult = Mother.rs485PerformPoll();
    if (validBrainResult) {interpreter();}
    handleInputs();    
    stageUpdate(); 
    wdt_reset();
    delay(5);
}




