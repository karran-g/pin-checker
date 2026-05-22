/*
  LCD.ino

  HD44780-based LCD1602A driver for the Arduino Uno.
  Handles 4-bit initialisation, cursor positioning, and character/string output.
  No external libraries - built directly on top of Arduino's digitalWrite.

  Pins: { RS=13, E=12, DB4=11, DB5=10, DB6=9, DB7=8 }
  RW tied to GND.

  All initialisation sequences and timing details are from the LCD-1602A datasheet by Sunfounder that I found online.
*/

// ################################################################################
// Pin Definition
// ################################################################################

// LCD pin array — { RS, E, DB4, DB5, DB6, DB7 }
const int lcdPins[6] = { 13, 12, 11, 10, 9, 8 };

// ################################################################################
// Bit Operations
// ################################################################################

// To check if that specific bit digit is 1 or 0 using bit mask.
uint8_t bitMaskCheck(uint8_t nibble, uint8_t mask) {
    return ((nibble & mask) && mask);  // Redundant but stays for clarity.
}

// ################################################################################
// Data Transmission
// ################################################################################

// To send a whole nibble, done by bit masking every single bit digit and writing to the respective bit pin.
void sendNibble(uint8_t nibble) {
    digitalWrite(lcdPins[2], bitMaskCheck(nibble, 0b0001));
    digitalWrite(lcdPins[3], bitMaskCheck(nibble, 0b0010));
    digitalWrite(lcdPins[4], bitMaskCheck(nibble, 0b0100));
    digitalWrite(lcdPins[5], bitMaskCheck(nibble, 0b1000));
    //EN pulse explained in data sheet.
    digitalWrite(lcdPins[1], HIGH);  // EN rising edge - latch data.
    delayMicroseconds(1);
    digitalWrite(lcdPins[1], LOW);  // EN falling edge - HD44780 reads on the falling edge.
}

// To send a whole byte in 4-bit mode by breaking the byte down into higher and lower nibbles.
void sendByte(uint8_t data, bool rsRegister) {
    digitalWrite(lcdPins[0], rsRegister);
    uint8_t lowNibble = data & 0b00001111;          // Bit mask to only get the lower nibble.
    uint8_t highNibble = (data >> 4) & 0b00001111;  // Bit shift right by 4, then same lower mask to get a 4-bit nibble.

    sendNibble(highNibble);
    delayMicroseconds(40);
    sendNibble(lowNibble);
    delayMicroseconds(40);
}

// ################################################################################
// Initialisation
// ################################################################################

// Initialise the LCD:
//  -> Perform the startup routine and set the LCD to 4-bit mode.
//  -> Follows the  HD44780 initialisation sequence: three 8-bit reset nibbles
//       - must precede the 4-bit mode switch; this is a controller reset protocol, it is needed.
//  -> Busy flag cannot be checked during this, so fixed delays are used throughout.
void lcdInit() {
    for (int i : lcdPins) {
        pinMode(i, OUTPUT);
    }
    digitalWrite(lcdPins[0], LOW);  // Set RS to 0.

    // ################################################################################
    
    // Startup routine, inbuilt.
    delay(40);  // Wait-time after start-up.
    sendNibble(0b0011);
    delay(5);  // Wait-time, bf(Busy flag) cannot be checked.
    sendNibble(0b0011);
    delayMicroseconds(120);  // Wait-time, bf cannot be checked.
    sendNibble(0b0011);
    delayMicroseconds(40);

    // ################################################################################
    
    // Set to 4-bit mode.
    sendNibble(0b0010);  // 0 0 1 0
    delayMicroseconds(40);

    // ################################################################################
   
    // Set function: 4-bit mode with number of lines = 2 and font = 5×8.
    sendByte(0b00101000, 0);  // 0 0 1 0 N F X X

    // ################################################################################
    
    // Display off.
    sendByte(0b00001000, 0);  // 0 0 0 0 1 0 0 0

    // ################################################################################
    
    // Display clear.
    sendByte(0b00000001, 0);  // 0 0 0 0 0 0 0 1
    delay(2);

    // ################################################################################
    
    // Set entry mode with cursor direction = right and shift disabled.
    sendByte(0b00000110, 0);  // 0 0 0 0 0 1 I/D S

    // ################################################################################
    
    // Display on - cursor off, blink off.
    sendByte(0b00001100, 0);  // 0 0 0 0 1 D C B
}

// ################################################################################
// Cursor
// ################################################################################

// To set the cursor to a specific memory address.
// Valid rows: 0 (DDRAM base 0x00) and 1 (DDRAM base 0x40).
void setCursor(uint8_t row, uint8_t col) {
    uint8_t address;
    switch (row) {
        case 0:
            address = 0x00 + col;               // Add the col offset to the memory for row 0, col 0.
            sendByte(0b10000000 | address, 0);  // Send the byte for the inbuilt set cursor command + the relevant memory address.
            break;
        case 1:
            address = 0x40 + col;               // Add the col offset to the memory for row 1, col 0.
            sendByte(0b10000000 | address, 0);  // Send the byte for the inbuilt set cursor command + the relevant memory address.
            break;
        default:
            return;  // Invalid row - do nothing.
    }
    delayMicroseconds(40);
}

// ################################################################################
// Output
// ################################################################################

// To convert and send the data for a given string as bytes using the walking pointer method.
void printStr(const char* str) {
    while (*str) {
        sendByte(*str, 1);  // Sending the relevant byte with RS register = 1 to write data to display.
        str++;
    }
}

// Same as printStr() for a single character.
void printChar(char character) {
    sendByte(character, 1);  // Sending the relevant byte with RS register = 1 to write data to display.
}

// ################################################################################
// LCD Clear - for utility
// ################################################################################

// Clears the display and resets the cursor to home position.
void lcdClear() {
    sendByte(0b00000001, 0);
    delay(2);  // HD44780 requires ~ 1.5ms to execute the clear command (datasheet).
}