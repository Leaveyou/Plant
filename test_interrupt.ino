#include <LowPower.h>
#include <Wire.h>
#include <ds3231.h>

#define BUFF_MAX 256
#define HOUR_INTERVAL 0
#define MINUTE_INTERVAL 1


const byte interruptPin = 2;
volatile byte state = LOW;
int awakePin = 13;
struct ts t;
uint8_t sleep_period = 1;       // the sleep interval in minutes between 2 consecutive alarms

typedef struct time_interval {
    uint8_t hours;
    uint8_t minutes;

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

void interrupt_routine(){
    DS3231_get(&t);

    time_interval timeInterval;
    timeInterval.hours = HOUR_INTERVAL;
    timeInterval.minutes = MINUTE_INTERVAL;

    set_timer(timeInterval);
}

void loop() {
    digitalWrite(awakePin, HIGH);
    delay(100);
    if(digitalRead(interruptPin) == LOW) {
        Serial.println("Alarm is ringing.");
        interrupt_routine();
    }
    else{
        Serial.println("Arduino has rebooted");
    }

    if (DS3231_triggered_a2()) {
        Serial.println("No  alarm set. Setting a new one");
        interrupt_routine();
    } else {
        Serial.println("Alarm already set.");
    }

    delay(100);
    digitalWrite(awakePin, LOW);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

}

void set_timer(time_interval timeInterval) {
    struct ts t;

    DS3231_get(&t);

    // calculate the hour and minute when the next alarm will be triggered

    unsigned char wakeup_minute = (t.min + timeInterval.minutes) % 60;
    unsigned char wakeup_hour = ((t.hour + timeInterval.hours) + (t.min + timeInterval.minutes) / 60) % 24;

    DS3231_clear_a2f();

    // flags define what calendar component to be checked against the current time in order
    // to trigger the alarm
    // A2M2 (minutes) (0 to enable, 1 to disable)
    // A2M3 (hour)    (0 to enable, 1 to disable)
    // A2M4 (day)     (0 to enable, 1 to disable)
    // DY/DT          (dayofweek == 1/dayofmonth == 0)
    uint8_t flags[4] = {0, 0, 1, 1};
    DS3231_set_a2(wakeup_minute, wakeup_hour, 0, flags);

    // activate Alarm2
    DS3231_set_creg(DS3231_CONTROL_INTCN | DS3231_CONTROL_A2IE);
}


void blink() {
    // do nothing. Just wakes up arduino
}
