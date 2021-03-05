#include <LowPower.h>
#include <Wire.h>
#include <ds3231.h>

#define BUFF_MAX 256

#define HOUR_INTERVAL 0
#define MINUTE_INTERVAL 0
#define SECOND_INTERVAL 5

const byte interruptPin = 2;
volatile byte state = LOW;
int awakePin = 13;
struct ts t;

typedef struct time_interval {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} time_interval;


void setup() {
    Serial.begin(9600);
    pinMode(awakePin, OUTPUT);
    pinMode(interruptPin, INPUT_PULLUP);
    Wire.begin();
    DS3231_init(DS3231_CONTROL_INTCN);
    attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
    //interrupt_routine();  // delete me
}

void interrupt_routine() {
    DS3231_get(&t);
    time_interval timeInterval;
    timeInterval.hours = HOUR_INTERVAL;
    timeInterval.minutes = MINUTE_INTERVAL;
    timeInterval.seconds = SECOND_INTERVAL;
    set_timer(timeInterval);
}

void loop() {
    digitalWrite(awakePin, HIGH);
    delay(100);
    if (digitalRead(interruptPin) == LOW) {
        Serial.println("Alarm is ringing.");
        interrupt_routine();
    } else {
        Serial.println("Arduino has rebooted");
    }

    // todo: sqw needs external pull-up resistor. This means no alarm can function without external power. DS3231_triggered_a1 might be enough. TBD
    char buff[BUFF_MAX];
    DS3231_get_a1(&buff[0], 59);
    if (buff[0] == 0x00 || DS3231_triggered_a1()) {
        Serial.println("No  alarm set. Setting a new one");
        interrupt_routine();

    } else {
        Serial.println("Alarm already set.");
    }

    delay(100);
    digitalWrite(awakePin, LOW);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

}

void set_timer(time_interval timeInterval)
{
    struct ts t;

    DS3231_get(&t);

    // calculate the hour and minute when the next alarm will be triggered

    unsigned char wakeup_second = t.sec + timeInterval.seconds;
    unsigned char wakeup_minute = t.min + timeInterval.minutes;
    unsigned char wakeup_hour = t.hour + timeInterval.hours;

    if (wakeup_second >= 60 ) {
        wakeup_minute += wakeup_second / 60;
        wakeup_second %=60;
    }

    if (wakeup_minute >= 60 ) {
        wakeup_hour += wakeup_minute / 60;
        wakeup_minute %=60;
    }

    if (wakeup_hour >= 60 ) {
        wakeup_hour %= 24;
    }

    DS3231_clear_a1f();
    DS3231_clear_a2f();

    // flags define what calendar component to be checked against the current time in order
    // to trigger the alarm
    // (seconds) (0 to enable, 1 to disable)
    // (minutes) (0 to enable, 1 to disable)
    // (hour)    (0 to enable, 1 to disable)
    // (day)     (0 to enable, 1 to disable)
    // DY/DT          (dayofweek == 1/dayofmonth == 0)
    uint8_t flags[5] = {0, 0, 0, 1, 1};
    DS3231_set_a1(wakeup_second, wakeup_minute, wakeup_hour, 0, flags);

    // activate Alarm2
    DS3231_set_creg(DS3231_CONTROL_INTCN | DS3231_CONTROL_A1IE);
}


void blink() {
    // do nothing. Just wakes up arduino
}

