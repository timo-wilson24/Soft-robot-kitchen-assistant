//////////////////////////////////////////////
//       Arduino Rotary Encoder Menu        //
//                 v1.0                     //
//           http://www.educ8s.tv           //
/////////////////////////////////////////////

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

// ------------------------------- pin mapping -------------------------------//
Adafruit_PCD8544 display = Adafruit_PCD8544(13, 12, 8, 7, 6);
// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define SW 4

// Mosfet PWM outputs
#define SOL1 5  //solenoid
#define SOL2 7
#define PUMP1 8  //air pump 1
#define PUMP2 9  //air pump 2

// Pressure Sensor
#define PRS1_LOG A0  // Honeywell basic sensor
#define PRS2_LOG A5  // ditto + 1

// Force Sensor
#define FSR1 A3  // Is also connected to GND and 10K ohm resistor
#define FSR2 A4

// ---------------------------- sensor variables -----------------------------//
// Force Sensitive Resistor
const int FSRmax = 50;      // the point where there is nothing touching the resistor
const int FSRminForce = 9;  // The point where the force sensor 'bottoms out'
int FSRval1;
int currentFSR1;
int lastFSR1;
int mappedFSR1;
int FSRval2;
int currentFSR2;
int lastFSR2;
int mappedFSR2;

// Pressure Sensor
const int PRSmax = 50;  // the point where there is nothing touching the resistor
int PRSVal1;
int currentPRS1;
int lastPRS1 = 0;
int mappedPRS1;
int PRSVal2;
int currentPRS2;
int lastPRS2 = 0;
int mappedPRS2;
// what the raw value from the sensor will be added to to give the final value.
const int PRS1_intercept = -165;
const int PRS2_intercept = -165;
// ---------------------------- display variables ----------------------------//
int menuitem = 1;
int frame = 1;
int page = 1;
int lastMenuItem = 1;

//Main Menu items
String menuItem1 = "Hardness Mode";
String menuItem2 = "Device Select";
String menuItem3 = "Sensors";
String menuItem4 = "Manual";
String menuItem5 = "Backlight: ON";
String menuItem6 = "Reset";
String menuItem7 = "Power OFF";
// Sub/Page 3 menu items
String subMenuItem3a = "Force Sensors";
String subMenuItem3b = "Pressure:";


// Menu 1: Hardness Mode
String HardnessMode[3] = { "Onion", "Carrot", "Tomato" };
int selectedHardnessMode = 0;

// Menu 2: Device Select
String Device[2] = { "Finger", "Clamp" };
int selectedDevice = 0;

// Menu 3: Sensors
String sensorVar1 = "Finger";
String sensorVar2 = "Clamp";

// Menu 4+:
boolean backlight = true;
boolean powerVar = false;
const int contrast = 60;

boolean up = false;
boolean down = false;
boolean middle = false;

ClickEncoder *encoder;
int16_t last, value;


// ---------------------------- void setup ---------------------------- //
void setup() {
  // Solnoid Output pins for MOSFET baord
  pinMode(SOL1, OUTPUT);
  pinMode(SOL2, OUTPUT);
  // Pump Outputs for PWM
  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);

  // display set up
  pinMode(9, OUTPUT);
  turnBacklightOn();
  display.begin();
  display.clearDisplay();
  display.setContrast(contrast);
  display.display();

  // encoder set up
  encoder = new ClickEncoder(CLK, DT, SW);
  encoder->setAccelerationEnabled(false);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  last = encoder->getValue();
  display.setRotation(2);

  // Setup Serial Monitor
  Serial.begin(9600);
}

// ---------------------------- void loop ---------------------------- //
void loop() {

  drawMenu();

  readRotaryEncoder();
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Clicked:
        middle = true;
        break;
    }
  }
  MenuCode();  // All display, menu settings etc.
  ForceSensor();
  PressureSensor();

  Serial.print("PRS1:");
  Serial.print(PRSVal1);
  Serial.print(",");
  Serial.print("PRS2:");
  Serial.print(PRSVal2);
  Serial.println(",");
}

