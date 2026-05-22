/*
  Membrane_switch.ino

  4x4 membrane keypad driver for the Arduino Uno.
  Handles row/column scanning, edge detection, and debounce.
  No external libraries - built directly on top of Arduino's digitalWrite/digitalRead.

  Rows: pins - 7, 6, 5, 4  (OUTPUT)
  Cols: pins - 3, 2, A4, A5 (INPUT_PULLUP)
*/

// ###########################################################################
// Pin definitions
// ###########################################################################

// Row and column arrays containing the corresponding Arduino pins.

const int membraneRows[4] = { 7, 6, 5, 4 };
const int membraneCols[4] = { 3, 2, A4, A5 };

// ###########################################################################
// Key Map
// ###########################################################################

// 2D lookup table mapping each [row][column] index to its character.
// Matches the actual physical layout of the 4x4 membrane switch keyapd.

const char keys[4][4] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
};

// ###########################################################################
// Key State Tracking
// ###########################################################################

// keysCurrentState[4][4] -> each key's state in the current scan (if it is/was being pressed).
// keysLastState[4][4]    -> each key's state from the previous scan.
// Both correspond index for index to the keys[4][4].

bool keysCurrentState[4][4] = { false };
bool keysLastState[4][4] = { false };

// ###########################################################################
// Debounce
// ###########################################################################

// lastPresstime[4][4] — timestamp (in ms) of the last confirmed press for each key.
//                       Corresponds index for index to the keys[4][4].
// DEBOUNCE_TIME       — minimum time (in ms) that must elapse between a confirmed press
//                       and the next valid detection for the same key.
//                       Filters out mechanical contact bounce.

unsigned long lastPresstime[4][4] = { 0 };
const unsigned long DEBOUNCE_TIME = 50;

// ###########################################################################
// Setup
// ###########################################################################

void membraneSwitch_setup() {
    for (int i : membraneRows) {
        pinMode(i, OUTPUT);     // Initialises all row pins as OUTPUT and drives them HIGH (idle state).
        digitalWrite(i, HIGH);  // Initialises all column pins as INPUT_PULLUP.
    }
    for (int j : membraneCols) {
        pinMode(j, INPUT_PULLUP);
    }
}

// ###########################################################################
// Scan
// ###########################################################################

// Performs one full scan cycle across all rows and columns.
// Returns the character of the key pressed, or '\0' if nothing was detected.
//
// Scan logic:
//   Each row is briefly driven LOW one at a time. If a key is pressed, it shorts
//   that row to its column, setting the INPUT_PULLUP column LOW.
//   A valid press is confirmed only on a rising edge and after DEBOUNCE_TIME ms has passed since the last confirmed press.

//   A flag-based exit is used so the active row pin is always driven back to HIGH before the function exits.

char membraneSwitch_buttonscan() {

    bool edge;                   // Rising edge flag -> true if this is a new press, not a hold.
    char keyTemp;                // Stores the character of the detected key.
    int r = 0;                   // Current row index.
    bool buttonPressed = false;  // Exits both loops once a valid press is confirmed.

    while (r < 4 && !buttonPressed) {
        digitalWrite(membraneRows[r], LOW);  // Drive row LOW to enable column detection.
        int c = 0;

        while (c < 4 && !buttonPressed) {

            // A pressed key short ciruits the LOW row to its column, setting the INPUT_PULLUP column LOW.
            if (digitalRead(membraneCols[c]) == 0) {

                keysCurrentState[r][c] = 1;  // Mark key as pressed. Redundant but kept for clarity with the definition of a rising edge below.

                // Rising edge: key is pressed now, was not pressed last cycle
                // AND
                // Enough time has passed to rule out contact bounce.
                edge = keysCurrentState[r][c] && !keysLastState[r][c] && (millis() - lastPresstime[r][c]) > DEBOUNCE_TIME;

                if (edge) {
                    lastPresstime[r][c] = millis();  // Record timestamp of this confirmed press.
                    keyTemp = keys[r][c];            // Store the corresponding character.
                    buttonPressed = true;            // Signal early exit.
                }
            }

            // Update state arrays for this cell regardless of whether a press was detected, keeps state-machine consistent.
            keysLastState[r][c] = keysCurrentState[r][c];
            keysCurrentState[r][c] = 0;  // Reset for the next cycle.
            c++;
        }

        digitalWrite(membraneRows[r], HIGH);  // Restore row to HIGH before moving on.
        r++;
    }

    if (buttonPressed) {
        return keyTemp;// Return the character of the pressed key.
    }
    return '\0';  // No valid press detected this cycle.
}
