#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <LiquidCrystal.h>

#define TFT_CS 6
#define TFT_RST 9
#define TFT_DC 10
#define BUTTON_PIN A0
#define BET_INCREASE_PIN A1
#define BET_DECREASE_PIN A2
#define ADD_CREDITS_PIN A3

#include "cherry.h"
#include "apple.h"
#include "seven.h"
#include "welcome.h"
#include "bar.h"
#include "banana.h"
#include "grape.h"
#include "strawberry.h"
#include "melon.h"

LiquidCrystal lcd(8, 7, 5, 4, 3, 2);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const uint8_t* slotSymbols[] = { cherry_bitmap, apple_bitmap, seven_bitmap, bar_bitmap, banana_bitmap, grape_bitmap, strawberry_bitmap, melon_bitmap };

const int symbolWeights[] = {15, 15, 5, 5, 25, 10, 15, 10};
const int totalWeight = 75;
const int numSymbols = sizeof(slotSymbols) / sizeof(slotSymbols[0]);

int credits = 100;

const int symbolWidth = 48;
const int symbolHeight = 48;

bool buttonHeldDown = false;
bool welcomeScreenShown = false;

int bet = 10;
const int maxBet = 100;
const int minBet = 10;

const int multipliers[8][3] = {
    {20, 50, 100},
    {30, 80, 150},
    {100, 300, 1000},
    {75, 250, 750},
    {15, 30, 50},
    {50, 150, 500},
    {25, 70, 200},
    {30, 100, 300}
};

int calculatePayout(int grid[3][5]) {
    int payout = 0;

    const int winningLines[5][5][2] = {
        {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}},
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}},
        {{2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}},
        {{0, 0}, {1, 1}, {2, 2}, {-1, -1}, {-1, -1}},
        {{2, 0}, {1, 1}, {0, 2}, {-1, -1}, {-1, -1}}
    };

    for (int line = 0; line < 5; line++) {
        int firstSymbol = -1;
        int matchCount = 0;

        for (int i = 0; i < 5; i++) {
            int row= winningLines[line][i][0];
            int col= winningLines[line][i][1];

            if (row== -1 || col== -1) break;

            if (firstSymbol== -1) {
                firstSymbol= grid[row][col];
                matchCount= 1;
            } else if (grid[row][col]== firstSymbol) {
                matchCount++;
            } else {
                break;
            }
        }

        if (matchCount >= 3) {
            payout += multipliers[firstSymbol][matchCount - 3] * (bet / 10);
        }
    }

    return payout;
}

void adjustBet() {
    static unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 200;

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (digitalRead(BET_INCREASE_PIN)== LOW && bet + 10 <= credits && bet < maxBet) {
            bet += 10;
            lastDebounceTime = millis();
            displayBetAndCredits();
        }

        if (digitalRead(BET_DECREASE_PIN)== LOW && bet > minBet) {
            bet -= 10;
            lastDebounceTime = millis();
            displayBetAndCredits();
        }
    }
}

void displayBetAndCredits() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Credits: ");
    lcd.print(credits);

    lcd.setCursor(0, 1);
    lcd.print("Bet: ");
    lcd.print(bet);
}

int pickWeightedSymbol() {
    int randomValue = random(1, totalWeight + 1);
    int cumulativeWeight = 0;

    for (int i=0; i<numSymbols;i++) {
        cumulativeWeight+= symbolWeights[i];
        if (randomValue<= cumulativeWeight) {
            return i;
        }
    }
    return 0;
}

void drawWelcomeScreen() {
    tft.setRotation(4);
    tft.drawBitmap(0, 0, welcome_bitmap, 240, 320, ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextColor(ILI9341_YELLOW);
}

void drawSlotMachineBackground() {
    tft.setRotation(3);
    tft.fillScreen(tft.color565(139, 69, 19));

    int slotSize = 52;
    int borderThickness = 2;
    int startX = 30;
    int startY = 42;

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 5; col++) {
            int x= startX+ col * slotSize;
            int y= startY+ row * slotSize;

            tft.fillRect(x, y, slotSize, slotSize, ILI9341_YELLOW);
            tft.fillRect(x + borderThickness, y + borderThickness, slotSize - 2 * borderThickness, slotSize - 2 * borderThickness, ILI9341_BLACK);
        }
    }
}

void drawSlotResult(int slotGrid[3][5]) {
    tft.setRotation(3);

    int slotSize = 52;
    int borderThickness = 2;
    int startX = 30;
    int startY = 42;

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 5; col++) {
            int x = startX + col * slotSize + borderThickness;
            int y = startY + row * slotSize + borderThickness;

            tft.drawBitmap(x, y, slotSymbols[slotGrid[row][col]], 48, 48, ILI9341_WHITE);
        }
    }

    delay(1000);
}

void showWinMessage() {
    tft.setRotation(3);
    tft.fillScreen(ILI9341_GREEN);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.setCursor(20, 150);
    tft.println("You Win!");
    delay(2000);
}

bool checkWinCondition(int grid[3][5]) {
    for (int row= 0; row < 3; row++) {
        bool rowWin= true;
        for (int col= 1; col < 5; col++) {
            if (grid[row][col] != grid[row][0]) {
                rowWin = false;
                break;
            }
        }
        if (rowWin) return true;
    }

    for (int col= 0; col < 5; col++) {
        bool colWin = true;
        for (int row= 1; row < 3; row++) {
            if (grid[row][col] != grid[0][col]) {
                colWin = false;
                break;
            }
        }
        if (colWin) return true;
    }

    return false;
}

void showGameOver() {
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(3);
    tft.setCursor(80, 180);
    tft.println("Game Over");
    delay(2000);
}

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BET_INCREASE_PIN, INPUT_PULLUP);
    pinMode(BET_DECREASE_PIN, INPUT_PULLUP);
    pinMode(ADD_CREDITS_PIN, INPUT_PULLUP);

    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK);
    lcd.begin(16, 2);
    lcd.clear();
    displayBetAndCredits();
}

void loop() {
    adjustBet();
    displayBetAndCredits();

    int buttonState = digitalRead(BUTTON_PIN);

    if (digitalRead(ADD_CREDITS_PIN)== LOW) {
        credits += 200;
        displayBetAndCredits();
        delay(200);
    }

    if (!welcomeScreenShown) {
        drawWelcomeScreen();

        while (digitalRead(BUTTON_PIN)== HIGH) {
            delay(10);
        }

        welcomeScreenShown = true;
        buttonHeldDown = true;
    }

    if (welcomeScreenShown && buttonState== LOW && buttonHeldDown) {
        buttonHeldDown = false;

        if (credits < bet) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Not enough");
            lcd.setCursor(0, 1);
            lcd.print("credits!");
            delay(2000);
            return;
        }

        credits -= bet;
        displayBetAndCredits();

        int slotGrid[3][5];
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 5; col++) {
                slotGrid[row][col] = pickWeightedSymbol();
            }
        }

        drawSlotMachineBackground();
        delay(300);

        drawSlotResult(slotGrid);
        int payout = calculatePayout(slotGrid);
        if (payout > 0) {
            credits += payout;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("You Won!");
            lcd.setCursor(0, 1);
            lcd.print("Credits: ");
            lcd.print(payout);
            delay(1000);
        }
        displayBetAndCredits();
    }

    if (buttonState == HIGH) {
        buttonHeldDown = true;
    }
}
