#include "FastLED.h"

/*
 * Initialisation of all variables and constants.
 */

// Data PIN's.
#define REMOTE_INTR_PIN 3
#define REMOTE_A_PIN 5
#define REMOTE_B_PIN 7
#define REMOTE_C_PIN 4
#define REMOTE_D_PIN 6
#define LED_DATA_PIN 13

// Number of LED's.
#define NUM_LEDS 28

// Initialize counter and config minutes.
int counter;
int config_minutes = 20;

// Initialize mode (-1: config, 0: pause, 1: running).
int mode           = -1;

// Initiaize blink flag, which is used to let LED's blink.
bool blink         = false;

// Color for confiugration.
CHSV config_color = CHSV(130, 255, 100);

// Array which defines which LED's should light up for each digit.
int led_digits[10][7] = {
    {1,1,1,1,1,1,0},    // 0
    {1,1,0,0,0,0,0},    // 1
    {1,0,1,1,0,1,1},    // 2
    {0,0,1,1,1,1,1},    // 3
    {0,1,0,1,1,0,1},    // 4
    {0,1,1,0,1,1,1},    // 5
    {1,1,1,0,1,1,1},    // 6
    {0,0,1,1,1,0,0},    // 7
    {1,1,1,1,1,1,1},    // 8
    {0,1,1,1,1,1,1},    // 9
};


// Initialize leds (LED strip).
CRGB leds[NUM_LEDS];


/*
 * Setup method which is called when the controller is started.
 */

void setup()
{
    // Setup PIN's.
    pinMode(REMOTE_A_PIN, INPUT);
    pinMode(REMOTE_B_PIN, INPUT);
    pinMode(REMOTE_C_PIN, INPUT);
    pinMode(REMOTE_D_PIN, INPUT);

    // Setup interrupt PIN.
    attachInterrupt(digitalPinToInterrupt(REMOTE_INTR_PIN), intr, RISING);

    // Setup LED strip.
    FastLED.addLeds<WS2811, LED_DATA_PIN, BRG>(leds, NUM_LEDS);
}


/**
 * Interrupt callback function which controls the counter, config_minutes and mode.
 */

void intr()
{
    // Update counter.
    if(mode == -1 && digitalRead(REMOTE_A_PIN) == HIGH)
        counter = config_minutes * 60;

    // Pause.
    if(mode == 1 && digitalRead(REMOTE_A_PIN) == HIGH)
        mode = 0;

    // Start.
    else if(digitalRead(REMOTE_A_PIN) == HIGH)
        mode = 1;

    // Reset / go into config mode.
    else if((mode == 0 || counter <= 0) && digitalRead(REMOTE_C_PIN) == HIGH)
        mode = -1;

    // Increase config_minutes.
    else if(mode == -1 && digitalRead(REMOTE_B_PIN) == HIGH && config_minutes < 99)
        config_minutes ++;

    // Decrease config_minutes.
    else if(mode == -1 && digitalRead(REMOTE_D_PIN) == HIGH && config_minutes > 1)
        config_minutes --;
}


/**
 * Main loop method of the controller, which runs in an endless-loop.
 */

void loop()
{
    // Mode 1 is running, which will show the counter or the blinking timeout.
    if(mode == 1)
    {
        if(counter || blink)
            showCounter();
        else if(!blink)
            showNothing();

        if(counter > 0)
            counter--;

        delay(1000);
    }

    // Mode 0 is pause, which will show the blinking counter.
    else if(mode == 0)
    {
        if(blink)
            showCounter();
        else
            showNothing();
        delay(500);
    }

    // Mode -1 is config, which will show the config.
    else if(mode == -1)
    {
        showConfig();
    }

    // Invert blink flag.
    blink = !blink;
}


/**
 * Displays nothing, which basically blacks out the LED's.
 */

void showNothing()
{
    FastLED.clear();
    FastLED.show();
}


/**
 * Displays the config value.
 */

void showConfig()
{
    FastLED.clear();
    setNumber(config_minutes, config_color);
    FastLED.show();
}


/**
 * Displays the active counter value.
 */

void showCounter()
{
    // Get color for specific second.
    CHSV color = getColor();

    FastLED.clear();

    // Countdown for minutes.
    if(counter > 60)
        setNumber((counter + 60 - 1) / 60, color);

    // Countdown for seconds.
    else if(counter <= 60 && counter > 0)
        setNumber(counter, color);

    // Timeout.
    else
        setTimeout();

    FastLED.show();
}


/**
 * Shows the timeout.
 */

void setTimeout()
{
    leds[12] = leds[13] = leds[26] = leds[27] = getColor();
}


/**
 * Shows a number in a specific color.
 */

void setNumber(int number, CHSV color)
{
    // Split first and second digit with math.
    int digit1 = number / 10;
    int digit2 = number % 10;

    // Only display first digit if it's higher than 0.
    if(digit1)
        setDigit(digit1, 0, color);

    // Always display second digit (with an offset of 14).
    setDigit(digit2, 14, color);
}


/**
 * Sets the LED's for a single digit with an optional offset.
 */

void setDigit(int digit, int offset, CHSV color)
{
    /*
     * We've a 7-segment LED digit, but "each" segment has 2 LED's. Therefor we
     * initialize "i" for the segment count and "j" for the LED's start address.
     */

    for(int i=0, j=offset; i<7; i++, j=offset+i*2)
        if(led_digits[digit][i])
            leds[j] = leds[j+1] = color;
}


/**
 * Returns the color for the current second.
 */

CHSV getColor()
{
    int hue = (float)(counter) / (float)(config_minutes * 60) * 120;
    return CHSV(hue, 255, 255);
}