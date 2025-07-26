#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

int LED1 = D4;
int LED2 = D3;
int LED3 = D10;
int LED4 = D9;

int BUT1 = D5;
int BUT2 = D6;
int BUT3 = D7;
int BUT4 = D8;

byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

byte diamond[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

byte club[8] = {
  0b00000,
  0b01110,
  0b01110,
  0b11111,
  0b00100,
  0b01110,
  0b00000,
  0b00000
};

byte spade[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b01110,
  0b00000,
  0b00000
};

struct Card {
  int value;
  int suit;  
  const char* name;
};

Card deck[52] = {

  {1, 0, "A"}, {2, 0, "2"}, {3, 0, "3"}, {4, 0, "4"}, {5, 0, "5"}, {6, 0, "6"}, {7, 0, "7"},
  {8, 0, "8"}, {9, 0, "9"}, {10, 0, "10"}, {11, 0, "J"}, {12, 0, "Q"}, {13, 0, "K"},

  {1, 1, "A"}, {2, 1, "2"}, {3, 1, "3"}, {4, 1, "4"}, {5, 1, "5"}, {6, 1, "6"}, {7, 1, "7"},
  {8, 1, "8"}, {9, 1, "9"}, {10, 1, "10"}, {11, 1, "J"}, {12, 1, "Q"}, {13, 1, "K"},

  {1, 2, "A"}, {2, 2, "2"}, {3, 2, "3"}, {4, 2, "4"}, {5, 2, "5"}, {6, 2, "6"}, {7, 2, "7"},
  {8, 2, "8"}, {9, 2, "9"}, {10, 2, "10"}, {11, 2, "J"}, {12, 2, "Q"}, {13, 2, "K"},

  {1, 3, "A"}, {2, 3, "2"}, {3, 3, "3"}, {4, 3, "4"}, {5, 3, "5"}, {6, 3, "6"}, {7, 3, "7"},
  {8, 3, "8"}, {9, 3, "9"}, {10, 3, "10"}, {11, 3, "J"}, {12, 3, "Q"}, {13, 3, "K"}
};

bool used_cards[52] = {false};
bool deal = false;
int balance = 200;
int bet = 10;
bool hit = true;
int hand = 0;
int bot_hand = 0;
bool game_over = false;
int last_player_card = -1;  
int last_dealer_card = -1;  

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  lcd.createChar(0, heart);     
  lcd.createChar(1, diamond);   
  lcd.createChar(2, club);      
  lcd.createChar(3, spade);     

  pinMode(BUT1, INPUT_PULLUP);
  pinMode(BUT2, INPUT_PULLUP);
  pinMode(BUT3, INPUT_PULLUP);
  pinMode(BUT4, INPUT_PULLUP);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  randomSeed(analogRead(A0));
  Serial.println("Starting Game!!!!");

  updateDisplay();
}

void loop() {
  if (!deal) {

    if (digitalRead(BUT1) == LOW) {
      deal = true;
      hit = true;
      hand = 0;
      bot_hand = 0;
      game_over = false;

      int card1 = dealCard();
      int card2 = dealCard();
      hand += getCardValue(card1);
      hand += getCardValue(card2);
      last_player_card = card2;  

      int dealer_card1 = dealCard();
      int dealer_card2 = dealCard();
      bot_hand += getCardValue(dealer_card1);
      bot_hand += getCardValue(dealer_card2);
      last_dealer_card = dealer_card1;  

      updateDisplay();
      delay(200);
    } else if ((digitalRead(BUT2) == LOW) && (bet < balance)) {
        bet += 10;
        if (bet > balance) bet = balance;
        updateDisplay();
        delay(200);
    } else if ((digitalRead(BUT3) == LOW) && (bet > 10)) {
        bet -= 10;
        if (bet < 10) bet = 10;
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED3, HIGH);
          delay(100);
          digitalWrite(LED3, LOW);
          delay(100);
        }
        updateDisplay();
        delay(200);
    } else if (digitalRead(BUT4) == LOW) {
        bet = balance;
        digitalWrite(LED4, HIGH);
        delay(1000);
        digitalWrite(LED4, LOW);
        updateDisplay();
        delay(200);
    }
  } else {

    if (hit && !game_over) {
      if (digitalRead(BUT1) == LOW) { 
        int hit_card = dealCard();
        hand = checkAce(hit_card, hand);
        last_player_card = hit_card;  

        if (hand > 21) {
          game_over = true;
          balance -= bet;
          updateDisplay();
          showGameResult(); 
        }
        updateDisplay();
        delay(200);

      } else if (digitalRead(BUT2) == LOW) { 
        hit = false;
        playDealer();
        delay(200);

      } else if ((digitalRead(BUT3) == LOW) && (bet*2 <= balance)) { 
        bet *= 2;
        int hit_card = dealCard();
        hand = checkAce(hit_card, hand);
        last_player_card = hit_card;  

        if (hand > 21) {
          game_over = true;
          balance -= bet;
          updateDisplay();
          showGameResult(); 
        } else {
          hit = false;
          playDealer();
        }
        delay(200);

      } else if (digitalRead(BUT4) == LOW) { 
        deal = false;
        balance -= bet/2;
        resetDeck();
        updateDisplay();
        delay(200);
      }
    } else {

      if (digitalRead(BUT1) == LOW) {
        deal = false;
        resetDeck();
        updateDisplay();
        delay(200);
      }
    }
  }
}

