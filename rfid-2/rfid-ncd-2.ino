SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

#include <Wiegand.h>
#include <NCD2Relay.h>

NCD2Relay relayController;
WIEGAND wg;

const int byteSize = 16;
const int relayPulse = 100;
const int accessCodeLength = 4;
const int accessCodeInputTimeout = 3000;
int accessCode[4] = {9,9,9,9};
int ledPin = D6;
int keysPressed = 0;
int accessCodeTemp[4] = {0,0,0,0};
unsigned long lastKeyPress = millis();

const int requiredDelaySinceRelayPulse = 15000;
unsigned long lastRelayPulse = millis() - requiredDelaySinceRelayPulse;


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
    return 0;
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
	
    // Setup relay controller
    relayController.setAddress(0, 0, 0);
    
	// Setup wiegand protocol from reader
	wg.begin();
	
    Particle.function("addKey", addKey);
	Particle.function("removeKey", removeKey);
	Particle.function("setKeyCode", setKeyCode);
	Particle.function("openPort", openPort);
	Particle.connect();
}

int addKey(String params) {
	int key = (int) strtol(params, NULL, 16);
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

int openPortFromRFID() {
    if ((millis() - lastRelayPulse) > requiredDelaySinceRelayPulse) {
        openPort("");
        lastRelayPulse = millis();
    }
}

int setKeyCode(String params) {
    if (params.length() != accessCodeLength) {
        return 0;
    }
    for (int i = 0; i < accessCodeLength; i++) {
        accessCode[i] = params.substring(i, i + 1).toInt();
    }
    printCode(accessCode);
    return 1;
}

void loop() {
	if(wg.available())
	{
		int code = wg.getCode();
		
		Serial.println("Shown " + String(code, HEX));
		
		// Keypress
		if (wg.getWiegandType() == 4) {
		    addToCode(code);
		    return;
		}
		
	    // Particle.publish("key-swiped", String(code));
		if (valid(code)) {
			Serial.println("Key is allowed!");
            openPortFromRFID();		
		} else {
			Serial.println("Key is not allowed");
			//Particle.publish("key-invalid", String(code));
		}
		Serial.print("Wiegand HEX = ");
		Serial.print(wg.getCode(), HEX);
		Serial.print(", DECIMAL = ");
		Serial.print(wg.getCode());
		Serial.print(", Type W");
		Serial.println(wg.getWiegandType());
		
		openPortFromRFID();
	}
	if (keysPressed > 0 && ((millis() - lastKeyPress) >= accessCodeInputTimeout)) {
	    Serial.println("Key press timeout");
	    resetAccessCodeTemp();
	}
}

void addToCode(int key) {
    accessCodeTemp[keysPressed] = key;
    keysPressed++;
    printCode(accessCodeTemp);
    lastKeyPress = millis();
    if (keysPressed == 4) {
        if (checkCode(accessCodeTemp)) {
            Serial.println("Correct key code!");
            openPort("");
        } else {
            Serial.println("Invalid code");
        }
        // Reset keys pressed
        resetAccessCodeTemp();
    }
}

void printCode(int code[]) {
    Serial.print("Code: ");
    for (int i = 0; i < accessCodeLength; i++) {
        Serial.print(code[i]);
    }
    Serial.println("");
}

bool checkCode(int code[]) {
    for (int i = 0; i < accessCodeLength; i++) {
        if (accessCode[i] != code[i]) {
            return false;
        }
    }
    return true;
}

void resetAccessCodeTemp() {
    accessCodeTemp[0] = 0;
    accessCodeTemp[1] = 0;
    accessCodeTemp[2] = 0;
    accessCodeTemp[3] = 0;
    keysPressed = 0;
    Serial.println("Key code reset");
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
