#include "Calculator.h"

// Helper Functions

inline bool IsOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

inline bool IsMultiplier(char c) {
    return c == '+' || c == '-';
}

inline byte Precedence(char op) {
    return (op == '+' || op == '-') ? 1 : (op == '*' || op == '/') ? 2
                                                                   : 0;
}

// Stack Functions
void Calculator::PushOperator(char op) {
    operatorStack[operatorStackTop] = op;
    operatorStackTop++;
}

void Calculator::PushDigit(double digit) {
    digitStack[digitStackTop] = digit;
    digitStackTop++;
}

void Calculator::PushDigit(String& digit) {
    if (digit.length() > 0) {
        PushDigit(digit.toFloat());
        digit = "";
    }
}

void Calculator::PopOperator() {
    operatorStackTop--;
}

void Calculator::PopDigit() {
    digitStackTop--;
}

char Calculator::PeekOperator() {
    return operatorStack[operatorStackTop - 1];
}

double Calculator::PeekDigit() {
    return digitStack[digitStackTop - 1];
}

void Calculator::Clear() {
    operatorStackTop = 0;
    digitStackTop = 0;
}

String Calculator::Calculate(String expression) {
    Clear();
    char c = ' ';
    String numberStr = "";
    double number1 = 0.0;
    double number2 = 0.0;
    byte i = 0;
    bool decimalFlag = false;

    // If Empty Expression Return Empty String
    if (expression.length() == 0) {
        return "";
    }

    // Cannot Start with Operator or )
    if ((IsOperator(expression[0]) || expression[0] == ')')) {
        if (!IsMultiplier(expression[0])) {
            return FORMAT_ERROR;
        } else {
            numberStr += expression[0];
            i++;
        }
    }

    for (; i < expression.length(); i++) {
        c = expression[i];

        if (isDigit(c) || c == '.') {
            // Check if Expression is Formatted Correctly. Cannot Have ) without Operator in Between.
            if (i != 0 && expression[i - 1] == ')') {
                return FORMAT_ERROR;
            }

            // Check For Decimal and Multiple Decimal Points
            if (decimalFlag) {
                if (c == '.') {
                    return FORMAT_ERROR;
                }
            } else {
                if (c == '.') {
                    decimalFlag = true;
                }
            }

            numberStr += c;

        } else if (c == '(') {
            // Check if Expression is Formatted Correctly. Cannot Have digit or ) without Operator in Between.
            if (i != 0 && (isDigit(expression[i - 1]) || expression[i - 1] == ')')) {
                return FORMAT_ERROR;
            }

            // Token Code End

            PushOperator('(');

        } else if (c == ')') {
            // Check if Expression is Formatted Correctly. Cannot Have operator or ( before a closing parenthesis.
            if (i != 0 && (IsOperator(expression[i - 1]) || expression[i - 1] == '(')) {
                return FORMAT_ERROR;
            }

            // Push to Digit Stack if Previous Token was a Number
            PushDigit(numberStr);
            decimalFlag = false;

            // Token Code End

            while (PeekOperator() != '(') {
                number2 = PeekDigit();
                PopDigit();

                number1 = PeekDigit();
                PopDigit();

                PushDigit(ApplyOperation(number1, number2, PeekOperator()));
                PopOperator();
            }

            PopOperator();

        } else if (IsOperator(c)) {
            // Check if Expression is Formatted Correctly. Cannot Have operator or ( before another operator.
            if (i != 0 && (IsOperator(expression[i - 1]) || expression[i - 1] == '(')) {
                // Check if Number is Negative Number
                if (IsMultiplier(c) && numberStr.length() == 0) {
                    numberStr += c;
                    continue;
                } else {
                    return FORMAT_ERROR;
                }
            }

            // Push to Digit Stack if Previous Token was a Number
            PushDigit(numberStr);
            decimalFlag = false;

            // Token Code End

            while (operatorStackTop > 0 && Precedence(PeekOperator()) >= Precedence(c)) {
                number2 = PeekDigit();
                PopDigit();

                number1 = PeekDigit();
                PopDigit();

                PushDigit(ApplyOperation(number1, number2, PeekOperator()));
                PopOperator();
            }

            PushOperator(c);
        }
    }

    // Push if Last Token is a Number
    if (numberStr.length() > 0) {
        PushDigit(numberStr);
    }

    // Clear Operator Stack
    while (operatorStackTop > 0) {
        number2 = PeekDigit();
        PopDigit();

        number1 = PeekDigit();
        PopDigit();

        PushDigit(ApplyOperation(number1, number2, PeekOperator()));
        PopOperator();
    }

    return String(PeekDigit());
}

double Calculator::ApplyOperation(double num1, double num2, char op) {
    switch (op) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            return num1 / num2;
        default:
            return 0.0;
    }
}

// Constructors/Initializers
Calculator::Calculator() {
    Clear();
}