#include "mbed.h"
#include "arm_book_lib.h"

#define TIME_INI  1593561600  // 1 de julio de 2020, 00:00:00
#define BAUD_RATE 9600
#define DELAY_BETWEEN_TICK 500 // 500 ms
#define SWITCH_TICK_RAIN BUTTON1
#define RAINFALL_CHECK_INTERVAL 1 // in minute
#define MM_PER_TICK 0.2f // 0.2 mm de agua por tick

void printRain(const char* buffer);
const char* DateTimeNow(void);
void accumulateRainfall();
bool hasTimePassedMinutesRTC(int minutes);
void printAccumulatedRainfall();


BufferedSerial pc(USBTX, USBRX, BAUD_RATE);

DigitalOut alarmLed(LED1);
DigitalIn tickRain(SWITCH_TICK_RAIN);
DigitalOut tickLed(LED2);

int rainfallCount = 0;
int lastMinute = -1;

int main()
{
    tickRain.mode(PullDown);
    alarmLed = OFF;
    tickLed = OFF;

    // Configurar la fecha y hora inicial
    set_time(TIME_INI); 

    while (true) {
        if (tickRain == ON) {
            alarmLed = ON;
            tickLed = ON;
            const char* currentTime = DateTimeNow();
            printRain(currentTime);
            accumulateRainfall();
            delay(DELAY_BETWEEN_TICK);
        } else {
            alarmLed = OFF;
            tickLed = OFF;
        }
       if (hasTimePassedMinutesRTC(RAINFALL_CHECK_INTERVAL)) {
            printAccumulatedRainfall();
            rainfallCount = 0;
        }

        
    }
}

void printAccumulatedRainfall() {
    time_t seconds = time(NULL);
    struct tm* timeinfo = localtime(&seconds);
    char dateTime[80];
    strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M", timeinfo);
    
    int rainfallInteger = (int)(rainfallCount * MM_PER_TICK);
    int rainfallDecimal = (int)((rainfallCount * MM_PER_TICK - rainfallInteger) * 100);
    
    char buffer[100];
    int len = sprintf(buffer, "%s - Accumulated rainfall: %d.%02d mm\n", 
                      dateTime, rainfallInteger, rainfallDecimal);
    
     pc.write(buffer, len);
}

void accumulateRainfall() {
    time_t seconds = time(NULL);
    struct tm* timeinfo = localtime(&seconds);
    int currentMinute = timeinfo->tm_min;
    
    rainfallCount++;
    
    if (currentMinute != lastMinute) {
        if (lastMinute != -1) {  // No imprimir en la primera iteración
            printf("Rainfall in the last minute: %d ticks\n", rainfallCount);
        }
        rainfallCount = 0;
        lastMinute = currentMinute;
    }
}

const char* DateTimeNow() {
    time_t seconds = time(NULL);
    static char bufferTime[80];
    strftime(bufferTime, sizeof(bufferTime), "%Y-%m-%d %H:%M:%S", localtime(&seconds));
    return bufferTime;
}

void printRain(const char* buffer) {
    //printf("%s - Rain detected\n", buffer);
    pc.write(buffer, strlen(buffer));
    pc.write(" - Rain detected\r\n", 18);   
}

bool hasTimePassedMinutesRTC(int minutes) {
    static int lastMinute = -1;
    static int minutesPassed = 0;

    time_t seconds = time(NULL);
    struct tm* timeinfo = localtime(&seconds);
    int currentMinute = timeinfo->tm_min;

    if (lastMinute == -1) {
        lastMinute = currentMinute;
        return false;
    }

    if (currentMinute != lastMinute) {
        minutesPassed++;
        lastMinute = currentMinute;

        if (minutesPassed >= minutes) {
            minutesPassed = 0;
            return true;
        }
    }

    return false;
}