#include <LiquidCrystal.h>

// ======================================================
// LCD
// ======================================================
// RS -> 12
// E  -> 11
// D4 -> 5
// D5 -> 4
// D6 -> 3
// D7 -> 2

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// GAME

int score = 0;
bool allow_jump = false;

// Prevent seaweed from spawning too close together
int obstacleCooldown = 0;


// ======================================================
// PINS
// ======================================================

// Button connects between pin 8 and GND
const int jumpButton = 8;

// Buzzer
const int buzzer = 7;


// ======================================================
// CUSTOM CHARACTERS
// ======================================================

// Character 0
// LEFT side of fish:
byte fish_tail[8] = {
  B10000,
  B11000,
  B01100,
  B00111,
  B00111,
  B01100,
  B11000,
  B10000
};


// Character 1
// RIGHT side of fish:
byte fish_body[8] = {
  B00100,
  B01110,
  B11111,
  B11011,
  B11111,
  B11111,
  B01110,
  B00100
};


// Character 2
// Small seaweed
byte seaweed_small[8] = {
  B00100,
  B10100,
  B10101,
  B01101,
  B00110,
  B00100,
  B00100,
  B00100
};


// Character 3
// Large seaweed
byte seaweed_big[8] = {
  B10100,
  B10101,
  B01101,
  B00110,
  B10100,
  B11101,
  B01110,
  B00100
};

// GAME WORLD

// 32 characters:
// 0-15  = top row
// 16-31 = bottom row
//
// Fish begins on bottom row.
//
// Character:
// 0 = fish tail
// 1 = fish body
// 2 = small seaweed
// 3 = big seaweed
// 32 = blank

char world[] = {

  // TOP ROW
  32, 32, 32, 32,
  32, 32, 32, 83,
  99, 111, 114, 101,
  58, 32, 32, 32,

  // BOTTOM ROW
  0, 1, 32, 32,
  32, 32, 32, 32,
  32, 32, 32, 32,
  32, 32, 32, 32
};


// ======================================================
// BUTTON
// ======================================================

bool get_button() {
  return digitalRead(jumpButton);
}


// ======================================================
// SCROLL WORLD
// ======================================================

uint8_t scroll_world() {

  // GAME SPEED
  delay(150);


  // --------------------------------------------------
  // CREATE NEW SEAWEED
  // --------------------------------------------------

  // If we're in the cooldown period, force an empty
  // space so seaweed cannot appear right next to
  // another piece of seaweed.

  if (obstacleCooldown > 0) {

    world[31] = 32;

    obstacleCooldown--;
  }

  else {

    // Same general random obstacle idea as original
    char random_object = random(2, 35);

    if (random_object < 4) {

      // 2 = small seaweed
      // 3 = large seaweed

      world[31] = random_object;

      // At least 5 empty spaces before another
      // piece of seaweed can appear
      obstacleCooldown = 5;
    }

    else {

      world[31] = 32;
    }
  }


  // --------------------------------------------------
  // MOVE SEAWEED LEFT
  // --------------------------------------------------

  for (int i = 16; i < 32; i++) {

    // Only move seaweed
    if (world[i] == 2 || world[i] == 3) {

      char prev;

      if (i < 31) {
        prev = world[i + 1];
      }
      else {
        prev = 32;
      }


      // ------------------------------------------------
      // COLLISION
      // ------------------------------------------------

      // Fish occupies TWO blocks:
      //
      // bottom row positions:
      //
      // world[16] = tail
      // world[17] = body/head
      //
      // If seaweed reaches either position while
      // fish is down, game over.

      if (i == 18) {

        if (world[16] == 0 ||
            world[17] == 1) {

          return 1;
        }
      }


      // Scroll seaweed left
      world[i - 1] = world[i];

      world[i] = prev;
    }
  }


  // Keep score area clean
  world[15] = 32;


  return 0;
}


// ======================================================
// UPDATE WORLD
// ======================================================

