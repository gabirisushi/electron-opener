// This #include statement was automatically added by the Particle IDE.
#include <ncd-4-relay-maqen.h>

#include "Wiegand.h"

WIEGAND wg;
int ledPin = D6;
const int byteSize = 4;
NCD4Relay relayController;
int relayPulse = 200;

// EEPROM convience methods

int lookupInEEPROM(int code) {
    int test;
    int i;
    for (i = 0; i < (EEPROM.length() - 1); i = i + byteSize) {
        EEPROM.get(i, test);
        if (test == code) {
            return i;
        }
    }
    return -1;
}

int firstEmptyEEPROMAddress() {
    int test;
    int i;
    for (i = 0; i < (EEPROM.length() - 1); i = i + byteSize) {
        EEPROM.get(i, test);
        Serial.printlnf("%d = %d", i, test);
        if (test == -1) {
            return i;
        }
    }
}

// Application

void setup() {
    
	Serial.begin(115200);

	Serial.println("Let's go!");
	
	// Setup reader led
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, LOW);

    // For testing
	// EEPROM.clear();

	Serial.printlnf("%d bytes total memory", EEPROM.length());

	Particle.function("addKey", addKey);
	Particle.function("removeKey", removeKey);
	Particle.function("openPort", openPort);

    // Setup relay controller
    relayController.setAddress(0, 0, 0);

	// Setup wiegand protocol from reader
	wg.begin();
}

int addKey(String params) {
	int key = params.toInt();
	Serial.printlnf("Trying to save key %d, %d bytes", key, sizeof(key));
	if (key > 0) {
	    int address = firstEmptyEEPROMAddress();
	    if (address == -1) {
	        Serial.println("EEPROM full?");
	        return -1;
	    }
	    if (lookupInEEPROM(key) != -1) {
	        Serial.println("Key already stored in EEPROM");
	        return 1;
	    }
	    EEPROM.put(address, key);
		Serial.printlnf("Saved key to EEPROM (%d), %d bytes", address, sizeof(key));
		return 1;
	}
	return -1;
}

int removeKey(String params) {
    int key = params.toInt();
    Serial.printlnf("Remove key %d", key);
    int address = lookupInEEPROM(key);
    Serial.printlnf("Address %d", address);
    if (address == -1) {
        Serial.println("Unable to lookup key");
        return -1;
    }
    EEPROM.put(address, -1);
    Serial.println("Removed key");
    return 1;
}

int openPort(String params) {
    digitalWrite(ledPin, HIGH);
    relayController.turnOnAllRelays();
    delay(relayPulse);
    relayController.turnOffAllRelays();
    digitalWrite(ledPin, LOW);
    return 1;
}

void loop() {
	if(wg.available())
	{
		int code = wg.getCode();
		Serial.println("Shown " + String(code));
		/*Serial.println(sizeof(code));
		EEPROM.put(0, code);
		int received;
		Serial.println(EEPROM.get(0, received));*/
		Particle.publish("key-swiped", String(code));
		if (valid(code)) {
			Serial.println("Key is allowed!");
            openPort("");			
		} else {
			Serial.println("Key is not allowed");
			// Particle.publish("key-invalid", String(code));
		}
		Serial.print("Wiegand HEX = ");
		Serial.print(wg.getCode(), HEX);
		Serial.print(", DECIMAL = ");
		Serial.print(wg.getCode());
		Serial.print(", Type W");
		Serial.println(wg.getWiegandType());
	}
}

void flash() {
	digitalWrite(ledPin, HIGH);
	delay(100);
	digitalWrite(ledPin, LOW);
	delay(100);
	digitalWrite(ledPin, HIGH);
	delay(100);
	digitalWrite(ledPin, LOW);
	delay(100);
	digitalWrite(ledPin, HIGH);
	delay(100);
	digitalWrite(ledPin, LOW);
}

bool valid(int code) {
	int address = lookupInEEPROM(code);
	if (address != -1) {
		return true;
	}
	return false;
}
