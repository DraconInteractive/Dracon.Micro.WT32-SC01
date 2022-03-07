/*

 Example sketch for TFT_eSPI library.

 No fonts are needed.
 
 Draws a 3d rotating cube on the TFT screen.
 
 Original code was found at http://forum.freetronics.com/viewtopic.php?f=37&t=5495
 
 */

#define BLACK 0x0000
#define WHITE 0xFFFF

#include <SPI.h>
#include <TFT_eSPI.h> 
#include <Adafruit_FT6206.h>
#include <BluetoothSerial.h>

TFT_eSPI tft = TFT_eSPI();
Adafruit_FT6206 ts = Adafruit_FT6206();
BluetoothSerial SerialBT;

String messageIn = "", messageOut = "";
char incomingChar;

int16_t h;
int16_t w;

uint16_t t_y;
uint16_t t_x;

struct Point3d
{
  int x;
  int y;
  int z;
};

struct Point2d
{
  int x;
  int y;
};

bool pressed;
bool onPress, onRelease;

void touchupdate();
void btupdate();
void btreceive ();
void btsend_pos ();
void btsend_state (uint8_t);
void btsend_message(String);
void btsend();

/***********************************************************************************************************************************/
void setup() {
  Serial.begin(115200);

  if (SerialBT.begin("Dracon Touch")) 
  {
    Serial.println("Bluetooth Started. Ready to pair.");
  }
  else
  {
    Serial.println("Bluetooth startup failed.");
  }
  
  // Pins 18/19 are SDA/SCL for touch sensor on this device
  // 40 is a touch threshold
  if (!ts.begin(18, 19, 40)) {
    Serial.println("Couldn't start touchscreen controller");
    while (true);
  }

  tft.init();

  // Thanks to https://github.com/seaniefs/WT32-SC01-Exp
  // for figuring this out (backlight hack)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, 128);

  h = tft.height();
  w = tft.width();

  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);

  pressed = false;
}

void loop() {

  touchupdate();

  btupdate();

  delay(14); // Delay to reduce loop rate (reduces flicker caused by aliasing with TFT screen refresh rate)
}

void touchupdate ()
{
  onPress = false;
  onRelease = false;

  if (ts.touched()) {
    TS_Point p = ts.getPoint();

    p.x = map(p.x, 40, 280, 320, 0); // I'm mapping the touch coordinates to start at 40 and end at 280 because I found this to
    
    uint16_t tt_y = t_y;
    uint16_t tt_x = t_x;
                                       // make the touches more accurete.
    t_y = p.x; // because we are using the screen horizontal the x-axis becomes the y-asis
    t_x = p.y; // and the y-axis becones the x-axis

    uint16_t difX = (t_x - tt_x);
    uint16_t difY = (t_y - tt_y);
    //Serial.printf("%d %d | %d %d\n", t_x, t_y, difX, difY);

    tft.drawCircle(t_x, t_y, 3, TFT_ORANGE);
    tft.drawLine(tt_x, tt_y, t_x, t_y, TFT_RED);

    if (!pressed)
    {
      onPress = true;
    }
    pressed = true;
  }
  else 
  {
    if (pressed)
    {
      onRelease = true;
      tft.fillScreen(TFT_BLACK);
    }
    pressed = false;
  }
}

void btupdate () {
  /*
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }*/

  if (SerialBT.available()) 
  {
    incomingChar = SerialBT.read();
    if (incomingChar != '\n') {
      messageIn += String(incomingChar);
    } else {
      btreceive();
    }
  }

  if (pressed)
  {
    btsend_pos();
  }

  if (onPress) 
  {
    btsend_state(-1);
  }

  if (onRelease)
  {
    btsend_state(-2);
  }
}

void btreceive () 
{
  // Add logic based on messageIn
  messageIn = "";
}

void btsend_pos () 
{
  String xM = String(t_x);
  String yM = String(t_y);
  messageOut = String(0 + "_" + xM + "_" + yM);
  btsend();
}

void btsend_state(uint8_t state)
{
  messageOut = String(state);
  btsend();
}

void btsend_message (String message) {
  messageOut = String (1 + "_" + message);
  btsend();
}

void btsend () 
{
  SerialBT.println(messageOut);
  Serial.println(messageOut);
}