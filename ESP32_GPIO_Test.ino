/*
================================================================================================
=================================== ESP32 GPIO Toggle Test Sketch ==============================
================================================================================================

This sketch is designed to help you manually test the functionality of individual GPIO pins on your ESP32.
It allows you to toggle a selected GPIO pin between HIGH (~3.3V DC) and LOW (~0V DC) states
via the serial monitor, confirming its output capability and helping to diagnose wiring issues.
The code will automatically set the previous pin back to Low on entry of a new Pin

------------------------------------------------------------------------------------------------
WARNING / DISCLAIMER:
Use this code at your own risk. While designed for safe testing, incorrect usage or testing
of critical pins (like GPIO0/GPIO2) can affect ESP32 boot behavior or, in rare cases,
damage hardware if short circuits are present or inappropriate loads are connected.
Always ensure you understand your wiring and component limits.
If you break hardware / yourself, the author is not liable.
------------------------------------------------------------------------------------------------

HOW TO USE:

1.  FLASH THIS SKETCH: Upload this code to your ESP32 board using the Arduino IDE.
2.  OPEN SERIAL MONITOR: Open the Serial Monitor in the Arduino IDE (set baud rate to 115200).
3.  POWER ON ESP32: Ensure your ESP32 and any connected components are powered on.
4.  USE MULTIMETER:
    *   Set your multimeter to DC VOLTAGE mode (range appropriate for 0-5V, e.g., 20V DC).
    *   Place the BLACK (COM) probe of your multimeter on a known good GND pin of your ESP32.
    *   Use the RED (VΩmA) probe to touch the GPIO pin you want to test.
5.  ENTER PIN NUMBERS:
    *   In the Serial Monitor, you will be prompted to "Enter a pin number (Omit the 'GPIO' prefix):".
    *   Type the number of the GPIO pin you want to test (e.g., `25`, `33`) and press Enter.
    *   The sketch will toggle the state of that pin.
    *   **Observe your multimeter:**
        *   If the pin is set to HIGH, your multimeter should read approximately **3.3V DC**.
        *   If the pin is set to LOW, your multimeter should read approximately **0V DC**.
    *   **Special Pins:** For GPIO0 and GPIO2, a confirmation will be requested due to their boot-related functions.
6.  AUTOMATIC RESET: When you select a new valid pin, the *previously controlled pin* will automatically be set back to LOW to avoid conflicts and simplify testing.

================================================================================================
*/

#include <Arduino.h>
#include <map> // Required for std::map to store pin states

// A map to store the current state (LOW/HIGH) of each GPIO pin we interact with.
// Key: GPIO pin number (int)
// Value: Current state (LOW or HIGH, int)
std::map<int, int> pinStates;

int lastControlledPin = -1; // To keep track of the last pin we toggled

// Function to print a warning for risky pins
void printRiskyPinWarning(int pin) {
    Serial.print("WARNING: GPIO");
    Serial.print(pin);
    Serial.println(" has special functions (e.g., boot mode selection).");
    Serial.println("Toggling this pin could affect the ESP32's boot behavior or stability.");
    Serial.println("Proceed at your own risk. Are you sure you want to toggle? (Y/N)");
}

// Function to handle reading a Y/N confirmation
bool getConfirmation() {
    while (Serial.available() == 0) {
        // Wait for user input
        delay(10);
    }
    String confirmation = Serial.readStringUntil('\n');
    confirmation.trim();
    confirmation.toUpperCase(); // Convert to uppercase for easier comparison
    return confirmation.equals("Y");
}


void setup() {
    Serial.begin(115200); // Initialize serial communication at 115200 baud
    // Wait for the serial port to connect. Needed for native USB port boards only.
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n--- ESP32 GPIO Toggle Test ---");
    Serial.println("Purpose: Test individual GPIO pins by toggling their state.");
    Serial.println("Expected Voltage Levels (for digitalWrite):");
    Serial.println("  LOW:  ~0V DC");
    Serial.println("  HIGH: ~3.3V DC");
    Serial.println("------------------------------");
    Serial.println("Enter a GPIO pin number (e.g., 4, 13, 27) in the serial monitor to toggle its state.");
    Serial.println("Certain pins (like GPIO0, GPIO2) have special functions and will prompt for confirmation.");
    Serial.println("\nEnter a pin number (Omit the 'GPIO' prefix): ");
}

void loop() {
    if (Serial.available() > 0) {
        String inputString = Serial.readStringUntil('\n');
        inputString.trim();

        int pinNumber = inputString.toInt();

        // --- Pin Number Validation for commonly used GPIOs ---
        bool isValidOutputPin = false;
        bool isRiskyPin = false;

        switch (pinNumber) {
            case 0: case 2:
                isRiskyPin = true;
                isValidOutputPin = true;
                break;
            case 4: case 5:
            case 12: case 13: case 14: case 15: case 16: case 17:
            case 18: case 19: case 21: case 22: case 23:
            case 25: case 26: case 27:
            case 32: case 33:
                isValidOutputPin = true;
                break;
            default:
                isValidOutputPin = false;
                break;
        }

        if (isValidOutputPin) {
            // If a new pin is selected, and it's different from the last, set the last one to LOW
            if (lastControlledPin != -1 && lastControlledPin != pinNumber) {
                if (pinStates.find(lastControlledPin) != pinStates.end()) {
                    digitalWrite(lastControlledPin, LOW);
                    pinStates[lastControlledPin] = LOW;
                    Serial.print("GPIO");
                    Serial.print(lastControlledPin);
                    Serial.println(" set back to Low.");
                }
            }

            if (isRiskyPin) {
                printRiskyPinWarning(pinNumber);
                if (!getConfirmation()) {
                    Serial.println("Toggle cancelled. Enter another pin number: ");
                    // Important: If cancelled, do NOT update lastControlledPin
                    return; 
                }
            }

            // Set the current pin as the last controlled pin
            lastControlledPin = pinNumber;

            // Check if the pin's state is already in our map
            if (pinStates.find(pinNumber) == pinStates.end()) {
                // If not, initialize it as an OUTPUT and set to LOW
                pinMode(pinNumber, OUTPUT);
                digitalWrite(pinNumber, LOW);
                pinStates[pinNumber] = LOW;
                Serial.print("Pin ");
                Serial.print(pinNumber);
                Serial.println(" initialized to Low (~0V).");
            }

            // Get the current state from our map
            int currentState = pinStates[pinNumber];
            
            // Determine the new state (toggle)
            int newState = (currentState == LOW) ? HIGH : LOW;

            // Apply the new state to the pin
            digitalWrite(pinNumber, newState);
            
            // Update the state in our map
            pinStates[pinNumber] = newState;

            // Print the result
            Serial.print("Pin ");
            Serial.print(pinNumber);
            Serial.print(" toggled to ");
            Serial.println((newState == HIGH) ? "High (~3.3V)" : "Low (~0V)");

        } else {
            Serial.print("Pin ");
            Serial.print(pinNumber);
            Serial.println(" is not a commonly used or safe output pin for this test.");
            Serial.println("Please enter a commonly usable GPIO pin number for output (e.g., 2, 4, 12-19, 21-23, 25-27, 32, 33).");
        }
        Serial.println("\nEnter a pin number (Omit the 'GPIO' prefix): "); // Prompt again for the next input
    }
}
