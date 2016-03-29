const int LED_PIN = 1; // Digispark Model A

// on digispark for analogRead the pin and the port are different numbers, so to read Pin 2 you have to set pinMode(2, INPUT) but do analogRead(1).
const int POTI_1_PIN = 5;
const int POTI_1_PORT = 0;
const int POTI_2_PIN = 2;
const int POTI_2_PORT = 1;
const int BUTTON_PIN = 3;

const int RED_PIN = 0;
const int GREEN_PIN = 1;
const int BLUE_PIN = 4;

const int MAX_COLOR = 255;
const int SPEED_DIVISOR = 3;

/* 
 * the states of the lamp. The button switches between them 
 */
const int FIXED = 0;
const int COLOR_FADE = 1;
const int BRIGHTNESS_FADE = 2;
const int WHITE_DIMM = 3;
const int MAX_STATE = WHITE_DIMM;

int currentState = 1;

boolean wasButtonPressed = false;

/*
 * Color Fade
 */
int currentFadeColor = RED_PIN;
int currentFadeValue = 0;
const int COUNT_UP = 0;
const int COUNT_DOWN = 1;
int currentDirection = COUNT_UP;


void setup() {
  pinMode(POTI_1_PIN, INPUT);
  pinMode(POTI_2_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
 
  pinMode(RED_PIN,    OUTPUT);
  pinMode(GREEN_PIN,  OUTPUT);
  pinMode(BLUE_PIN,   OUTPUT);

  rgbWrite(0, 0, 0);
}

void loop() {
  if(isButtonPressed()) {
      blinki();
      if(currentState == MAX_STATE) {
        currentState = 0;
      } else {
        currentState += 1;
      }
  }

  int potiOneVal = analogRead(POTI_1_PORT);
  int potiTwoVal = analogRead(POTI_2_PORT);
  //int potiTwoVal = 0;

  if(currentState == FIXED) {
    int brightness = slimPotiValToBrightness(potiTwoVal);
    regulateFixedColor(potiOneVal, brightness);
  } else if(currentState == COLOR_FADE) {
    regulateColorFade(potiOneVal, potiTwoVal);
  } else if(currentState == BRIGHTNESS_FADE) {
    regulateBrightnessFade(potiOneVal, potiTwoVal);
  } else if(currentState == WHITE_DIMM) {
    regulateWhiteness(potiOneVal);
  } else {
    currentState = 0;
  }
}

void regulateWhiteness(int brightness) {
  int pwmVal = slimPotiValToBrightness(brightness);
  rgbWrite(pwmVal, pwmVal, pwmVal);
}

int slimPotiValToBrightness(int brightnessVal) {
  // vals are between 0-1023, rgb can be between 0-255
  int brightness = 0;
  if(brightnessVal >= 4) {
    brightness = abs(brightnessVal / 4);
  }
  
  if(brightness > MAX_COLOR) {
    brightness = MAX_COLOR;
  } else if(brightness < 5) {
    brightness = 0;
  }

  return brightness;
}

/*
 * @param colorVal 0-1023 beginning with blue to red to green to blue
 * @param brightnessVal between 0-255, the higher the darker
 */
void regulateFixedColor(int colorVal, int brightnessVal) {
  /* adjust color
   * on the left side of the poti is pure red (255,0,0), on the right side is green (0,255,0).
   * between is blue
   */
  if(colorVal <= 255) {
    analogWrite(RED_PIN, dimm(colorVal, brightnessVal));
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 255-brightnessVal);
  } else if(colorVal > 255 && colorVal <= 511) {
    analogWrite(RED_PIN, 255-brightnessVal);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, dimm(511-colorVal, brightnessVal));
  } else if(colorVal > 511 && colorVal <= 767) {
    analogWrite(RED_PIN, 255-brightnessVal);
    analogWrite(GREEN_PIN, dimm(colorVal-511, brightnessVal));
    analogWrite(BLUE_PIN, 0);
  } else if(colorVal > 767 && colorVal <= 1023) {
    analogWrite(RED_PIN, dimm(1023-colorVal, brightnessVal));
    analogWrite(GREEN_PIN, 255-brightnessVal);
    analogWrite(BLUE_PIN, 0);
  }
}

