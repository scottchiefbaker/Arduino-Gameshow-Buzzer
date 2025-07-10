#include <FastLED.h>
#include <YX5300_ESP32.h>

// *make sure the RX on the YX5300 goes to the TX on the ESP32, and vice-versa
#define YX5300_RX 5
#define YX5300_TX 3

const String VERSION = "v0.1.2";

const uint8_t RGB_PIN    = 33;  // Pin the ds2812s are on
const uint16_t LED_COUNT = 200; // Number of LEDs on the strip
CRGB leds[LED_COUNT];

const uint8_t button1_pin = 39; // Red team
const uint8_t button2_pin = 37; // Blue team
const uint8_t button3_pin = 35; // Yellow team

const uint8_t attract_minutes = 2;    // Minutes of inactivity before attract mode starts
const uint16_t lockout_time   = 3000; // Lockout other buttons for X milliseconds after a buzz-in
uint32_t last_buzzin          = 0;
int8_t last_color             = -1;

#define RGB_RED    1
#define RGB_GREEN  2
#define RGB_BLUE   3
#define RGB_PURPLE 4
#define RGB_YELLOW 5
#define RGB_ORANGE 6
#define RGB_WHITE  7
#define RGB_OFF    0

YX5300_ESP32 mp3; // The global MP3 object for playing sound effects

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void setup() {
	Serial.begin(115200);

	mp3 = YX5300_ESP32(Serial1, YX5300_RX, YX5300_TX);

	// River Room production - GRB
	FastLED.addLeds<WS2812, RGB_PIN, GRB>(leds, LED_COUNT);
	// Test bed LED strand - BGR
	//FastLED.addLeds<WS2812, RGB_PIN, BGR>(leds, LED_COUNT);

	FastLED.setBrightness(99);

	led_on(RGB_PIN, RGB_RED);
	delay(1000);
	led_on(RGB_PIN, RGB_BLUE);
	delay(1000);
	led_on(RGB_PIN, RGB_GREEN);
	delay(1000);

	pinMode(button1_pin, INPUT_PULLUP);
	pinMode(button2_pin, INPUT_PULLUP);
	pinMode(button3_pin, INPUT_PULLUP);

	Serial.printf("Gameshow Buzzer %s\r\n\r\n", VERSION.c_str());
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

	// If there is a tie we randomly choose a team the tied to "win" in check_tie()
	if (is_tie) {
		Serial.printf("OMG THERE WAS A TIE\r\n");
		//led_on(RGB_PIN, RGB_WHITE);
		//delay(10);
	}

	if (has_buzz_in && !is_locked_out) {
		last_buzzin = millis();

		// Play the first MP3 in the first folder
		play_sound(1);

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
	// After a couple of minutes we go into attract mode
	} else if (enter_attract_mode()) {
		attract_mode();
	} else if (!is_locked_out) {
		led_on(RGB_PIN, RGB_OFF); // Turn off LED
	}
}

bool enter_attract_mode() {
	const uint32_t next_attract_time = last_buzzin + (attract_minutes * 60 * 1000);

	bool ret = false;
	if (millis() > next_attract_time) {
		ret = true;
	} else {
		ret = false;
	}

	return ret;
}

void attract_mode() {
	pride();
	FastLED.show();
}

// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
void pride() {
	static uint16_t sPseudotime = 0;
	static uint16_t sLastMillis = 0;
	static uint16_t sHue16 = 0;

	uint8_t sat8 = beatsin88( 87, 220, 250);
	uint8_t brightdepth = beatsin88( 341, 96, 224);
	uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
	uint8_t msmultiplier = beatsin88(147, 23, 60);

	uint16_t hue16 = sHue16;//gHue * 256;
	uint16_t hueinc16 = beatsin88(113, 1, 3000);

	uint16_t ms = millis();
	uint16_t deltams = ms - sLastMillis ;
	sLastMillis  = ms;
	sPseudotime += deltams * msmultiplier;
	sHue16 += deltams * beatsin88( 400, 5,9);
	uint16_t brightnesstheta16 = sPseudotime;

	for( uint16_t i = 0 ; i < LED_COUNT; i++) {
		hue16 += hueinc16;
		uint8_t hue8 = hue16 / 256;

		brightnesstheta16  += brightnessthetainc16;
		uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

		uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
		uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
		bri8 += (255 - brightdepth);

		CRGB newcolor = CHSV( hue8, sat8, bri8);

		uint16_t pixelnumber = i;
		pixelnumber = (LED_COUNT-1) - pixelnumber;

		nblend( leds[pixelnumber], newcolor, 64);
	}
}

bool check_tie(uint8_t &b1, uint8_t &b2, uint8_t &b3) {
	// Seed the random numbers with uptime micros
	srand(micros());

	// Three way tie!?!
	if ((b1 && b1 == b2) && (b3 == b2)) {
		int rand = random(100) % 3;

		// Set them all to zero
		b1 = b2 = b3 = 0;

		if      (rand == 0) { b1 = 1; }
		else if (rand == 1) { b2 = 1; }
		else if (rand == 2) { b3 = 1; }

		return true;
	}

	// Tie between 1 and 2
	if (b1 && b1 == b2) {
		int rand = random(100) % 2;

		// Set them all to zero
		b1 = b2 = 0;

		if      (rand == 0) { b1 = 1; }
		else if (rand == 1) { b2 = 1; }

		return true;
	}

	// Tie between 2 and 3
	if (b2 && b2 == b3) {
		int rand = random(100) % 2;

		// Set them all to zero
		b2 = b3 = 0;

		if      (rand == 0) { b2 = 1; }
		else if (rand == 1) { b3 = 1; }

		return true;
	}

	// Tie between 1 and 3
	if (b3 && b3 == b1) {
		int rand = random(100) % 2;

		// Set them all to zero
		b1 = b3 = 0;

		if      (rand == 0) { b1 = 1; }
		else if (rand == 1) { b3 = 1; }

		return true;
	}

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

bool play_sound(uint16_t num) {
	mp3.playTrack(num);

	return true;
}
