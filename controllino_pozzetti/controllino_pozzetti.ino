#include <Ethernet.h>

#define RISCALDATORE 2        // DO0
#define CARICO_ACQUA 3        // DO1
#define POMPA_PERISTALTICA 4  // DO2
#define AGITATORE 5           // DO3
#define TENSIONE_SENSORE_1 6  // DO4
#define TENSIONE_SENSORE_2 7  // DO5
#define TENSIONE_SENSORE_3 8  // DO6

#define IN_EMERGENZA A0
#define IN_TEMP_POZZETTO A14 
#define IN_LIVELLO A2 



byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF
};

IPAddress ip(192, 168, 100, 4);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(0, 0, 0, 0);
EthernetServer server(80);
EthernetClient client;

volatile float t_vasca, livello;
volatile bool emergenza = false;

// comunicazione verso PC
volatile unsigned long previousMillisAnalogs = 0;
volatile unsigned long previousMillisStatus = 0;
volatile unsigned long lastCmdMillis = 0;

// variabili per la gestione del protocollo tra PC e ardunio
const byte numChars = 128;
char receivedChars[numChars];
boolean newData = false;



void setup() {
  pinMode(IN_EMERGENZA, INPUT);
  pinMode(IN_TEMP_POZZETTO, INPUT);
  pinMode(IN_LIVELLO, INPUT);

  pinMode(RISCALDATORE, OUTPUT);
  pinMode(CARICO_ACQUA, OUTPUT);
  pinMode(POMPA_PERISTALTICA, OUTPUT);
  pinMode(AGITATORE, OUTPUT);
  pinMode(TENSIONE_SENSORE_1, OUTPUT);
  pinMode(TENSIONE_SENSORE_2, OUTPUT);
  pinMode(TENSIONE_SENSORE_3, OUTPUT);

  Serial.begin(9600);

  Ethernet.begin(mac, ip);
  Serial.print("Ethernet BEGIN! IP=");
  Serial.println(Ethernet.localIP());
  server.begin();
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
float filterInput(float var, int raw) {
  return var * 0.99f + raw * 0.01f;
}

String format24V(int raw) {
  int c = map(raw, 0, 1023, 0, 24000);
  float v = c / 1000.0f;
  return String(v, 3);
}
void stoppa_tutto() {
  digitalWrite(POMPA_PERISTALTICA, LOW);
  digitalWrite(TENSIONE_SENSORE_1, LOW);
  digitalWrite(TENSIONE_SENSORE_2, LOW);
  digitalWrite(TENSIONE_SENSORE_3, LOW);
  digitalWrite(RISCALDATORE, LOW);
  digitalWrite(CARICO_ACQUA, LOW);
  digitalWrite(AGITATORE, LOW);
}
void loop() {
  unsigned long currentMillis = millis();
  unsigned long currentMicros = micros();
  //lastCmdMillis è dentro la funzione ParseCommands


  // lettura e media analogiche
  if (currentMicros - previousMillisAnalogs >= 5000000) {
    previousMillisAnalogs = currentMicros;
    // t_vasca = filterInput(t_vasca, analogRead(IN_TEMP_POZZETTO));
    //t_vasca = analogRead(IN_TEMP_POZZETTO);
    int rawvalue = analogRead(IN_TEMP_POZZETTO);
    
    t_vasca = map(rawvalue, 0, 1023, 0, 10000);
    Serial.println(t_vasca);

    //livello = filterInput(livello, analogRead(IN_LIVELLO));
    livello = digitalRead(IN_LIVELLO);
    //Serial.println(livello);
  }
  if (currentMillis - previousMillisStatus >= 100) {
    previousMillisStatus = currentMillis;
    emergenza = digitalRead(IN_EMERGENZA);
    if (emergenza) {
      // sicurezze attive: stoppa tutto
      stoppa_tutto();
    }
    // if (currentMillis - lastCmdMillis >= 5000) { //questo qua devo tiralo via, mi manda basso l'uscita
    //   // PC morto: stoppa tutto
    //   stoppa_tutto();
    // }
  }

  // comandi in arrivo??
  client = server.available();
  if (client) {
    Serial.print("Client BEGIN! IP=");
    Serial.println(client.remoteIP());
    RecvWithEndMarker();
    ParseCommands();
  }
}

void RecvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '|';
  char rc;
  while (client.available() && newData == false) {
    rc = client.read();
    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void ParseCommands() {
  if (newData == true) {
    lastCmdMillis = millis();
    newData = false;
    char* buf = strtok(receivedChars, ";");
    int command = atoi(buf);

    // STATUS
    if (command == 1) {
      String buff = "#" + String(emergenza ? "1" : "0") + ";" + String(mapfloat(t_vasca, 0, 1023, 0, ), 1) + ";" + String(mapfloat(livello, 0, 1023, 0, 24), 3) + ";";
      //cambiare l'invio dei valori dei sensori e del valore di temperatura
      client.println(buff);
      client.flush();
      //Serial.println(buff);
    } else if (command == 2) {
      buf = strtok(NULL, ";");
      int state = atol(buf);
      digitalWrite(RISCALDATORE, state ? HIGH : LOW);
      Serial.println("RISCALDATORE=>" + state);
    } else if (command == 3) {
      buf = strtok(NULL, ";");
      int state = atol(buf);
      digitalWrite(CARICO_ACQUA, state ? HIGH : LOW);
      Serial.println("CARICO_ACQUA=>" + state);
    } else if (command == 4) {
      buf = strtok(NULL, ";");
      int state = atol(buf);
      digitalWrite(POMPA_PERISTALTICA, state ? HIGH : LOW);
      Serial.println("POMPA_PERISTALTICA=>" + state);
    } else if (command == 5) {
      buf = strtok(NULL, ";");
      int state = atol(buf);
      digitalWrite(AGITATORE, state ? HIGH : LOW);
      Serial.println("AGITATORE=>" + state);
    } else if (command == 6) {
      buf = strtok(NULL, ";");
      int state = atol(buf);
      digitalWrite(TENSIONE_SENSORE_1, state ? HIGH : LOW);
      Serial.println("TENSIONE_SENSORE_1=>" + state);
    } else if (command == 7) {
      buf = strtok(NULL, ";");
      int state = atol(buf);
      digitalWrite(TENSIONE_SENSORE_2, state ? HIGH : LOW);
      Serial.println("TENSIONE_SENSORE_2=>" + state);
    } else if (command == 8) {
      buf = strtok(NULL, ";");
      int state = atol(buf);
      digitalWrite(TENSIONE_SENSORE_3, state ? HIGH : LOW);
      Serial.println("TENSIONE_SENSORE_3=>" + state);
    }
  }
}