int dimm(int color, int brightness) {
  int ret = color - brightness;
  if(ret < 0) {
    return 0;
  }

  return ret;
}

void regulateColorFade(int speedVal, int delayBetweenColors) {
  int speedo = calcSpeed(speedVal);
  int delayVal = delayBetweenColors;
  if(delayBetweenColors < 1) {
    delayVal = 0;
  }
  fadeColors(speedo, delayVal);
}



int calcSpeed(int speedVal) {
  // speedVal between 0-1024, but we need 0-100
  float speedi = 0;
  if(speedVal >= SPEED_DIVISOR) {
    speedi = speedVal / SPEED_DIVISOR;
  }
  return abs(speedi);
}

void regulateBrightnessFade(int colorVal, int speedVal) {
  countUpOrDown();
  regulateFixedColor(colorVal, currentFadeValue);
  
  int speedo = calcSpeed(speedVal);
  delay(speedo);

  if(currentFadeValue == 0) {
    currentDirection = COUNT_UP;
  } else if(currentFadeValue == MAX_COLOR) {
    currentDirection = COUNT_DOWN;
  }
}


void countUpOrDown() {
  if(currentDirection == COUNT_UP) {
    currentFadeValue++;
  } else {
    currentFadeValue--;
  }
}

void delayBetweenFadesIfNecessary(int delayTimeVal) {
  if(currentFadeValue == 0 || currentFadeValue == MAX_COLOR) {
    // delayTimeVal can be between 0-1023, that's max only a second, so let's multiply something
    int delayTime = delayTimeVal * 10;
    delay(delayTime);
  }
}

void fadeColors(int speedo, int delayTimeVal) {
  countUpOrDown();
  
  if(currentFadeColor == GREEN_PIN) {
    analogWrite(currentFadeColor, currentFadeValue);
  } else if(currentFadeColor == RED_PIN) {
    analogWrite(RED_PIN, currentFadeValue);
  } else if(currentFadeColor == BLUE_PIN) {
    analogWrite(BLUE_PIN, currentFadeValue);
  }
  delay(speedo);

  if(currentFadeColor == GREEN_PIN && currentFadeValue == MAX_COLOR) {
    currentFadeColor = RED_PIN;
    currentDirection = COUNT_DOWN;
  } else if(currentFadeColor == GREEN_PIN && currentFadeValue == 0) {
    currentFadeColor = RED_PIN;
    currentDirection = COUNT_UP;
  } else if(currentFadeColor == RED_PIN && currentFadeValue == 0) {
    currentFadeColor = BLUE_PIN;
    currentDirection = COUNT_UP;
  } else if(currentFadeColor == RED_PIN && currentFadeValue == MAX_COLOR) {
    currentFadeColor = BLUE_PIN;
    currentDirection = COUNT_DOWN;
  } else if(currentFadeColor == BLUE_PIN && currentFadeValue == MAX_COLOR) {
    currentFadeColor = GREEN_PIN;
    currentDirection = COUNT_DOWN;
  } else if(currentFadeColor == BLUE_PIN && currentFadeValue == 0) {
    currentFadeColor = GREEN_PIN;
    currentDirection = COUNT_UP;
  }

  delayBetweenFadesIfNecessary(delayTimeVal);
}

boolean isButtonPressed() {
  if(wasButtonPressed && proofButtonState(HIGH)) {
    wasButtonPressed = false;
  } else if(!wasButtonPressed && proofButtonState(LOW)) {
    wasButtonPressed = true;
    return true;
  }

  return false;
}

boolean proofButtonState(int highOrLow) {
  if(digitalRead(BUTTON_PIN) == highOrLow) {
    delay(50);
    if(digitalRead(BUTTON_PIN) == highOrLow) {
      return true;
    }
  }
  return false;
}

void blinki() {
  rgbWrite(0, 0, 255);
  delay(100);
  rgbWrite(0, 255, 0);
  delay(100);
  rgbWrite(255, 0, 0);
  delay(200);
  rgbWrite(0, 0, 0);
}

void rgbWrite(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}


