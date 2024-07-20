#include <FastLED.h>

const uint8_t RGB_PIN     = 23; // Pin the ds2812s are on
const uint16_t LED_COUNT  = 200; // Number of LEDs on the strip
CRGB leds[LED_COUNT];

const uint8_t button1_pin = 15; // Red team
const uint8_t button2_pin = 16; // Blue team
const uint8_t button3_pin = 4;  // Yellow team

const uint16_t lockout_time = 2500; // Milliseconds
uint32_t last_buzzin        = 0;
int8_t last_color           = -1;

#define RGB_RED    1
#define RGB_GREEN  2
#define RGB_BLUE   3
#define RGB_PURPLE 4
#define RGB_YELLOW 5
#define RGB_ORANGE 6
#define RGB_WHITE  7
#define RGB_OFF    0

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void setup() {
	Serial.begin(115200);

	// Init FastLED in RGB mode (GRB, or BRG also available)
	FastLED.addLeds<WS2812, RGB_PIN, RGB>(leds, LED_COUNT);
	FastLED.setBrightness(128);

	pinMode(button1_pin, INPUT_PULLUP);
	pinMode(button2_pin, INPUT_PULLUP);
	pinMode(button3_pin, INPUT_PULLUP);

	led_on(RGB_PIN, RGB_GREEN); // Green
	delay(3000);

	Serial.printf("Red    team is pin #%d\r\n", button1_pin);
	Serial.printf("Blue   team is pin #%d\r\n", button2_pin);
	Serial.printf("Yellow team is pin #%d\r\n", button3_pin);
	Serial.printf("Buzz-in lockout time is %d\r\n\r\n", lockout_time);
	led_on(RGB_PIN, RGB_OFF); // Off
}

void loop() {
	// These are INPUT_PULLUP so they are HIGH when not pressed so we
	// invert these so 0 = not pressed and 1 = pressed
	uint8_t b1 = !digitalRead(button1_pin);
	uint8_t b2 = !digitalRead(button2_pin);
	uint8_t b3 = !digitalRead(button3_pin);

	bool has_buzz_in   = (b1 || b2 || b3);
	bool is_locked_out = (millis() - last_buzzin) < lockout_time;
	bool is_tie        = check_tie(b1, b2, b3);

	// FIXME: If two buttons trigger at the same time we need to handle that
	// somehow. Currently B1 will beat B2 and B2 will beat B3 regardless of
	// timing. Maybe randomize?
	if (is_tie) {
		Serial.printf("OMG THERE WAS A TIE\r\n");
		led_on(RGB_PIN, RGB_WHITE);
		delay(200);

		return;
	}

	if (has_buzz_in && !is_locked_out) {
		last_buzzin = millis();

		if (b1) {
			Serial.printf("Red Team #1 buzzed in\r\n");
			buzz_in(1);
		} else if (b2) {
			Serial.printf("Blue Team #2 buzzed in\r\n");
			buzz_in(2);
		} else if (b3) {
			Serial.printf("Yellow Team #3 buzzed in\r\n");
			buzz_in(3);
		}

		//Serial.printf("B1: %d B2: %d B3: %d\r\n", b1, b2, b3);
	} else if (!is_locked_out) {
		led_on(RGB_PIN, RGB_OFF); // Turn off LED
	}
}

bool check_tie(uint8_t b1, uint8_t b2, uint8_t b3) {
	if (b1 && b1 == b2) { return true; }
	if (b2 && b2 == b3) { return true; }
	if (b3 && b3 == b1) { return true; }

	return false;
}

void buzz_in(int8_t team_num) {
	int8_t color = 0;

	if (team_num == 1) {
		color = RGB_RED;
	} else if (team_num == 2) {
		color = RGB_BLUE;
	} else if (team_num == 3) {
		color = RGB_YELLOW;
	}

	// Flashing affect with the team color
	for (int i = 0; i < 10; i++) {
		led_on(RGB_PIN, color);
		delay(100);
		led_on(RGB_PIN, RGB_OFF);
		delay(100);
	}
}

void led_on(uint8_t pin, int8_t color) {
	// Only set the color if it has changed
	if (color == last_color) {
		return;
	}

	//Serial.printf("Setting pin #%d to color %d\r\n", pin, color);

	// Turn on ALL the LEDs in the string
	if (color == RGB_RED) {
		for (uint16_t i = 0; i < LED_COUNT; i++) { leds[i] = CRGB::Red; }
	} else if (color == RGB_GREEN) {
		for (uint16_t i = 0; i < LED_COUNT; i++) { leds[i] = CRGB::Green; }
	} else if (color == RGB_BLUE) {
		for (uint16_t i = 0; i < LED_COUNT; i++) { leds[i] = CRGB::Blue; }
	} else if (color == RGB_PURPLE) {
		for (uint16_t i = 0; i < LED_COUNT; i++) { leds[i] = CRGB::Purple; }
	} else if (color == RGB_YELLOW) {
		for (uint16_t i = 0; i < LED_COUNT; i++) { leds[i] = CRGB::Yellow; }
	} else if (color == RGB_ORANGE) {
		for (uint16_t i = 0; i < LED_COUNT; i++) { leds[i] = CRGB(255, 255 / 6.0, 0); }
	} else if (color == RGB_WHITE) {
		for (uint16_t i = 0; i < LED_COUNT; i++) { leds[i] = CRGB::White; }
	} else {
		for (uint16_t i = 0; i < LED_COUNT; i++) { leds[i] = CRGB::Black; }
	}

	FastLED.show();

	last_color = color;
}
