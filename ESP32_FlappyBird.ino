#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
//#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ======= GAME SETTINGS =======
#define BUTTON_PIN 35
#define GRAVITY 0.6     // trọng lực
#define FLAP_STRENGTH -4    // độ cao nhảy
#define PIPE_SPEED 2
#define PIPE_GAP 25
#define PIPE_WIDTH 15
#define ANIM_INTERVAL 100 // ms đổi frame animation

unsigned long lastFlapTime = 0;
#define FLAP_DELAY 150 // ms

// ======= GAME VARIABLES =======
float birdY, birdVel;
int pipeX, pipeGapY;
int score;
bool gameOver = false;
bool buttonPressed = false;
bool buttonPrevState = false;
unsigned long lastAnimTime = 0;
bool animFrame = false;

// ======= SPRITES =======
// Chim cánh lên (8x8)
static const unsigned char PROGMEM birdUp[] = {
  0b00011000,
  0b00111100,
  0b01111110,
  0b11111111,
  0b11111111,
  0b01110110,
  0b00100000,
  0b00000000
};

// Chim cánh xuống (8x8)
static const unsigned char PROGMEM birdDown[] = {
  0b00011000,
  0b00111100,
  0b01111110,
  0b11111111,
  0b11111111,
  0b01111110,
  0b00011100,
  0b00001000
};

// ======= FUNCTIONS =======
void resetGame() {
  birdY = SCREEN_HEIGHT / 2;
  birdVel = 0;
  pipeX = SCREEN_WIDTH;
  pipeGapY = random(10, SCREEN_HEIGHT - PIPE_GAP - 10);
  score = 0;
  gameOver = false;
}

bool checkCollision() {
  if (birdY > SCREEN_HEIGHT - 8 || birdY < 0)
    return true;

  if (pipeX < 20 && pipeX + PIPE_WIDTH > 10) {
    if (birdY < pipeGapY || birdY + 8 > pipeGapY + PIPE_GAP)
      return true;
  }
  return false;
}

void drawBird() {
  const unsigned char* sprite = animFrame ? birdUp : birdDown;
  display.drawBitmap(10, (int)birdY, sprite, 8, 8, SH110X_WHITE);
}

void drawPipe() {
  display.fillRect(pipeX, 0, PIPE_WIDTH, pipeGapY, SH110X_WHITE);
  display.fillRect(pipeX, pipeGapY + PIPE_GAP, PIPE_WIDTH,
                   SCREEN_HEIGHT - pipeGapY - PIPE_GAP, SH110X_WHITE);
}

void drawScore() {
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("Score: ");
  display.print(score);
}

void setup() {
    Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  randomSeed(millis());

  if (!display.begin(0x3c, true)) {
    for (;;);
  }

  display.clearDisplay();
  display.display();
  resetGame();
}

void loop() {
  buttonPressed = !digitalRead(BUTTON_PIN);

  if (buttonPressed && !buttonPrevState && millis() - lastFlapTime > FLAP_DELAY) {
    birdVel = FLAP_STRENGTH;
    lastFlapTime = millis();
    }

  if (!gameOver) {
    // === Animation timing ===
    if (millis() - lastAnimTime > ANIM_INTERVAL) {
      animFrame = !animFrame;
      lastAnimTime = millis();
    }

    // === Input ===
    if (buttonPressed && !buttonPrevState) {
      birdVel = FLAP_STRENGTH;
    }

    // === Physics ===
    birdVel += GRAVITY;
    birdY += birdVel;

    // === Pipe movement ===
    pipeX -= PIPE_SPEED;
    if (pipeX + PIPE_WIDTH < 0) {
      pipeX = SCREEN_WIDTH;
      pipeGapY = random(10, SCREEN_HEIGHT - PIPE_GAP - 10);
      score++;
    }

    // === Collision ===
    if (checkCollision()) {
      gameOver = true;
    }

    // === Drawing ===
    display.clearDisplay();
    drawPipe();
    drawBird();
    drawScore();
    display.display();

  } else {
    // === Game Over ===
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(15, 15);
    display.print("GAME OVER");
    display.setTextSize(1);
    display.setCursor(35, 45);
    display.print("Score: ");
    display.print(score);
    display.setCursor(0, 56);
    display.print("Press button to play");
    display.display();

    if (buttonPressed && !buttonPrevState) {
        Serial.println("Button reset game");
      delay(200);
      resetGame();
    }
  }

  buttonPrevState = buttonPressed;
  delay(30);
}
