#include <UIPEthernet.h>

// CONSTANTS: START

const byte DEVICE_ID = B10;

const int PIN_LED_RED   = A0;
const int PIN_LED_GREEN = A1;
const int PIN_LED_BLUE  = A2;
const int PIN_BUTTON    = 3;

const long COLOR_RED    = 0xFF0000;
const long COLOR_ORANGE = 0x0000FF;
const long COLOR_GREEN  = 0x00FF00;
const long COLOR_OFF    = 0X000000;
const long COLOR_WHITE  = 0xFFFFFF;
const long COLOR_FLIRT  = 0xA000A0;


const byte STATUS_ROOM_UNDEFINED    = B00;
const byte STATUS_ROOM_FREE         = B01;
const byte STATUS_ROOM_IN_MEETING   = B10;
const byte STATUS_ROOM_MEETING_SOON = B11;

const byte COMMAND_REGISTER = B01;
const byte COMMAND_RESERVE  = B10;
const byte COMMAND_STATUS   = B11;

const byte BUTTON_ACTIVE   = B0;
const byte BUTTON_INACTIVE = B1;

// CONSTANTS: FINISH

// GLOBAL VARIBALES: START

int statusRoom = STATUS_ROOM_UNDEFINED;
int buttonState = BUTTON_INACTIVE;

long unsigned buttonLastActive = 0;
long unsigned buttonDelay = 1000;
long unsigned checkStateLastActive = 0;
long unsigned checkStateDelay = 60000;

bool isWaiting = false;
long currentLoadingColor;

// GLOBAL VARIBALES: FINISH


EthernetClient client;
void setup() {

  Serial.begin(9600);

  uint8_t mac[6] = {0x90, 0x11, 0x02, 0xF1, 0x94, 0x05};
  IPAddress ip(192, 168, 3, 3);

  Serial.println("Starting Network");
  Ethernet.begin(mac, ip);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
}

void loop() {
  clientConnect();

  if (isButtonClicked()) {
    debounceOnButtonClick();
  }

  if (client.available()) {
    byte buf[2];
    client.readBytes(buf, 2);
    byte command = buf[0];
    byte data = buf[1];

    if (COMMAND_STATUS == command) {
      statusRoom = data;
      analogWriteStatusRoom(statusRoom);
    }
      
    isWaiting = false;
  } else {
    if (isWaiting) {
      loading();
    }
  }
}

/**
   Try connect to the server
*/
void clientConnect() {
  while (client.connected() != 1) {
    Serial.println("try connection to server");
    if (! client.connect(IPAddress(192, 168, 3, 2), 8080)) {
      Serial.println("not this time");
      continue;
    }
    registerDevice();
    isWaiting = true;
    Serial.println("success connected");
  }
}

bool isButtonClicked() {
  return ! digitalRead(PIN_BUTTON);
}

void debounceOnButtonClick() {
  if (buttonLastActive != 0 && (millis() - buttonLastActive) < buttonDelay) {
    buttonLastActive = millis();
    return;
  }

  buttonLastActive = millis();

  onButtonClick();
}

void onButtonClick() {
  if (statusRoom == STATUS_ROOM_FREE) {
    reserverRoom();
    isWaiting = true;
  }
}

/**
   Write status room to RGB LED
*/
void analogWriteStatusRoom(int statusRoom)
{
  long color = mapStatusRoomToColor(statusRoom);
  anologWriteColor(color);
}

/**
   Map status room to RGB color
*/
long mapStatusRoomToColor(int statusRoom) {
  switch (statusRoom) {
    case STATUS_ROOM_FREE:
      return COLOR_GREEN;
    case STATUS_ROOM_IN_MEETING:
      return COLOR_RED;
    case STATUS_ROOM_MEETING_SOON:
      return COLOR_ORANGE;
    case STATUS_ROOM_UNDEFINED:
    default:
      return COLOR_OFF;
  }
}

/**
   Write a hex long do RGB LED
*/
void anologWriteColor(long color) {
  analogWrite(PIN_LED_RED, color >> 16);
  analogWrite(PIN_LED_GREEN, color >> 8 & 0x00FF);
  analogWrite(PIN_LED_BLUE, color & 0x0000FF);
}

void loading() {
  if (currentLoadingColor == COLOR_OFF) {
    currentLoadingColor = COLOR_FLIRT;
  } else {
    currentLoadingColor = COLOR_OFF;
  }

  anologWriteColor(currentLoadingColor);

  delay(100);
}

void registerDevice() {
  sendCommand(COMMAND_REGISTER, DEVICE_ID);
}

void reserverRoom() {
  sendCommand(COMMAND_RESERVE, B0);
}

void sendCommand(byte command, byte data) {
  byte buf[2] = {command, data};
  client.write(buf, 2);
}
