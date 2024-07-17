const uint8_t RGB_PIN     = 7;
const uint8_t button1_pin = 0; // Red team
const uint8_t button2_pin = 4; // Blue team
const uint8_t button3_pin = 9; // Yellow team

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

	pinMode(button1_pin, INPUT_PULLUP);
	pinMode(button2_pin, INPUT_PULLUP);
	pinMode(button3_pin, INPUT_PULLUP);

	led_on(RGB_PIN, RGB_PURPLE); // Purple
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
		led_on(RGB_PIN, RGB_ORANGE); // Orange
		delay(2000);

		return;
	}

	if (has_buzz_in && !is_locked_out) {
		if (b1) {
			Serial.printf("Team #1 buzzed in\r\n");
			led_on(RGB_PIN, RGB_RED); // Red
		} else if (b2) {
			Serial.printf("Team #2 buzzed in\r\n");
			led_on(RGB_PIN, RGB_BLUE); // Blue
		} else if (b3) {
			Serial.printf("Team #3 buzzed in\r\n");
			led_on(RGB_PIN, RGB_YELLOW); // Yellow
		}

		//Serial.printf("B1: %d B2: %d B3: %d\r\n", b1, b2, b3);
		last_buzzin = millis();
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

void led_on(uint8_t pin, int8_t color) {
	int8_t brightness = 20;

	// Only set the color if it has changed
	if (color == last_color) {
		return;
	}

	//Serial.printf("Setting pin #%d to color %d\r\n", pin, color);

	if (color == RGB_RED) {
		neopixelWrite(pin,brightness,0,0); // Red
	} else if (color == RGB_GREEN) {
		neopixelWrite(pin,0,brightness,0); // Green
	} else if (color == RGB_BLUE) {
		neopixelWrite(pin,0,0,brightness); // Blue
	} else if (color == RGB_PURPLE) {
		neopixelWrite(pin,brightness,0,brightness); // Purple
	} else if (color == RGB_YELLOW) {
		neopixelWrite(pin,brightness,brightness,0); // Yellow
	} else if (color == RGB_ORANGE) {
		neopixelWrite(pin,brightness,brightness / 5,0); // Orange
	} else if (color == RGB_WHITE) {
		neopixelWrite(pin,0,brightness,brightness); // White
	} else {
		neopixelWrite(pin,0,0,0); // Off/Black
	}

	last_color = color;
}
