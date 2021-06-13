#include <LowPower.h>
#include <Wire.h>
#include <DS3231.h>
#include <EEPROM.h>

#define HOUR_INTERVAL 0
#define MINUTE_INTERVAL 0
#define SECOND_INTERVAL 10
#define UNIX_INTERVAL (HOUR_INTERVAL * 3600 + MINUTE_INTERVAL * 60 + SECOND_INTERVAL)

const byte interruptPin = 2;
int awakePin = 13;
long lastCheck = 0;

RTClib myRTC;


// to do: customizable time interval / water pump
// todo: implementeaza scriere in bucla round-robin pe eeprom pt protectie și pentru log anterior (max 100.000 scrieri până e kaput)
// todo: implementeaza afisaj pe ecran
// implementeaza buton scos din sleep cu encoder de rotatie
// alarmă pt "nu pare sa se fi udat planta cand trebuia" - scrie eroare pe eeprom si afiseaza la apasare buton

void setup() 
{
    Serial.begin(115200);    
    pinMode(awakePin, OUTPUT);
    power_peripherials();
    pinMode(interruptPin, INPUT_PULLUP);
    Wire.begin();


//    for(int i = 0; i<512; i++) {
//      EEPROM.write(i, 0);
//    }

}

void loop() {
  
    uint32_t current_time = get_time();
    long last_check = getLastCheck();
    long time_since_check = current_time - last_check;

    if (time_since_check < UNIX_INTERVAL)
    {
        unpower_peripherials();
        nipsleep(UNIX_INTERVAL - time_since_check, 0, 0);
        power_peripherials();
    }
       
    check();
    delay(1000);
}

void nipsleep(long seconds, long minutes, long hours)
{
    // todo: treat interrupt pin. Currently it interrupts at most a 8s sleep cycle; perhaps intended
    unsigned long total_seconds = 0;
    total_seconds += seconds;
    total_seconds += minutes*60;
    total_seconds += hours*3600;
    
    for (unsigned long cursor = 0; cursor < total_seconds/8; cursor++ ) {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
    for (unsigned long cursor = 0; cursor < total_seconds%8; cursor++ ) {
        LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
    }
}

void check()
{
    // todo implement
}

long getLastCheck()
{
    if (!lastCheck){
        Serial.println("Last check time stale. getting from eeprom...");
        EEPROM.get(0, lastCheck);
    }
    
    return lastCheck;
}


void power_peripherials()
{
    digitalWrite(awakePin, HIGH);
    delay(1);
}

void unpower_peripherials()
{
    digitalWrite(awakePin, LOW);
    delay(1);
}

uint32_t get_time() 
{
    DateTime now = myRTC.now();
    return now.unixtime();
}
