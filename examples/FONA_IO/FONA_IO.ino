#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include "Adafruit_FONA.h"

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
#define FONA_KEY 5

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA f = Adafruit_FONA(FONA_RST);

Adafruit_FONA_Client fona = Adafruit_FONA_Client(&f);
PubSubClient client("io.adafruit.com", 1883, mqtt_callback, fona);

// change the key and feed names
#define AIO_KEY "xxxxxxxx"
#define PUBLISH_TOPIC "api/feeds/test/data/send.json"
#define SUBSCRIBE_TOPIC "api/feeds/test/data/receive.json"

unsigned long time;

void setup() {

  pinMode(FONA_KEY, OUTPUT);

  Serial.begin(115200);
  fonaSS.begin(4800);

  Serial.println(F("Starting..."));

  delay(2000);
  reset();

  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA..."));
    while(1);
  }

  Serial.println(F("FONA found."));

  while(! mqtt_connect()) {
    Serial.println(F("AIO connect failed... trying again"));
    reset();
  }

  Serial.println(F("AIO connected."));

  Serial.print(F("Subscribing to: "));
  Serial.print(SUBSCRIBE_TOPIC);

  client.subscribe(SUBSCRIBE_TOPIC);
  time = millis();

}

void loop() {

  // required for MQTT connection
  client.loop();

  // keep the connection alive
  if(! client.connected())
    mqtt_connect();

  // not yet
  if(time > millis())
    return;

  // push data
  client.publish(PUBLISH_TOPIC, "4");
  Serial.print(F("Sent '4' to: "));
  Serial.println(PUBLISH_TOPIC);

  // wait a couple seconds for the next publish
  time = millis() + 2000;

}

void reset() {

  Serial.println(F("Resetting FONA..."));

  digitalWrite(FONA_KEY, HIGH);
  delay(2000);
  digitalWrite(FONA_KEY, LOW);
  delay(2000);

}

boolean mqtt_connect() {

  Serial.println(F("Connecting to AIO..."));

  char client_name[30];

  // generate new client name
  sprintf(client_name, "fona-%ul", micros());

  if(! client.connect(client_name, AIO_KEY, NULL))
    return false;

  return true;

}

void mqtt_callback (char* topic, byte* payload, unsigned int length) {

  // dump topic and payload from subscriptions
  Serial.print(F("Received: "));
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println();

}
