#include <SoftwareSerial.h>

#define HC10_RX 8   // сюда TX от HC-10
#define HC10_TX 9   // отсюда на RX HC-10

SoftwareSerial hc10(HC10_RX, HC10_TX);
String cmdBuffer = "";

void setup() {
  Serial.begin(9600);
  hc10.begin(9600);
  delay(500);
  Serial.println("Ready. Type AT commands below.");
}

void loop() {
  if (hc10.available()) {
    Serial.write(hc10.read());
  }

  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (cmdBuffer.length() > 0) {
        hc10.print(cmdBuffer);
        hc10.print("\r\n");
        cmdBuffer = "";
      }
    } else {
      cmdBuffer += c;
    }
  }
}