// ------------------------ Sensor Value Gathering Code ---------------------- //

void PressureSensor() {
  currentPRS1 = analogRead(PRS1_LOG);
  if (currentPRS1 != lastPRS1) {
    PRSVal1 = ((analogRead(PRS1_LOG) * 1.6) + PRS1_intercept) * 0.1;
    delay(10);
  }
  lastPRS1 = currentPRS1;

  currentPRS2 = analogRead(PRS2_LOG);
  if (currentPRS2 != lastPRS2) {
    PRSVal2 = ((analogRead(PRS2_LOG) * 1.6) + PRS2_intercept) * 0.1;
    delay(10);
  }
  lastPRS2 = currentPRS2;
  delay(100);
}

void ForceSensor() {
  currentFSR1 = analogRead(FSR1);
  if (currentFSR1 != lastFSR1) {
    FSRval1 = map(analogRead(FSR1), FSRminForce, 1024, 0, (FSRmax + 1));
    delay(10);
    //Serial.print("Force Sensor Value:");
    // Serial.print(FSRval1);
    //Serial.println(",");
  }
  lastFSR1 = currentFSR1;
  delay(1);

  currentFSR2 = analogRead(FSR2);
  if (currentFSR2 != lastFSR2) {
    FSRval2 = map(analogRead(FSR2), FSRminForce, 1024, 0, (FSRmax + 1));
    delay(10);
    // Serial.print("Force Sensor Value2:");
    // Serial.print(FSRval2);
    // Serial.println(",");
  }
  lastFSR2 = currentFSR2;
  delay(1);
}

