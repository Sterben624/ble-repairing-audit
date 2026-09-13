#include <SoftwareSerial.h>

#define STM_RX 14  // сюда TX от STM32 (debug UART)
#define STM_TX 12  // не используется для приёма трассировки, но пусть будет

SoftwareSerial stmUart(STM_RX, STM_TX);

void setup() {
  Serial.begin(9600);
  stmUart.begin(115200);
  delay(500);
  Serial.println("Listening on STM32 debug UART...");
}

void loop() {
  if (stmUart.available()) {
    Serial.write(stmUart.read());
  }
}