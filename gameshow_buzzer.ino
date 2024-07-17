const uint8_t RGB_PIN     = 7;
const uint8_t button1_pin = 0;
const uint8_t button2_pin = 4;
const uint8_t button3_pin = 9;

const uint16_t lockout_time = 2500; // Milliseconds
uint32_t last_buzzin        = 0;

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void setup() {
	Serial.begin(115200);

	pinMode(button1_pin, INPUT_PULLUP);
	pinMode(button2_pin, INPUT_PULLUP);
	pinMode(button3_pin, INPUT_PULLUP);

	led_on(RGB_PIN, 4); // Purple
	delay(3000);

	Serial.printf("Red   team is pin #%d\r\n", button1_pin);
	Serial.printf("Blue  team is pin #%d\r\n", button2_pin);
	Serial.printf("Green team is pin #%d\r\n", button3_pin);
	Serial.printf("Buzz-in lockout time is %d\r\n\r\n", lockout_time);
	led_on(RGB_PIN, 0); // Purple
}

void loop() {
	uint8_t b1 = digitalRead(button1_pin);
	uint8_t b2 = digitalRead(button2_pin);
	uint8_t b3 = digitalRead(button3_pin);

	bool has_buzz_in   = (b1 == 0) || (b2 == 0) || (b3 == 0);
	bool is_locked_out = (millis() - last_buzzin) < lockout_time;

	if (has_buzz_in && !is_locked_out) {
		if (b1 == 0) {
			Serial.printf("Team #1 buzzed in\r\n");
			led_on(7, 1); // Red
		} else if (b2 == 0) {
			Serial.printf("Team #2 buzzed in\r\n");
			led_on(7, 2); // Green
		} else if (b3 == 0) {
			Serial.printf("Team #3 buzzed in\r\n");
			led_on(7, 3); // Blue
		}

		//Serial.printf("B1: %d B2: %d B3: %d\r\n", b1, b2, b3);
		last_buzzin = millis();
	} else if (!is_locked_out) {
		led_on(7, 0); // Turn off LED
	}

	// For some reason things lock up if there isn't a delay
	delay(1);
}

void led_on(uint8_t pin, int8_t color) {
	int8_t brightness = 20;

	if (color == 1) {
		neopixelWrite(pin,brightness,0,0); // Red
	} else if (color == 2) {
		neopixelWrite(pin,0,brightness,0); // Green
	} else if (color == 3) {
		neopixelWrite(pin,0,0,brightness); // Blue
	} else if (color == 4) {
		neopixelWrite(pin,brightness,0,brightness); // Purple
	} else if (color == 5) {
		neopixelWrite(pin,brightness,brightness,0); // Yellow
	} else if (color == 6) {
		neopixelWrite(pin,brightness,brightness / 5,0); // Orange
	} else if (color == 7) {
		neopixelWrite(pin,0,brightness,brightness); // White
	} else {
		neopixelWrite(pin,0,0,0); // Off/Black
	}
}
