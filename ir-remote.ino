
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "charif-2.4"
#define WLAN_PASS       "mahmoudi"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "charif"
#define AIO_KEY         "XXXXXXXXXXXXXxx"

/************ Global State (you don't need to change this!) ******************/

uint16_t Samsung_power_toggle[71] = {
    38000, 1, 1, 170, 170, 20, 63, 20, 63, 20, 63, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 63, 20, 63, 20, 63, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 63, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 63, 20,
    20, 20, 63, 20, 63, 20, 63, 20, 63, 20, 63, 20, 63, 20, 1798};

IRsend irsend(4);  // An IR LED is controlled by GPIO pin 4 (D2)


// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/


// Setup a feed called 'onoff' for subscribing to changes to the button
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff", MQTT_QOS_1);

/*************************** Sketch Code ************************************/



void onoffcallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);

   Serial.println("Toggling power");
#if SEND_GLOBALCACHE
  irsend.sendGC(Samsung_power_toggle, 71);
#else  // SEND_GLOBALCACHE
  Serial.println("Can't send because SEND_GLOBALCACHE has been disabled.");
#endif  // SEND_GLOBALCACHE
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(1000);
   digitalWrite(LED_BUILTIN, LOW);
}


void setup() {
  Serial.begin(115200);
  delay(10);

  irsend.begin();
   pinMode(LED_BUILTIN, OUTPUT);

  Serial.println(F("IR  Remote Controller"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  onoffbutton.setCallback(onoffcallback);
  
  // Setup MQTT subscription for time feed.

  mqtt.subscribe(&onoffbutton);

}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  mqtt.processPackets(10000);
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 10 seconds...");
       mqtt.disconnect();
       delay(10000);  // wait 10 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
