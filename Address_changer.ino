#include <Wire.h>

#define MAX_DEVICES 10

byte foundAddresses[MAX_DEVICES];
int deviceCount = 0;

void setup() {
  Wire.begin();
  Serial.begin(57600);
  
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
  Serial.println("SingleTact Flexible Address Changer");
  Serial.println("-----------------------------------");
  Serial.println("Automatically scanning for connected sensors...");
  
  // Scan I2C bus at startup
  scanI2C();
  
  displayMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();
    
    if (input >= '1' && input <= '9') {
      int index = input - '1';
      if (index < deviceCount) {
        promptForNewAddress(index);
      } else {
        Serial.println("Invalid selection. Please try again.");
        displayMenu();
      }
    } 
    else if (input == 's' || input == 'S') {
      Serial.println("Rescanning I2C bus...");
      scanI2C();
      displayMenu();
    } 
    else if (input == 'h' || input == 'H' || input == '?') {
      displayMenu();
    }
    else {
      // Clear any other characters
      while (Serial.available()) {
        Serial.read();
      }
    }
  }
  
  delay(50);
}

void scanI2C() {
  byte error, address;
  deviceCount = 0;
  
  Serial.println("Scanning I2C bus for devices...");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      if (deviceCount < MAX_DEVICES) {
        foundAddresses[deviceCount] = address;
        deviceCount++;
      }
      
      Serial.print("I2C device found at address 0x");
      printHex(address);
      
      // Check if this is the default SingleTact address
      if (address == 0x04) {
        Serial.print(" (Default SingleTact address)");
      }
      
      Serial.println();
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found! Check connections.");
  } else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.print(" device(s) on the I2C bus");
    Serial.println();
  }
  
  Serial.println("-----------------------------------");
}

void displayMenu() {
  Serial.println();
  Serial.println("Menu Options:");
  Serial.println("  s - Rescan I2C bus for devices");
  
  if (deviceCount > 0) {
    Serial.println("  Select a device to change its address:");
    
    for (int i = 0; i < deviceCount; i++) {
      Serial.print("  ");
      Serial.print(i + 1);
      Serial.print(" - Change device at address 0x");
      printHex(foundAddresses[i]);
      
      if (foundAddresses[i] == 0x04) {
        Serial.print(" (Default SingleTact address)");
      }
      
      Serial.println();
    }
  }
  
  Serial.println("  h - Display this menu");
  Serial.println("-----------------------------------");
}

void promptForNewAddress(int deviceIndex) {
  byte currentAddress = foundAddresses[deviceIndex];
  
  Serial.print("Selected device at address 0x");
  printHex(currentAddress);
  Serial.println();
  Serial.println("Enter new address as 0x followed by two hex digits");
  Serial.println("Example: 0x0A for address 10, 0x10 for address 16");
  
  // Wait for "0x" prefix
  while (true) {
    if (Serial.available() >= 2) {
      char prefix[2];
      prefix[0] = Serial.read();
      prefix[1] = Serial.read();
      
      if (prefix[0] == '0' && (prefix[1] == 'x' || prefix[1] == 'X')) {
        Serial.print("0x"); // Echo back the prefix
        break;
      } else {
        // Clear buffer and prompt again
        Serial.println("Error: Input must start with 0x");
        Serial.println("Enter new address as 0x followed by two hex digits:");
        
        // Clear any remaining characters
        while (Serial.available()) {
          Serial.read();
        }
      }
    }
    delay(10);
  }
  
  // Now wait for the two hex digits
  while (true) {
    if (Serial.available() >= 2) {
      char hexDigits[2];
      hexDigits[0] = Serial.read();
      hexDigits[1] = Serial.read();
      
      Serial.print(hexDigits[0]);
      Serial.print(hexDigits[1]);
      Serial.println(); // New line after full input
      
      // Convert the hex digits to a byte
      byte newAddress = hexCharToByte(hexDigits[0]) << 4 | hexCharToByte(hexDigits[1]);
      
      // Check if address is valid
      if (newAddress >= 0x03 && newAddress <= 0x7F) {
        // Confirm the change
        Serial.print("Change address from 0x");
        printHex(currentAddress);
        Serial.print(" to 0x");
        printHex(newAddress);
        Serial.println("? (y/n)");
        
        // Wait for confirmation
        while (true) {
          if (Serial.available() > 0) {
            char confirm = Serial.read();
            
            if (confirm == 'y' || confirm == 'Y') {
              changeAddress(currentAddress, newAddress);
              break;
            } 
            else if (confirm == 'n' || confirm == 'N') {
              Serial.println("Address change cancelled");
              displayMenu();
              break;
            }
          }
          delay(10);
        }
      } else {
        Serial.print("Error: Invalid address 0x");
        printHex(newAddress);
        Serial.println(" - must be between 0x03 and 0x7F");
        displayMenu();
      }
      break;
    }
    delay(10);
  }
  
  // Clear any remaining characters
  while (Serial.available()) {
    Serial.read();
  }
}

// Convert a hex character to its 4-bit value
byte hexCharToByte(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else {
    return 0; // Default for invalid characters
  }
}

void changeAddress(byte currentAddress, byte newAddress) {
  byte i2cPacketLength = 5;
  byte outgoingI2CBuffer[i2cPacketLength];
  
  outgoingI2CBuffer[0] = 0x02;  // Command
  outgoingI2CBuffer[1] = 0x00;  // Offset
  outgoingI2CBuffer[2] = 0x01;  // Length
  outgoingI2CBuffer[3] = newAddress;  // New address
  outgoingI2CBuffer[4] = 0xFF;  // End marker
  
  Wire.beginTransmission(currentAddress);
  Wire.write(outgoingI2CBuffer, i2cPacketLength);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.print("Successfully changed address to 0x");
    printHex(newAddress);
    Serial.println();
    Serial.println("Rescanning I2C bus to verify...");
    delay(500);  // Give sensor time to reset
    scanI2C();
  } else {
    Serial.print("Error changing address. Error code: 0x");
    printHex(error);
    Serial.println();
    Serial.println("Make sure sensor is connected and address is correct.");
  }
  
  displayMenu();
}

// Helper function to print byte as hex
void printHex(byte value) {
  if (value < 16) {
    Serial.print("0");
  }
  Serial.print(value, HEX);
}