#include "Timer.h"

Timer::Timer(bool* keyPressed, char* key, bool* power, void (*StateChange)(byte), LiquidCrystal* lcd) : keyPressed(keyPressed), key(key), power(power), StateChange(StateChange), lcd(lcd) {
}

void Timer::begin() {
    Wire.begin();
    rtc.begin();
    rtc.adjust(DateTime(ReferenceDate));
}

void Timer::EnterTimer() {
    *power = false;
    ToggleDisplay();
    DisplayInstructions();
}

void Timer::Update() {
    // Do Nothing if not active
    if (state != 1) return;

    remainingTime = setTime - rtc.now().secondstime() + startTime;

    // If True Timer Reached
    if (remainingTime <= 0 || remainingTime > 400000) {
        // Stop Timer and Reset
        Stop();
        Reset();
        bool prevPower = *power;
        // Wait For Input to Stop Flashing Screen
        while (!*keyPressed && Serial.available() == 0) {
            prevPower = !prevPower;
            digitalWrite(LCD_ANODE, prevPower);
            delay(100);
        }

        digitalWrite(LCD_ANODE, *power);

        *keyPressed = true;
        *key = ' ';
        Serial.readString();
        Serial.flush();
    }
}

void Timer::AcceptInput() {
    if (*keyPressed) {
        *keyPressed = false;
        KeyCheck();
    }
}

void Timer::UpdateTimerString() {
    // When in State Set the timerString will be displayed for Timer Value Selection, else just print the value in remainingTime
    if (state != 2 && *power) {
        lcd->setCursor(0, 0);
        temp = remainingTime / 3600;
        timerString = (temp) < 10 ? "0" + String(temp) : String(temp);
        timerString += ":";
        temp = (remainingTime / 60) % 60;
        timerString += (temp) < 10 ? "0" + String(temp) : String(temp);
        timerString += ':';
        temp = remainingTime % 60;
        timerString += (temp) < 10 ? "0" + String(temp) : String(temp);
        lcd->print(timerString);
    }
}

void Timer::ToggleDisplay() {
    *power = !*power;
    lcd->clear();
    digitalWrite(LCD_ANODE, *power);
    state = (state == 2) ? 0 : state;
    Reset();
    lcd->noCursor();
}

void Timer::KeyCheck() {
    if (*key == 'O') ToggleDisplay();

    if (!*power) return;

    if (*key == 'M') {
        StateChange(0);
        return;
    };

    switch (state) {
        case 0:
            /* Stopped */
            switch (*key) {
                case '1':
                    // Start
                    Start();
                    break;
                case '2':
                    // Reset
                    Reset();
                    break;
                case '3':
                    // Set
                    state = 2;
                    cursorPosition = 0;
                    break;
                default:
                    break;
            }
            break;
        case 1:
            /* Active */
            switch (*key) {
                case '1':
                    // Stop
                    Stop();
                    break;
                default:
                    break;
            }
            break;
        case 2:
            /* Set */
            switch (*key) {
                case '0' ... '9':
                    InsertKey();
                    break;
                case '<':
                    MoveLeft();
                    break;
                case '>':
                    MoveRight();
                    break;
                case '=':
                    Set();
                    break;
                default:
                    break;
            }
            break;
    }

    DisplayInstructions();
}

void Timer::Start() {
    if (setTime == 0) return;
    // If reset Start from Beginning else Resume
    if (remainingTime == setTime) {
        // Start from Beginning
        startTime = rtc.now().secondstime();
    } else {
        startTime = rtc.now().secondstime() - (setTime - remainingTime);
    }
    state = 1;
}

void Timer::Stop() {
    // Update remainingTime before Stopping
    remainingTime = setTime - rtc.now().secondstime() + startTime;
    state = 0;
}

void Timer::Reset() {
    // Resets the remainingTime value to the Set Value
    remainingTime = setTime;
}

void Timer::Set() {
    // Sets the setTime variable using the timerString value set during this state
    setTime = timerString.substring(0, 2).toInt() * 3600 + timerString.substring(3, 5).toInt() * 60 + timerString.substring(6, 8).toInt();
    Reset();
    state = 0;
}

void Timer::DisplayInstructions() {
    lcd->clear();

    if (state == 2) {
        lcd->print(timerString);
    }

    // Write the instructions below the Timer Value
    lcd->setCursor(0, 1);

    switch (state) {
        case 0:
            lcd->print(TIMER_STATE_0_INSTRUCTIONS);
            break;
        case 1:
            lcd->print(TIMER_STATE_1_INSTRUCTIONS);
            break;
        case 2:
            lcd->print(TIMER_STATE_2_INSTRUCTIONS);
            break;
        default:
            break;
    }

    // Set the Cursor Position According to the cursorPosition
    lcd->setCursor(cursorPosition, 0);

    // When State is Set, show Cursor else noCursor
    if (state == 2) {
        lcd->cursor();
    } else {
        lcd->noCursor();
    }
}

void Timer::InsertKey() {
    if ((cursorPosition == 3 || cursorPosition == 6) && (*key) > '5') {  // Skip the Colon
        *key = '5';
    }

    timerString[cursorPosition] = *key;
}

void Timer::MoveLeft() {
    if (cursorPosition > 0) {
        cursorPosition--;
    }
    // Skip Colon
    if (cursorPosition == 2 || cursorPosition == 5) {
        cursorPosition--;
    }
}

void Timer::MoveRight() {
    if (cursorPosition < timerString.length() - 1) {
        cursorPosition++;
    }
    // Skip Colon
    if (cursorPosition == 2 || cursorPosition == 5) {
        cursorPosition++;
    }
}