void update_world() {

  int game_over = scroll_world();


  // ==================================================
  // WIN
  // ==================================================

  if (score == 999) {

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("    YOU WIN!    ");

    lcd.setCursor(0, 1);

    // Fish
    lcd.write(byte(0));
    lcd.write(byte(1));

    lcd.print("   ");

    // Seaweed
    lcd.write(byte(2));
    lcd.write(byte(3));
    lcd.write(byte(2));
    lcd.write(byte(3));

    while (1);
  }


  // ==================================================
  // GAME OVER
  // ==================================================

  if (game_over) {

    tone(buzzer, 500, 500);

    lcd.clear();

    lcd.setCursor(2, 0);
    lcd.print("GAME OVER!");

    lcd.setCursor(3, 1);
    lcd.print("Score: ");
    lcd.print(score);


    score = 0;

    delay(1500);


    lcd.clear();

    lcd.setCursor(2, 0);
    lcd.print("Press Start!");

    lcd.setCursor(0, 1);

    // Display fish
    lcd.write(byte(0));
    lcd.write(byte(1));


    // Wait for button
    while (get_button() == HIGH) {
      ;
    }


    // Reset world

    for (int i = 0; i < 32; i++) {
      world[i] = 32;
    }


    // Put "Score:" back
    world[7]  = 'S';
    world[8]  = 'c';
    world[9]  = 'o';
    world[10] = 'r';
    world[11] = 'e';
    world[12] = ':';


    // Put fish back
    world[16] = 0;
    world[17] = 1;


    obstacleCooldown = 0;
  }

  score++;


  lcd.setCursor(13, 0);

  lcd.print("   ");

  lcd.setCursor(13, 0);

  lcd.print(score);

  lcd.setCursor(0, 0);


  for (int i = 0; i < 32; i++) {

    if (i == 16) {

      lcd.setCursor(0, 1);
    }


    // Don't overwrite score
    if (i < 13 || i > 15) {

      lcd.write(byte(world[i]));
    }
  }
}


// ======================================================
// SETUP
// ======================================================

void setup() {

  // Button uses Arduino's internal pull-up resistor
  pinMode(jumpButton, INPUT_PULLUP);

  pinMode(buzzer, OUTPUT);


  // Initialize LCD
  lcd.begin(16, 2);


  // Custom characters
  lcd.createChar(0, fish_tail);
  lcd.createChar(1, fish_body);

  lcd.createChar(2, seaweed_small);
  lcd.createChar(3, seaweed_big);


  // Randomize obstacle generation
  randomSeed(analogRead(A0));


  lcd.clear();
}


// ======================================================
// MAIN LOOP
// ======================================================

void loop() {

  lcd.clear();

  lcd.setCursor(2, 0);

  lcd.print("Press Start!");


  // Draw fish
  lcd.setCursor(0, 1);

  lcd.write(byte(0));
  lcd.write(byte(1));


  // Wait for button press
  while (get_button() == HIGH) {
    ;
  }


  // Wait for release so the same press doesn't
  // immediately trigger a jump
  while (get_button() == LOW) {
    delay(10);
  }

  // RESET GAME

  score = 0;

  obstacleCooldown = 0;


  for (int i = 0; i < 32; i++) {

    world[i] = 32;
  }


  // Score text
  world[7]  = 'S';
  world[8]  = 'c';
  world[9]  = 'o';
  world[10] = 'r';
  world[11] = 'e';
  world[12] = ':';


  // Fish starts on bottom row
  world[16] = 0;
  world[17] = 1;

  // GAME LOOP

while (true) {

  // JUMP
  if (get_button() == LOW) {

    tone(buzzer, 1000, 100);

    // Remove fish from bottom
    world[16] = 32;
    world[17] = 32;

    // Put fish on top
    world[0] = 0;
    world[1] = 1;

    // Stay in the air while seaweed moves
    for (int i = 0; i < 4; i++) {
      update_world();
    }

    // Remove fish from top
    world[0] = 32;
    world[1] = 32;

    // Put fish back on bottom
    world[16] = 0;
    world[17] = 1;

    // Wait until button is released
    // so holding it doesn't cause repeated jumps
    while (get_button() == LOW) {
      delay(10);
    }
  }

  update_world();
}
}