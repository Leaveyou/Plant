#include <LowPower.h>
#include <Wire.h>
#include <DS3231.h>
#include <EEPROM.h>

#define AWAKE_PIN 13  // Powers peripherials. set via [un]unpower_peripherials() function
#define EEPROM_DATA_VERSION 2 // increment this every type you make changes to the eeprom structure

// store settings on eeprom 
typedef struct settings {
    uint8_t data_version   = EEPROM_DATA_VERSION % 256;
    uint8_t checks_stored  = 10;
    uint8_t pump_seconds   = 5;
    uint8_t check_interval = 12; 
    uint16_t humidity_threshold = 50;
} settings;

// store checks on eeprom
typedef struct check {
    uint32_t timestamp = 0;
    uint16_t humidity = 0;
    bool ran = 0;
} check;


settings CurrentSettings;
RTClib myRTC;
uint16_t eeprom_write_address = 0;




// to do: customizable time interval / water pump
// todo: implementeaza scriere in bucla round-robin pe eeprom pt protectie și pentru log anterior (max 100.000 scrieri până e kaput)
// todo: implementeaza afisaj pe ecran
// implementeaza buton scos din sleep cu encoder de rotatie
// alarmă pt "nu pare sa se fi udat planta cand trebuia" - scrie eroare pe eeprom si afiseaza la apasare buton


void setup() 
{
    Serial.begin(115200);    
    pinMode(AWAKE_PIN, OUTPUT);
    power_peripherials();
    Wire.begin();

    CurrentSettings = GetCurrentSettings();
}

void loop() {
  
    uint32_t current_time = get_time();
    long last_check = getLastCheck();
    long time_since_check = current_time - last_check;

    uint16_t check_interval_seconds = CurrentSettings.check_interval * 3600; //time always processed in seconds for unixtime
    if (time_since_check < check_interval_seconds) 
    {
        unpower_peripherials();
        nipsleep(check_interval_seconds - time_since_check, 0, 0);
        power_peripherials();
    }
       
    wakeup_routine();

    
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

void log_watering()
{
    uint32_t current_time = get_time(); 
    // todo: everything below
}


void water_plant() 
{
    // todo: implement    
}

int getHumidity()
{
    return 250;
    // todo: implement
}

void wakeup_routine()
{
    uint8_t humidity = getHumidity();
    uint32_t timestamp = get_time();
    
    if (humidity < CurrentSettings.humidity_threshold) {
        water_plant();
    }
    
    log_watering();
}

long getLastCheck()
{
    // todo: implement
}


void power_peripherials()
{
    digitalWrite(AWAKE_PIN, HIGH);
    delay(1);
}

void unpower_peripherials()
{
    digitalWrite(AWAKE_PIN, LOW);
    delay(1);
}

uint32_t get_time() 
{
    DateTime now = myRTC.now();
    return now.unixtime();
}

settings GetCurrentSettings()
{
    settings eeprom_settings;
    EEPROM.get(0, eeprom_settings);
    if (eeprom_settings.data_version == EEPROM_DATA_VERSION)
    {
        return eeprom_settings;
              
    }
    settings default_settings;
    FlashEEPROM(default_settings);
}

void FlashEEPROM(settings system_settings)
{
    Serial.println("");

    Serial.print("Flashing EEPROM... ");
    check default_check;
    EEPROM.put(0, system_settings);    

    for (size_t i = 0; i < system_settings.checks_stored; i++)
    {
        size_t check_address = sizeof(system_settings) + i*sizeof(check);
        EEPROM.put(check_address, default_check); 
    }
    eeprom_write_address = sizeof(system_settings);
    Serial.println(" done");
}
