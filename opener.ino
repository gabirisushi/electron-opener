int openPort(String command);
int relayPin = D3;
int relayPinTwo = D4;
int ledPin = D7;
int defaultDelay = 200;

void setup()
{
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  pinMode(relayPinTwo, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(relayPin, LOW);
  digitalWrite(relayPinTwo, LOW);
  digitalWrite(ledPin, LOW);

  // register the cloud function
  bool success = Particle.function("openPort", openPort);
}

void loop()
{
  // this loops forever
  if (Particle.connected() == false) {
    Particle.connect();
  }
}

// this function automagically gets called upon a matching POST request
int openPort(String params) {
    digitalWrite(ledPin, HIGH);
    if (params == "1" || params == "") {
        digitalWrite(relayPin, HIGH);
    } else if (params == "2") {
        digitalWrite(relayPinTwo, HIGH);
    } else {
        digitalWrite(relayPin, HIGH);
        digitalWrite(relayPinTwo, HIGH);
    }
    delay(defaultDelay);
    digitalWrite(relayPin, LOW);
    digitalWrite(relayPinTwo, LOW);
    digitalWrite(ledPin, LOW);
    Serial.println("Relay now closed");
    return 1;
}