/**********************************************
   Ampel für das BSA (Bogensport Ampelsystem)
   Created by the BSA Team
   Visit: ampel.gras-it.de for more informations
 **********************************************/

/**********************************************
   Includes
 **********************************************/
/*Ethernet Shield*/
#include <Ethernet.h>
/*RGB Matrix*/
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
/*Timer*/
#include <Timer.h>

/**********************************************
   Definitions
 **********************************************/
/*RGB Matrix*/
#define OE   9
#define LAT 10
#define CLK 11
#define A   A0
#define B   A1
#define C   A2
#define D   A3

/*Amel System GPIO*/
#define PIN_RED     11
#define PIN_YELLOW  12
#define PIN_GREEN   13
#define PIN_HORN    0
#define MAX_LENGTH  5     //Max char for non rotating char
#define SPEED       500   //Speed delay in ms

/**********************************************
   Variables and Instances
 **********************************************/
/*RGB Matrix*/
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64);

/*Ethernet Server*/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //physical mac address
byte ip[] = { 192, 168, 1, 10 };                      //IP Address in lan
byte gateway[] = { 192, 168, 1, 1 };                   //internet access via router
byte subnet[] = { 255, 255, 255, 0 };                  //subnet mask
EthernetServer server(80);                             //server port

/*Ampel System*/
bool red;
bool yellow;
bool green;
bool horn;
String buffStr;
bool changeStr;
String str;

/*Timer*/
Timer tim;

void setup() {
  // set Pins
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_RED, OUTPUT);

  // set Variables
  red = false;
  yellow = false;
  green = false;
  horn = false;
  buffStr = "";
  changeStr = false;
  str = "";

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();

  // initialize LED Matrix
  matrix.begin();
  matrix.setTextSize(2);     // size 1 == 8 pixels high
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves
  matrix.setCursor(8, 0);    // start at top left, with 8 pixel of spacing
  matrix.setTextColor(matrix.Color333(7, 7, 7));
  matrix.println(Ethernet.localIP());

  tim.after(500, set_LEDMatrix);
}

void loop() {
  update_API();

  set_pins();

  tim.update();
}

/**********************************************
   Helper functions
 **********************************************/
/*set GPIO Pins*/
void set_pins() {
  if (red) {
    digitalWrite(PIN_RED, HIGH);
  }
  else {
    digitalWrite(PIN_RED, LOW);
  }

  if (yellow) {
    digitalWrite(PIN_YELLOW, HIGH);
  }
  else {
    digitalWrite(PIN_YELLOW, LOW);
  }

  if (green) {
    digitalWrite(PIN_GREEN, HIGH);
  }
  else {
    digitalWrite(PIN_GREEN, LOW);
  }

  if (horn) {
    digitalWrite(PIN_HORN, HIGH);
  }
  else {
    digitalWrite(PIN_HORN, LOW);
  }
}

/*set Text on LED Matrix*/
void set_LEDMatrix() {
  if (changeStr) {
    buffStr = str;
    changeStr = false;
  }
  if (buffStr != "") {
    matrix.println();
    matrix.println(buffStr);
    if (buffStr.length() > MAX_LENGTH) {
      buffStr = buffStr.substring(buffStr.length() - 2,buffStr.length() - 1) + buffStr.substring(0,buffStr.length() - 2);
    }
  }
}

/*simulate the Ampel API*/
void update_API() {
  // Create a client connection
  String answer = "";
  String readString = "";
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
        }

        //if HTTP request has ended
        if (c == '\n') {
          //check set action
          if (readString.indexOf("act=set") > 0) {
            for (int i = 1; i < 6; i++) {
              String buff = getValue(readString, '&', i);
              if (buff != "") {
                if (buff.startsWith("r=")) {
                  if (getValue(buff, '=', 1).equalsIgnoreCase("true")) {
                    red = true;
                    answer += "True|";
                  }
                  else {
                    red = false;
                    answer += "False|";
                  }
                }
                else if (buff.startsWith("y=")) {
                  if (getValue(buff, '=', 1).equalsIgnoreCase("true")) {
                    yellow = true;
                    answer += "True|";
                  }
                  else {
                    yellow = false;
                    answer += "False|";
                  }
                }
                else if (buff.startsWith("g=")) {
                  if (getValue(buff, '=', 1).equalsIgnoreCase("true")) {
                    green = true;
                    answer += "True|";
                  }
                  else {
                    green = false;
                    answer += "False|";
                  }
                }
                else if (buff.startsWith("h=")) {
                  if (getValue(buff, '=', 1).equalsIgnoreCase("true")) {
                    horn = true;
                    answer += "True|";
                  }
                  else {
                    horn = false;
                    answer += "False|";
                  }
                }
                else if (buff.startsWith("s=")) {
                  str = getValue(buff, '=', 1);
                  changeStr = true;
                  answer += str + "|";
                }
              }
            }
          }
          //check get action
          else if (readString.indexOf("act=get") > 0) {
            String buff = getValue(readString, '&', 1);
            for (int i = 0; i < buff.length(); i++) {
              if (buff.charAt(i) == 'r' || buff.charAt(i) == 'R') {
                if (red) {
                  answer += "True|";
                }
                else {
                  answer += "False|";
                }
              }
              else if (buff.charAt(i) == 'y' || buff.charAt(i) == 'Y') {
                if (yellow) {
                  answer += "True|";
                }
                else {
                  answer += "False|";
                }
              }
              else if (buff.charAt(i) == 'g' || buff.charAt(i) == 'G') {
                if (green) {
                  answer += "True|";
                }
                else {
                  answer += "False|";
                }
              }
              else if (buff.charAt(i) == 'h' || buff.charAt(i) == 'H') {
                if (horn) {
                  answer += "True|";
                }
                else {
                  answer += "False|";
                }
              }
              else if (buff.charAt(i) == 's' || buff.charAt(i) == 'S') {
                answer += str + "|";
              }
            }
          }
          client.println("HTTP/1.1 200 OK"); //send new page
          client.println("Content-Type: text/html");
          client.println();
          client.println(answer.substring(0, answer.length() - 2));

          delay(1);
          //stopping client
          client.stop();
        }
      }
    }
  }
}

/*split String Into Array*/
String getValue(String data, char separator, int index){
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

