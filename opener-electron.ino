// This #include statement was automatically added by the Particle IDE.
#include <ncd-4-relay-maqen.h>

PRODUCT_ID(3267);
PRODUCT_VERSION(5);

int openPort(String command);
int ledPin = D7;
NCD4Relay relayController;
const int defaultDelay = 1000;

void setup()
{
  Serial.begin(115200);

  relayController.setAddress(0, 0, 0);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // register the cloud function
  bool success = Particle.function("openPort", openPort);
  
  Serial.println("Program ready");
}

int turnOnRelay(int number) {
    digitalWrite(ledPin, HIGH);
    delay(defaultDelay);
    relayController.turnOffAllRelays();
    digitalWrite(ledPin, LOW);
    return 1;
}

int openPort(String params) {

    if (params == "") {
        digitalWrite(ledPin, HIGH);
        relayController.turnOnAllRelays();
        delay(defaultDelay);
        relayController.turnOffAllRelays();
        digitalWrite(ledPin, LOW);
        return 1;
    }
    
    if (params == "1") {
        return turnOnRelay(1);
    }
    
    if (params == "2") {
        return turnOnRelay(2);
    }
    
    if (params == "3") {
        return turnOnRelay(3);
    }
    
    if (params == "4") {
        return turnOnRelay(4);
    }
    
    return 0;
}