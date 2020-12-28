//#include <Arduino.h>
#include <initializer_list>

#include <unity.h>

#include "display.h"

WordKlokDisplay display;

template <typename T>
void checkAllFlags(int hours, int minutes,
                   std::initializer_list<T> mask_positive,
                   std::initializer_list<T> mask_negative) {
    auto encoded = display.encodeTime(hours, minutes);
    for (auto i : mask_positive) {
        TEST_ASSERT_TRUE(encoded & i);
    }
    for (auto i : mask_negative) {
        TEST_ASSERT_FALSE_MESSAGE(encoded & i, "expected mask to be clear");
    }
}

template <typename T>
void checkAllFlags(int hours, int minutes,
                   std::initializer_list<T> mask_positive) {
    checkAllFlags(hours, minutes, mask_positive, {});
}

void test_five_minutes_after_any_hour(void) {
    checkAllFlags(12, 5, {MFIVE, MINUTES, PAST});
    checkAllFlags(9, 7, {MFIVE, MINUTES});
    checkAllFlags(7, 9, {MFIVE, MINUTES});
}

void test_ten_minutes_after_any_hour(void) {
    checkAllFlags(12, 12, {MTEN, MINUTES, PAST});
    checkAllFlags(9, 10, {MTEN, MINUTES});
    checkAllFlags(7, 14, {MTEN, MINUTES});
}

void test_quarter_after_any_hour(void) {
    checkAllFlags(12, 15, {QUARTER, PAST}, {MINUTES});
    checkAllFlags(9, 19, {QUARTER}, {MINUTES});
    checkAllFlags(7, 17, {QUARTER}, {MINUTES});
}

void test_twenty_minutes() {
    checkAllFlags(12, 20, {TWENTY, MINUTES});
    checkAllFlags(7, 24, {TWENTY, MINUTES});
}

void test_twenty_five_minutes() {
    checkAllFlags(12, 25, {TWENTY, MFIVE, MINUTES});
    checkAllFlags(7, 29, {TWENTY, MFIVE, MINUTES});
}

void test_half_hour() {
    checkAllFlags(12, 30, {HALF, PAST}, {MINUTES});
    checkAllFlags(9, 34, {HALF, PAST}, {MINUTES});
}

void test_o_clock() {
    checkAllFlags(12, 0, {OCLOCK}, {MINUTES, PAST});
    checkAllFlags(8, 3, {OCLOCK}, {MINUTES, PAST});
}

void test_twenty_five_to() {
    checkAllFlags(12, 35, {TWENTY, MFIVE, MINUTES, TO}, {PAST});
    checkAllFlags(12, 39, {TWENTY, MFIVE, TO}, {PAST});
}

void test_twenty_to() {
    checkAllFlags(12, 40, {TWENTY, MINUTES, TO}, {PAST, MFIVE});
    checkAllFlags( 7, 44, {TWENTY, MINUTES, TO}, {PAST, MFIVE});
}

void test_quarter_to() {
    checkAllFlags(12, 45, {QUARTER, TO}, {PAST, MINUTES});
    checkAllFlags(7,  49, {QUARTER, TO}, {PAST, MINUTES});
}

void test_ten_to() {
    checkAllFlags(12, 50, {MTEN, MINUTES, TO}, {PAST, QUARTER});
    checkAllFlags(7,  54, {MTEN, MINUTES, TO}, {PAST, QUARTER});
}

void test_five_to() {
    checkAllFlags(12, 55, {MFIVE, MINUTES, TO}, {PAST, QUARTER});
    checkAllFlags(7, 59, {MFIVE, MINUTES, TO}, {PAST, QUARTER});
}

void test_one_o_clock()  { checkAllFlags(1, 2, {ONE, OCLOCK}); }
void test_five_o_clock() { checkAllFlags(5, 2, {HFIVE, OCLOCK}, {MFIVE, ONE}); }
void test_quarter_to_seven () {
    checkAllFlags(6, 45, {QUARTER, TO, SEVEN}, {SIX, OCLOCK});
}
void test_quarter_to_one() {
    checkAllFlags(12, 45, {QUARTER, TO, ONE}, {TWELVE, OCLOCK});
}
void test_complete_cases() {
    checkAllFlags(12, 59, {MFIVE, TO, ONE}, {TWELVE, QUARTER, OCLOCK});
    checkAllFlags( 1, 00, {ONE, OCLOCK}, {TO, TWELVE, QUARTER});
}

void test_24h_to_12h_conversion() {
    checkAllFlags(13, 59, {MFIVE, TO, TWO}, {TWELVE, QUARTER, OCLOCK});
    checkAllFlags(14, 00, {TWO, OCLOCK}, {TWELVE, QUARTER});
    checkAllFlags(14,  4, {TWO, OCLOCK}, {TWELVE, QUARTER});

    checkAllFlags(12, 59, {MFIVE, TO, ONE}, {TWELVE, QUARTER, OCLOCK});
    checkAllFlags(13, 01, {ONE, OCLOCK}, {TWELVE, QUARTER, TO});

    checkAllFlags(18, 31, {HALF, PAST, SIX}, {OCLOCK, SEVEN, QUARTER, TO});
    checkAllFlags(18, 40, {TWENTY, TO, SEVEN}, {HALF, SIX, QUARTER, PAST});
}

void test_23_20() { checkAllFlags(23, 20, {TWENTY, MINUTES, PAST, ELEVEN}); }

void test_00_00() { checkAllFlags(0, 0, {IT, IS, TWELVE, OCLOCK}); }

void process() {

    UNITY_BEGIN();

    RUN_TEST(test_five_minutes_after_any_hour);
    RUN_TEST(test_ten_minutes_after_any_hour);
    RUN_TEST(test_quarter_after_any_hour);
    RUN_TEST(test_twenty_minutes);
    RUN_TEST(test_twenty_five_minutes);
    RUN_TEST(test_half_hour);
    RUN_TEST(test_o_clock);
    RUN_TEST(test_twenty_five_to);
    RUN_TEST(test_twenty_to);
    RUN_TEST(test_quarter_to);
    RUN_TEST(test_ten_to);
    RUN_TEST(test_five_to);

    RUN_TEST(test_one_o_clock);
    RUN_TEST(test_five_o_clock);
    RUN_TEST(test_quarter_to_seven);
    RUN_TEST(test_quarter_to_one);
    RUN_TEST(test_complete_cases);

    RUN_TEST(test_24h_to_12h_conversion);

    RUN_TEST(test_23_20);
    RUN_TEST(test_00_00);

    UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>

void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    process();

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}

#else // ! ARDUINO === Native

int main(int argc, char *argv[]) { process(); }

#endif