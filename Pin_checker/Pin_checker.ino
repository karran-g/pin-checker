/*
  Pin_checker.ino

  4-digit PIN entry state machine using a 4x4 membrane keypad and LCD1602A display.
  Three states: ENTERING, SUCCESS, LOCKED.
  Depends on Membrane_switch.ino and LCD.ino.

*/
// ################################################################################
// Libraries
// ################################################################################

// Uses <string.h> for memset and <ctype.h> for isdigit.
#include <string.h>
#include <ctype.h>

// ################################################################################
// PIN & State
// ################################################################################

const char defaultPin[4] = { '1', '2', '3', '4' };

enum State { ENTERING = 0,
             SUCCESS = 1,
             LOCKED = 2 };
State state = ENTERING;

// ################################################################################
// Input Tracking
// ################################################################################

byte count = 0;  // Number of digits entered so far.
char buffer[4];  // Stores the entered PIN digits.
byte tries = 3;  // Remaining attempts before lockout.

// ################################################################################
// Display State
// ################################################################################

byte digitMaskcol = 6;        // LCD column where the last entered digit sits; 6 -> offset where PIN digits start.
bool digitRevealing = false;  // True while the last entered digit is still visible.
bool freshState = true;       // True when the display needs to be redrawn for the current state.
char bufferError[16];         // Buffer for formatted error messages.

// ################################################################################
// Timestamps
// ################################################################################

unsigned long timeStamp_enter = 0;    // Time of last digit entry.
unsigned long timeStamp_success = 0;  // Time of successful PIN entry.

// ################################################################################
// Setup
// ################################################################################

void setup() {
    Serial.begin(9600);
    membraneSwitch_setup();
    lcdInit();
    lcdClear();
}

// ################################################################################
// Helpers
// ################################################################################

// Compares two 4-character arrays character by character.
bool pinCheck(const char* str1, const char* str2) {
    bool flag = true;
    for (int i = 0; i < 4; i++) {
        if (str1[i] != str2[i]) {
            flag = false;
        }
    }
    return flag;
}

// ################################################################################
// Main Loop
// ################################################################################

void loop() {
    char button = membraneSwitch_buttonscan();
    Serial.println(button);

    // Mask the last entered digit with '*' after 500ms.
    if (digitRevealing && (millis() - timeStamp_enter > 500)) {
        setCursor(1, digitMaskcol);
        printChar('*');
        digitRevealing = false;
    }

    if (state == ENTERING) {
        if (freshState == true) {
            setCursor(0, 3);
            printStr("Enter Pin:");
            freshState = false;
        }

        if (button) {
            if (count < 4 && isdigit(button)) {
                setCursor(1, count + 6);
                printChar(button);
                timeStamp_enter = millis();
                if (digitRevealing == true) {
                    setCursor(1, digitMaskcol);
                    printChar('*');  // Force-mask previous digit on fast input.
                }
                digitMaskcol = count + 6;
                digitRevealing = true;
                buffer[count] = button;
                count++;

            } else if (count == 4 && button == 'A') {
                if (pinCheck(buffer, defaultPin)) {
                    state = SUCCESS;
                    tries = 3;
                    memset(buffer, 0, sizeof(buffer));
                    lcdClear();
                } else {
                    tries--;
                    setCursor(0, 1);
                    sprintf(bufferError, "Wrong, tries:%d", tries);
                    printStr(bufferError);
                    delay(2000);
                    lcdClear();
                    if (tries >= 1) {
                        setCursor(0, 3);
                        printStr("try again.");
                        delay(2000);
                        lcdClear();
                        freshState = true;
                    } else if (tries == 0) {
                        state = LOCKED;
                        tries = 3;
                        lcdClear();
                    }
                    memset(buffer, 0, sizeof(buffer));
                    memset(bufferError, 0, sizeof(bufferError));
                }
                count = 0;

            } else if (count > 0 && button == 'B') {  // Backspace.
                count--;
                setCursor(1, count + 6);
                printChar(' ');
            }
        }

    } else if (state == SUCCESS) {
        setCursor(0, 2);
        printStr("Pin correct!");
        setCursor(1, 0);
        printStr("Will reset in 5s");
        delay(3000);
        lcdClear();
        timeStamp_success = millis();
        while (millis() - timeStamp_success < 2000) {
            setCursor(0, 3);
            printStr("Resetting.");
        }
        lcdClear();
        state = ENTERING;
        freshState = true;

    } else if (state == LOCKED) {
        setCursor(0, 5);
        printStr("LOCKED");
        setCursor(1, 0);
        printStr("Press C to reset");
        if (button == 'C') { 
            lcdClear(); 
            setCursor(0, 3);
            printStr("Resetting.");
            delay(1500);
            lcdClear();
            state = ENTERING;
            freshState = true;
        }
    }
}