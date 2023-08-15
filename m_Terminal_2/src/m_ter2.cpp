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
// timedTrigger shall reset this value 
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
 * @brief handles timeouts rfidTX and timeout of the RFID presentation
*/
void timedTrigger() {
    if (timestamp > millis() || lastStage != stage) { return; }
    switch (stage) {
        case unlock: 
            stage = failedUnlock;
        break;
        // alternatively use cardspresent to send evaluation
        default: 
            if ((stage & (seperationLocked + seperationUnlocked)) > 0) {
                if (cardsPresent == 0) {return;}
                resetDualRfid();
                sendResult(false, 0);
                sendResult(false, 1);
            }
        break;
    }
}


/**
 * @brief  handles RFId scanning required on both sides
 * @param passNo 
*/
void checkDualRfid(int passNo) {
     
    timestamp = millis() + rfidTimeout;
    if (stage == seperationUnlocked && inputPCF.digitalRead(reedDoor)) {
        resetDualRfid();
        Serial.println("door is not closed");
        return;
    }
    cardsPresent |= passNo + 1;
    brainsPresent |= Mother.getPolledSlave() + 1;

    // not presented on both cards, hence we exit here
    if (cardsPresent <= PasswordAmount) { return; }
    // not both rfid readers used so its invalid
    if (brainsPresent <= int(labAccess) + 1) {
        Serial.println("not enoough brains present");
        return;
    }

    cardsPresent = brainsPresent = 0;

    switch (stage) {
        case seperationUnlocked: 
            Mother.motherRelay.digitalWrite(door, doorClosed); 
            delay(100);
            if (Mother.getPolledSlave() == 0) {
                switch (passNo) {
                    // either this case?
                    case 0: MotherIO.setOuput(davidSeperated, true); break;
                    case 1: MotherIO.setOuput(rachelSeperated, true); break;
                }
            } else {
                switch (passNo) {
                    case 0: MotherIO.setOuput(rachelSeperated, true); break;
                    case 1: MotherIO.setOuput(davidSeperated, true); break;
                }
            }
            stage = seperationLocked; 
        break;
        case seperationLocked:
            Mother.motherRelay.digitalWrite(door, doorOpen); // first thing we do since we dont wanna
            delay(100);
            MotherIO.setOuput(seperationEnd, true);
            stage = seperationUnlocked;
        break;
    }

    sendResult(true, 0);
    sendResult(true, 1);

    wdt_reset();
    delay(500);
    MotherIO.outputReset();
    delay(4500);
    wdt_reset();
}


/**
 * @brief handles room specific actions
 * @param passNo 
*/
void handleCorrectPassword(int passNo) {

    switch (stage) {}
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
 * @todo handle the unlock/locking here with 2 access modules
*/
void handleResult() {
    Mother.STB_.rcvdPtr = strtok(Mother.STB_.rcvdPtr, KeywordsList::delimiter.c_str());
    if ((Mother.STB_.rcvdPtr != NULL) && passwordInterpreter(Mother.STB_.rcvdPtr)) {
        // excluding the cases where both cards need to be present
        // here may be the ussie... keep the sendresult stuff in one place
        if ((stage & (seperationUnlocked + seperationLocked)) == 0) {
            sendResult(true);
        }
    } else {
        sendResult(false);
    }
}


// again good candidate for a mother specific lib
bool checkForRfid() {
    if (strncmp(KeywordsList::rfidKeyword.c_str(), Mother.STB_.rcvdPtr, KeywordsList::rfidKeyword.length() ) != 0) {
        return false;
    } 
    Mother.STB_.rcvdPtr += KeywordsList::rfidKeyword.length();
    handleResult();
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


void oledFailed() {
    timestamp = millis() + displayFailedUnlock;
    char timeoutMsg[32] = "";
    strcpy(timeoutMsg, oledHeaderCmd.c_str());
    strcat(timeoutMsg, KeywordsList::delimiter.c_str());
    strcat(timeoutMsg, stageTexts[stageIndex]);
    char cleanMsg[32] = "";
    strcpy(cleanMsg, oledHeaderCmd.c_str());
    strcat(cleanMsg, KeywordsList::delimiter.c_str());
    strcat(cleanMsg,  "Clean Airlock"); 
    while (timestamp > millis()) {
        wdt_reset();
        Mother.sendCmdToSlave(timeoutMsg);
        delay(1500);
        Mother.sendCmdToSlave(cleanMsg);
        delay(1500);
    }  
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
    
    if ( (stage & (idle | seperationUnlocked | seperationLocked)) == 0) { return; }

    int result = MotherIO.getInputs();
    result -= result & (1 << reedDoor);
    if (result == 0) { return; }
    // Serial.println("result");
    // Serial.println(result);
    // delay(5000);
    wdt_reset();
    switch (result) {
        default: break;
    }
}


void setup() {

    Mother.begin();
    // starts serial and default oled
    Mother.relayInit(relayPinArray, relayInitArray, relayAmount);
    MotherIO.ioInit(intputArray, sizeof(intputArray), outputArray, sizeof(outputArray));

    Serial.println("WDT endabled");
    enableWdt();

    Mother.rs485SetSlaveCount(1);

    setStageIndex();

    wdt_reset();
}


void loop() {
    validBrainResult = Mother.rs485PerformPoll();
    timedTrigger();
    if (validBrainResult) {interpreter();}
    handleInputs();    
    stageUpdate(); 
    wdt_reset();
}




