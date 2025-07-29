bool game_active = false;
int numLeds = 4;
int rounds = 0;
const long maxTime = 2000;
long currentTime = 2000;
unsigned long runtime;
int activeLeds[4] = {-1, -1, -1, -1};

int ledPins[] = {D4, D3, D10, D9};
int buttonPins[] = {D5, D6, D7, D8};

void setup() {
  for (int i = 0; i < numLeds; i++){
    pinMode(ledPins[i], OUTPUT);
  }

  for (int i = 0; i < numLeds; i++){
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  randomSeed(analogRead(0));

}

bool isValidSpot(int value) {
  for (int i = 0; i < numLeds; i++){
    if (activeLeds[i] == value) return false;
  }
  return true;
}

void winner() {
  for (int i = 0; i < numLeds; i++) {
    for (int j = 0; j < numLeds; j++) {
      digitalWrite(ledPins[j], LOW);
    }
    delay(200);

    for (int j = 0; j < numLeds; j++) {
      digitalWrite(ledPins[j], HIGH);
    }
    delay(200);
  }
  game_active = false;
  currentTime = maxTime;
  rounds = 0;
  delay(1000);
}

void gameOver() {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < numLeds; j++) {
      digitalWrite(ledPins[j], LOW);
    }
    delay(100);
    for (int j = 0; j < numLeds; j++) {
      digitalWrite(ledPins[j], HIGH);
    }
    delay(100);
  }
  game_active = false;
  currentTime = maxTime;
  rounds = 0;
  delay(1000);
}


void loop() {
  if (!game_active){
    bool allButtonsReleased = true;
    for (int i = 0; i < numLeds; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        digitalWrite(ledPins[i], LOW);
        allButtonsReleased = false;
        break;
      }
    }
    
    if (allButtonsReleased) {
      game_active = true;

      int ledsActive = 1;
      if (currentTime <= 1500) ledsActive = 2;
      if (currentTime <= 1000) ledsActive = 3;
      if (currentTime <= 750) ledsActive = 4;

      for (int i = 0; i < numLeds; i++) activeLeds[i] = -1;

      for (int i = 0; i < ledsActive; i++){
        digitalWrite(ledPins[i], HIGH);
        int randomValue;
        do {
          randomValue = random(numLeds);
        } while (isValidSpot(randomValue));

        activeLeds[i] = randomValue;
        digitalWrite(ledPins[randomValue], LOW);
      }

      runtime = millis();
    }

  } else {
    if (millis() - runtime >= currentTime){
      for (int i = 0; i < numLeds; i++) {
        if (activeLeds[i] != -1) {
          digitalWrite(ledPins[activeLeds[i]], HIGH);
        }
      }

      game_active = false;

    } else {
      for (int i = 0; i < numLeds; i++) {
        if (activeLeds[i] == -1 && digitalRead(buttonPins[i]) == LOW) {
          gameOver();
          return;
        }
      }
      bool allButtonsPressed = true;

      for (int i = 0; i < numLeds; i++){
        if (activeLeds[i] != -1 && digitalRead(buttonPins[activeLeds[i]]) == HIGH) {
          allButtonsPressed = false;
        }
      }

      if (allButtonsPressed) {
        rounds ++;
        currentTime -= (rounds * 100);
        if (currentTime < 100) {
          currentTime = 100;
          winner();
        } else {
          game_active = false;

          for (int i = 0; i < numLeds; i++) {
            digitalWrite(ledPins[i], LOW);
          }

          delay(200);

          for (int i = 0; i < numLeds; i++) {
            digitalWrite(ledPins[i], HIGH);
          }

          delay(1000);
        }
      }
    }
  }
}
