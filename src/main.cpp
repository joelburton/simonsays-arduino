/** Simon for Arduino. */

#include <Arduino.h>

#define BUZZER_PIN 8

// Button and LED pins, in order: green BR, yellow TR, red BL, blue TL
int LED_PINS[] = {12, 9, 5, 2};
int BUTTON_PINS[] = {13, 10, 6, 3};
int NOTES[] = {262, 294, 330, 349};

// Number of rounds for difficulty --- blue=4, yellow=6, red=10, green=16
int DIFFICULTIES[] = {16, 6, 10, 4};

int numToWin;
int solution[32];  // Max # of rounds


void setup() {
    Serial.begin(9600);

    for (int i = 0; i < 4; i++) {
        pinMode(LED_PINS[i], OUTPUT);
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    }

    // Seed random generator from random noise on analog 0
    randomSeed(analogRead(0));
}

/** Turn off all leds and turn on ledNum (-1 to just turn everything off) */

void setLed(int ledNum) {
    for (int i = 0; i < 4; i++) {
        digitalWrite(LED_PINS[i], i == ledNum ? HIGH : LOW);
    }
}

/** Returns button pushed (if any; if not, returns -1) */

int getButton() {
    for (int i = 0; i < 4; ++i) {
        if (digitalRead(BUTTON_PINS[i]) == LOW) return i;
    }
    return -1;
}

/** Before each game, light REDs in order until button is pushed. Returns button pushed. */

int preGame() {
    unsigned long previousMillis = millis();
    int count = 0;
    int pushed;

    while ((pushed = getButton()) == -1) {
        if (millis() - previousMillis > 250) {
            setLed(count % 4);
            if (count < 4) tone(BUZZER_PIN, NOTES[count % 4], 50);
            previousMillis = millis();
            ++count;
        }
    }

    setLed(-1);
    return pushed;
}

/** Display clue for roundNum. */

void displayClue(int roundNum) {
    setLed(-1);

    for (int i = 0; i <= roundNum; i++) {
        setLed(solution[i]);
        tone(BUZZER_PIN, NOTES[solution[i]], 250);
        delay(500);
        setLed(-1);
        delay(250);
    }
}

/** Wait for a button press and return t/f for whether it is correct. */

bool checkGuess(int goal) {
    int pushed;
    while ((pushed = getButton()) == -1) {}

    setLed(pushed);
    tone(BUZZER_PIN, NOTES[pushed], 250);
    delay(250);
    setLed(-1);
    delay(100); // debounce
    return goal == pushed;
}

/** Play game: returns -1 for win or round # of missed move. */

int gamePlay() {
    // Calculate full solution for game
    for (int i = 0; i < numToWin; i++) {
        solution[i] = (int) random(0, 4);
    }

    for (int roundNum = 0; roundNum < numToWin; ++roundNum) {
        displayClue(roundNum);

        for (int i = 0; i <= roundNum; i++) {
            if (checkGuess(solution[i])) {
                // Correct input
                if (i == numToWin - 1 ) return -1;
            } else {
                // Incorrect input
                return i;
            }
        }

        delay(1000);
    }
}

/** Winning display: spin lights quickly. */

void winDisplay() {
    int order[] = {0, 1, 2, 3, 3, 2, 1, 0};
    for (int i = 0; i < 8 * 3; ++i) {
        digitalWrite(LED_PINS[order[i % 8]], HIGH);
        delay(75);
        digitalWrite(LED_PINS[order[i % 8]], LOW);
        delay(75);
    }
}

/** Play winning song. */

void winSong() {
    tone(BUZZER_PIN, 294, 250);
    delay(200);
    tone(BUZZER_PIN, 294, 250);
    delay(200);
    tone(BUZZER_PIN, 294, 250);
    delay(200);
    tone(BUZZER_PIN, 392, 500);
    delay(500);
    tone(BUZZER_PIN, 392, 250);
    delay(200);
    tone(BUZZER_PIN, 440, 250);
    delay(200);
    tone(BUZZER_PIN, 392, 250);
    delay(200);
    tone(BUZZER_PIN, 440, 250);
    delay(200);
    tone(BUZZER_PIN, 494, 500);
}

/** Flash all LEDs --- played at end of game. */

void flashAll(int numTimes) {
    for (int i = 0; i < numTimes; ++i) {
        digitalWrite(LED_PINS[0], HIGH);
        digitalWrite(LED_PINS[1], HIGH);
        digitalWrite(LED_PINS[2], HIGH);
        digitalWrite(LED_PINS[3], HIGH);
        delay(200);
        digitalWrite(LED_PINS[0], LOW);
        digitalWrite(LED_PINS[1], LOW);
        digitalWrite(LED_PINS[2], LOW);
        digitalWrite(LED_PINS[3], LOW);
        delay(200);
    }
}

/** Play losing buzzer song. */

void loseSong() {
    tone(BUZZER_PIN, 98, 250);
    delay(250);
    tone(BUZZER_PIN, 93, 250);
    delay(250);
    tone(BUZZER_PIN, 87, 250);
    delay(250);
}

/** Main loop:
 *
 * - runs pre-game screen
 * - after button push, starts game
 * - handles win/lose
 * - returns (which starts all over again)
 */

void loop() {
    numToWin = DIFFICULTIES[preGame()];
    delay(1000);

    int roundLost = gamePlay();
    if (roundLost == -1) {
        delay(500);
        winSong();
        winDisplay();
        delay(1000);
        flashAll(3);
        delay(1000);
    } else {
        delay(100);
        loseSong();
        delay(200);
        tone(BUZZER_PIN, NOTES[solution[roundLost]], 250);
        setLed(solution[roundLost]);
        delay(500);
        flashAll(3);
        delay(1000);
    }
}