// -------------------------- All Menu/Display Code -------------------------- //
void MenuCode() {

  if (up && page == 1) {

    up = false;
    if (menuitem == 2 && frame == 2) {
      frame--;
    }
    if (menuitem == 3 && frame == 3) {
      frame--;
    }
    if (menuitem == 4 && frame == 4) {
      frame--;
    }
    if (menuitem == 5 && frame == 5) {
      frame--;
    }
    lastMenuItem = menuitem;
    menuitem--;
    if (menuitem == 0) {
      menuitem = 1;
    }
  } else if (up && page == 2 && menuitem == 1) {
    up = false;
    selectedHardnessMode--;
    if (selectedHardnessMode == -1) {
      selectedHardnessMode = 2;
    }
  } else if (up && page == 2 && menuitem == 2) {
    up = false;
    selectedDevice--;
    if (selectedDevice == -1) {
      selectedDevice = 1;
    }
  } else if (up && page == 2 && menuitem == 3) {
    up = false;
    // insert code
  } else if (up && page == 2 && menuitem == 4) {
    up = false;
    //insert code to do
  }

  if (down && page == 1)  //We have turned the Rotary Encoder Clockwise
  {

    down = false;
    if (menuitem == 3 && lastMenuItem == 2) {
      frame++;
    } else if (menuitem == 4 && lastMenuItem == 3) {
      frame++;
    } else if (menuitem == 5 && lastMenuItem == 4 && frame != 5) {
      frame++;
    } else if (menuitem == 5 && lastMenuItem == 4 && frame != 5) {
      frame++;
    } else if (menuitem == 6 && lastMenuItem == 5 && frame != 5) {
      frame++;
    }
    lastMenuItem = menuitem;
    menuitem++;
    if (menuitem == 8) {
      menuitem--;
    }

  } else if (down && page == 2 && menuitem == 1) {
    down = false;
    selectedHardnessMode++;
    if (selectedHardnessMode == 3) {
      selectedHardnessMode = 0;
    }
  } else if (down && page == 2 && menuitem == 2) {
    down = false;
    selectedDevice++;
    if (selectedDevice == 2) {
      selectedDevice = 0;
    }
  } else if (down && page == 2 && menuitem == 3) {
    down = false;
    // selectedLanguage++;
    // if(selectedLanguage == 3)
    // {
    //   selectedLanguage = 0;
    // }
  } else if (down && page == 2 && menuitem == 5) {
    down = false;
  }

  if (middle)  //Middle Button is Pressed
  {
    middle = false;

    if (page == 1 && menuitem == 5)  // Backlight Control
    {
      if (backlight) {
        backlight = false;
        menuItem5 = "Backlight OFF";
        turnBacklightOff();
      } else {
        backlight = true;
        menuItem5 = "Backlight: ON";
        turnBacklightOn();
      }
    }
    if (page == 2 && menuitem == 3) {
      page = 3;
    } else if (page == 3 && menuitem == 3) {
      page = 1;
    } else if (page == 1 && menuitem == 3) {
      page = 2;
    }
    if (page == 1 && menuitem == 6)  // Reset
    {
      resetDefaults();
    }
    if (page == 1 && menuitem == 7)  // PowerOff sequence
    {
      powerOff();
    }

    else if (page == 1 && menuitem <= 4 && menuitem != 3) {
      page = 2;
    } else if (page == 2 && menuitem != 3) {
      page = 1;
    }
  }
}
void drawMenu() {

  if (page == 1) {
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print("MAIN MENU");
    display.drawFastHLine(0, 10, 83, BLACK);

    if (menuitem == 1 && frame == 1) {
      displayMenuItem(menuItem1, 15, true);
      displayMenuItem(menuItem2, 25, false);
      displayMenuItem(menuItem3, 35, false);
    } else if (menuitem == 2 && frame == 1) {
      displayMenuItem(menuItem1, 15, false);
      displayMenuItem(menuItem2, 25, true);
      displayMenuItem(menuItem3, 35, false);
    } else if (menuitem == 3 && frame == 1) {
      displayMenuItem(menuItem1, 15, false);
      displayMenuItem(menuItem2, 25, false);
      displayMenuItem(menuItem3, 35, true);
    }

    else if (menuitem == 2 && frame == 2) {
      displayMenuItem(menuItem2, 15, true);
      displayMenuItem(menuItem3, 25, false);
      displayMenuItem(menuItem4, 35, false);
    } else if (menuitem == 4 && frame == 2) {
      displayMenuItem(menuItem2, 15, false);
      displayMenuItem(menuItem3, 25, false);
      displayMenuItem(menuItem4, 35, true);
    } else if (menuitem == 3 && frame == 2) {
      displayMenuItem(menuItem2, 15, false);
      displayMenuItem(menuItem3, 25, true);
      displayMenuItem(menuItem4, 35, false);
    } else if (menuitem == 2 && frame == 2) {
      displayMenuItem(menuItem2, 15, true);
      displayMenuItem(menuItem3, 25, false);
      displayMenuItem(menuItem4, 35, false);
    }

    else if (menuitem == 3 && frame == 3) {
      displayMenuItem(menuItem3, 15, true);
      displayMenuItem(menuItem4, 25, false);
      displayMenuItem(menuItem5, 35, false);
    } else if (menuitem == 4 && frame == 3) {
      displayMenuItem(menuItem3, 15, false);
      displayMenuItem(menuItem4, 25, true);
      displayMenuItem(menuItem5, 35, false);
    } else if (menuitem == 5 && frame == 3) {
      displayMenuItem(menuItem3, 15, false);
      displayMenuItem(menuItem4, 25, false);
      displayMenuItem(menuItem5, 35, true);
    }

    else if (menuitem == 4 && frame == 4) {
      displayMenuItem(menuItem4, 15, true);
      displayMenuItem(menuItem5, 25, false);
      displayMenuItem(menuItem6, 35, false);
    } else if (menuitem == 5 && frame == 4) {
      displayMenuItem(menuItem4, 15, false);
      displayMenuItem(menuItem5, 25, true);
      displayMenuItem(menuItem6, 35, false);
    } else if (menuitem == 6 && frame == 4) {
      displayMenuItem(menuItem4, 15, false);
      displayMenuItem(menuItem5, 25, false);
      displayMenuItem(menuItem6, 35, true);
    }

    else if (menuitem == 5 && frame == 5) {
      displayMenuItem(menuItem5, 15, true);
      displayMenuItem(menuItem6, 25, false);
      displayMenuItem(menuItem7, 35, false);
    } else if (menuitem == 6 && frame == 5) {
      displayMenuItem(menuItem5, 15, false);
      displayMenuItem(menuItem6, 25, true);
      displayMenuItem(menuItem7, 35, false);
    } else if (menuitem == 7 && frame == 5) {
      displayMenuItem(menuItem5, 15, false);
      displayMenuItem(menuItem6, 25, false);
      displayMenuItem(menuItem7, 35, true);
    }

    display.display();
  } else if (page == 2 && menuitem == 1) {
    displayStringMenuPage(menuItem1, HardnessMode[selectedHardnessMode]);
  }

  else if (page == 2 && menuitem == 2) {
    displayStringMenuPage(menuItem2, Device[selectedDevice]);
  } else if (page == 2 && menuitem == 3) {
    displayTwoIntMenuPage(subMenuItem3a, FSRval1, FSRval2, sensorVar1, sensorVar2);

  } else if (page == 3 && menuitem == 3) {
    displayTwoIntMenuPage(subMenuItem3b, PRSVal1, PRSVal2, sensorVar1, sensorVar2);
  }

  else if (page == 2 && menuitem == 4) {
    //displayStringMenuPage(menuItem4, difficulty[selectedDifficulty]);
  } else if (page == 2 && menuitem == 5) {
    //displayStringMenuPage(menuItem5
  }
}