int dealCard() {
  int index;
  do {
    index = random(0, 52);
  } while (used_cards[index]);
  used_cards[index] = true;
  return index;  
}

int getCardValue(int cardIndex) {
  return deck[cardIndex].value;
}

void displayCard(int cardIndex, int row, int col) {
  if (cardIndex == -1) return;

  lcd.setCursor(col, row);
  lcd.print(deck[cardIndex].name);
  lcd.write(deck[cardIndex].suit);  
}

int checkAce(int cardIndex, int hand) {
  int cardValue = getCardValue(cardIndex);
  if (cardValue == 1) { 
    if (hand + 11 <= 21) {
      hand += 11; 
    } else {
      hand += 1;  
    }
  } else {
    hand += cardValue;
  }
  return hand;
}

void resetDeck() {
  for (int i = 0; i < 52; i++) {
    used_cards[i] = false;
  }
}

void playDealer() {
  while (bot_hand < 17) {
    int card = dealCard();
    int cardValue = getCardValue(card);
    if (cardValue == 1) { 
      if (bot_hand + 11 <= 21) {
        bot_hand += 11;
      } else {
        bot_hand += 1;
      }
    } else {
      bot_hand += cardValue;
    }
    last_dealer_card = card;  
  }

  if (bot_hand > 21 || (hand <= 21 && hand > bot_hand)) {
    balance += bet; 
  } else if (hand == bot_hand && hand <= 21) {

  } else {
    balance -= bet; 
  }

  game_over = true;
  updateDisplay();
  showGameResult(); 
}

void updateDisplay() {
  lcd.clear();

  if (!deal) {
    lcd.setCursor(0, 0);
    lcd.print("Balance: $");
    lcd.print(balance);

    lcd.setCursor(0, 1);
    lcd.print("Bet: $");
    lcd.print(bet);
    lcd.print(" [DEAL]");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("You: ");
    lcd.print(hand);

    if (last_player_card != -1) {
      displayCard(last_player_card, 0, 9);
    }
    if (!game_over && hit) {
      lcd.setCursor(12, 0);
      lcd.print("H/S");
    }

    lcd.setCursor(0, 1);

    if (game_over || !hit) {
      lcd.print("Dealer: ");
      lcd.print(bot_hand);
      if (last_dealer_card != -1) {
        displayCard(last_dealer_card, 1, 11);
      }

      if (game_over) {
        lcd.setCursor(14, 1);
        if ((hand <= 21 && bot_hand > 21) || (hand <= 21 && hand > bot_hand)) {
          lcd.print("W!");
        } else if (hand == bot_hand && hand <= 21) {
          lcd.print("P");
        } else {
          lcd.print("L!");
        }
      }
    } else {
      lcd.print("Dealer: ?");
      if (last_dealer_card != -1) {
        displayCard(last_dealer_card, 1, 10);
      }
    }
  }
}

void showGameResult() {
  if ((hand <= 21 && bot_hand > 21) || (hand <= 21 && hand > bot_hand)) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED1, HIGH);
      delay(150);
      digitalWrite(LED2, HIGH);
      delay(150);
      digitalWrite(LED3, HIGH);
      delay(150);
      digitalWrite(LED4, HIGH);
      delay(150);
      digitalWrite(LED1, LOW);
      delay(100);
      digitalWrite(LED2, LOW);
      delay(100);
      digitalWrite(LED3, LOW);
      delay(100);
      digitalWrite(LED4, LOW);
      delay(100);
    }
  } else if (hand == bot_hand && hand <= 21) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);
      digitalWrite(LED3, HIGH);
      digitalWrite(LED4, HIGH);
      delay(300);
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
      digitalWrite(LED4, LOW);
      delay(300);
    }
  } else {
    for (int i = 0; i < 8; i++) {
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);
      digitalWrite(LED3, HIGH);
      digitalWrite(LED4, HIGH);
      delay(100);
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
      digitalWrite(LED4, LOW);
      delay(100);
    }
  }
}