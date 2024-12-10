/*
To connect to Arduino via Serial (USB) go to File->Preferences and paste "http://arduino.esp8266.com/stable/package_esp8266com_index.json" 
into the additional boards manager URL and press OK. Then you should be able to go to the board manager (on the left of the screen) and search "esp8266".
install the board and select it by going to Tools->Board:->esp8266->Generic ESP8266 Module. 
To upload code via OTA (Over The Air) to the ESP go to Tools->Port->Network Ports and select the ESP.
Before you upload code you need to reset the ESP (red hardware button) and then click the upload button. The long red Text an the beginning is not an error. 
It will ask you for a password wich is "admin". After uploading there will be an error message that you can ignore.
If you want to connect to the ESP via WiFi to see the output frot the WL-134 (RFID-Chip) you can do that with PuTTY. enter the ip and select connection type other: Telnet.
To connect to the RFID-Chip: connect 5V to the Power and GND to the ESP-Pins. Connect the TxD-Pin from the RFID-Chip to GPIO-12 of the ESP.  
If you have any questions ask me on teams (Elias Wassermann).
*/

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include <SoftwareSerial.h>
#include <Rfid134.h>

#include "TelnetStream.h"

#include <Servo.h>


// The SSID and Password for the Wifi connection (chage these in the File "wifiCredentials.h")
#include "wifiCredentials.h"
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

// The IDs of the fish
constexpr int fish[5] = {0, 2, 3, 4, 7}; 

// Funny sentences that are printed out randomly on contact with a fish
constexpr char* sentences[16] = {
  "Aus Spaß wurde Ernst. Ernst ist heute 3 Jahre alt.",
	"Ich habe einen Kater. Er heißt Felix und liegt auf dem Sofa.",
	"Früher war alles besser. Sogar die Zukunft.",
	"Die Lage ist ernst, aber nicht hoffnungslos. Hoffnungslos ist nur Ernst.",
	"Mein Arzt hat gesagt, ich solle mehr laufen – jetzt habe ich eine Stunde lang versucht, ihm hinterherzukommen.",
	"Ich dachte, ich hätte den Dreh raus – jetzt habe ich nur einen kaputten Schraubenzieher.",
	"Grubi: Ist Mayonaise auch ein Instrument?",
	"Grubi: Der erste Baum ist eingepflanzt worden!",
	"Grubi: Mutterland der Industralisierung ist Asien!",
	"Emil: Wenn ich jetzt Bäume einpflanze, bin ich in 30 Jahren reich!",
	"Grubi: Ich bin ein Hund!",
	"Grubi: Die Erde ist eine Schreibe, weil wenn man im Flugzeug sitzt, fliegt das Flugzeug gerade und nicht nach unten!!!!!",
	"Grubi: Lamine Yamal ist besser wie Ronaldo",
	"Grubi: Die Welt geht morgen unter haben die Simpsons gesagt",
	"Ich habe gegend Baron gespielt, aber er war gar kein Baron hahahahhah",
	"Hohni: Was ist das Gegenteil von einer Polka und auf was fliegt Nils Holgerson in einem Wort: EntenWalzer!"
};

class RfidNotify
{
public:
  // If an error occured during reading of a frame from the rfid-chip
  static void OnError(Rfid134_Error errorCode)
  {
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }

  static void OnPacketRead(const Rfid134Reading& reading)
  {
    // the ID of the Fish (rfid-tag). (The last 4 binary digits)
    int IDOfFish = reading.id & 15; 

    // The size of the sentences array
    int sizeOfArray = sizeof(sentences);

    // a random number between  0 and the size of the array
    int randIndex = rand() % sizeOfArray;
    
    // print out a different message for every fish 
    switch (IDOfFish) {
      case fish[0]:
        TelnetStream.println("Moin Leute Trymacs hier. Ich bin der Pinke Fisch.");
        break;

      case fish[1]:
        TelnetStream.println("Ich bins Tim. Ich habe einen Kater. Ich bin der Lilane Fisch.");
        break;

      case fish[2]:
        TelnetStream.println("Ich bin Bruno und ich bin der Kammeramann. Ich bin der Blaue Fisch.");
        break;

      case fish[3]:
        TelnetStream.println("Bäng Bäng Brotato Gäng. Ich bin der Rote Fisch.");
        break;

      case fish[4]:
        TelnetStream.println("5. Fisch!");
        break;

      default:
        TelnetStream.println("Ich muss hier raus!!!!");
    }

    // print out one random sentence 
    TelnetStream.println(sentences[randIndex]);
  }
};

SoftwareSerial secondarySerial(12, 11); // RX, TX
Rfid134<SoftwareSerial, RfidNotify> rfid(secondarySerial);

Servo servo;

void setup() {
  // Setup Serial
  Serial.begin(9600);
  Serial.println("Booting");

  servo.attach(4);

  // Setup Wifi connection to local network (credentials given in the file "wifiCredentials.h")
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) { // Restart ESP if no connection to the network has been achieved (ex. wrong credentials)
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Set Hostname to "ESP8266"
  ArduinoOTA.setHostname("ESP8266");

  // Set Password for connecting via OTA to "admin"
  ArduinoOTA.setPassword("admin");

  // if code is uploaded via OTA print out progress bar (percent) to Serial.
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  // Handle Error occured while Uploading/Connecting OTA
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  // Start OTA, Telnet, serial to WL-134 and rfid
  ArduinoOTA.begin();
  TelnetStream.begin();
  secondarySerial.begin(9600);
  rfid.begin();

  // print IP to Serial
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  servo.write(0);
  delay(1000);
  servo.write(180);
  delay(1000);
}