void resetDefaults() {
  selectedHardnessMode = 0;
  selectedDevice = 0;
  backlight = true;
  powerVar = false;
  backlight = true;
  menuItem5 = "Backlight: ON";
}

void powerOff() {
  turnBacklightOff();
  //have systems here like turn off all pumps, have solenoids open
  powerVar = true;
}

void turnBacklightOn() {
  digitalWrite(9, HIGH);
}

void turnBacklightOff() {
  digitalWrite(9, LOW);
}

void displayTwoIntMenuPage(String menuItem, int value1, int value2, String Var1, String Var2) {
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(3, 0);
  display.print(menuItem);
  display.drawFastHLine(0, 10, 83, BLACK);
  display.setTextSize(0.8);
  display.setCursor(0, 15);
  display.print(Var1);
  display.setCursor(45, 15);
  display.print(Var2);
  display.drawFastVLine(41, 12, 50, BLACK);
  display.setTextSize(2);
  display.setCursor(4, 27);
  display.print(value1);
  display.setCursor(46, 27);
  display.print(value2);
  display.display();
}
void displayOneIntMenuPage(String menuItem, int value) {
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(5, 0);
  display.print(menuItem);
  display.drawFastHLine(0, 10, 83, BLACK);
  display.setCursor(5, 15);
  display.print("Value");
  display.setTextSize(2);
  display.setCursor(5, 25);
  display.print(value);
  display.setTextSize(2);
  display.display();
}

void displayStringMenuPage(String menuItem, String value) {
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(3, 0);
  display.print(menuItem);
  display.drawFastHLine(0, 10, 83, BLACK);
  // display.setCursor(5, 15);
  // display.print("Selection");
  display.setTextSize(2);
  display.setCursor(5, 15);
  display.print(value);
  display.setTextSize(2);
  display.display();
}

void displayMenuItem(String item, int position, boolean selected) {
  if (selected) {
    display.setTextColor(WHITE, BLACK);
  } else {
    display.setTextColor(BLACK, WHITE);
  }
  display.setCursor(0, position);
  display.print(">" + item);
}

void timerIsr() {
  encoder->service();
}

void readRotaryEncoder() {
  value += encoder->getValue();

  if (value / 2 > last) {
    last = value / 2;
    down = true;
    delay(150);
  } else if (value / 2 < last) {
    last = value / 2;
    up = true;
    delay(150);
  }
}
