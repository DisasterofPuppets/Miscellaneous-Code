/* Home Assistant voice assistant satellite, providing feedback via the Serial Monitor/Plotter
   and the ESP32's built-in LED.

   You will be prompted to select either the microphone or speaker check.
   The Speaker check will ask for a tone (440 is good) and play it for 5 seconds to confirm the speaker is working
   The Microphone check will blink the onboard LED to the volume of your voice to confirm audio input is working.

   Your ESPHome I2S Pin Definitions:
   It is assumed you have the same wiring per https://smarthomecircle.com/setup-esp32-with-wake-word-in-home-assistant#circuit-diagram-for-esp32-with-inmp441-microphone
   Direct link to wiring diagram: https://smarthomecircle.com/static/images/2023/esp32-voice-assistant/esp32-wiring-diagram.webp
     - Microphone (INMP441): WS (GPIO26), BCLK (GPIO25), SD (GPIO33)
     - Speaker (I2S DAC): DIN (GPIO27), BCLK (GPIO25), WS (GPIO26
     
*/
#include <driver/i2s_std.h>
#include <driver/gpio.h>

// I2S pin assignments (per wiring diagram)
#define I2S_WS         (gpio_num_t)26
#define I2S_SCK        (gpio_num_t)25
#define I2S_MIC_SD     (gpio_num_t)33
#define I2S_SPK_DOUT   (gpio_num_t)27

// Built-in LED pin
#define LED_BUILTIN 2

// Audio parameters
#define SAMPLE_RATE 16000
#define I2S_PORT_MIC I2S_NUM_0
#define I2S_PORT_SPK I2S_NUM_1
#define BUFFER_SIZE 1024

// Menu options
#define MIC_TEST 1
#define SPEAKER_TEST 2

void setup() {
  Serial.begin(115200);
  delay(4000); // allow USB serial to stabilize 
  Serial.flush();  // Ensures any buffered data is sent
  Serial.println("\nESP32 Audio Test Initialized");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  showMenu();
}

void loop() {
  if (Serial.available() > 0) {
    int choice = Serial.parseInt();
    while (Serial.available() > 0) Serial.read();

    switch (choice) {
      case MIC_TEST: runMicrophoneTest(); break;
      case SPEAKER_TEST: runSpeakerTest(); break;
      default:
        if (choice != 0) Serial.println("Invalid choice. Please select 1 or 2.");
        break;
    }

    delay(2000);
    showMenu();
  }
}

void showMenu() {
  Serial.println("\n=== ESP32 Audio Test Menu ===");
  Serial.println("1. Microphone Input test");
  Serial.println("2. Speaker Output test");
  Serial.println("Enter your choice (1 or 2):");
}

void runMicrophoneTest() {
  pinMode(27, OUTPUT);       // Mute speaker DIN
  digitalWrite(27, LOW);     // Force silence

  Serial.println("\n--- Microphone Input Test ---");
  Serial.print("Running Microphone Test . ");

  i2s_chan_handle_t rx_handle = NULL;
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT_MIC, I2S_ROLE_MASTER);
  esp_err_t err = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
  if (err != ESP_OK) {
    Serial.printf("Failed to create I2S channel: %s\n", esp_err_to_name(err));
    return;
  }

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = I2S_SCK,
      .ws   = I2S_WS,
      .dout = I2S_GPIO_UNUSED,
      .din  = I2S_MIC_SD,
      .invert_flags = { false, false, false }
    }
  };
  std_cfg.slot_cfg.bit_shift = true;

  err = i2s_channel_init_std_mode(rx_handle, &std_cfg);
  if (err != ESP_OK) {
    Serial.printf("Failed to init I2S: %s\n", esp_err_to_name(err));
    i2s_del_channel(rx_handle);
    return;
  }

  err = i2s_channel_enable(rx_handle);
  if (err != ESP_OK) {
    Serial.printf("Failed to enable I2S: %s\n", esp_err_to_name(err));
    i2s_del_channel(rx_handle);
    return;
  }

  int16_t samples[BUFFER_SIZE];
  size_t bytes_read;
  unsigned long testStart = millis();
  unsigned long lastDot = millis();

  while (millis() - testStart < 10000) {
    err = i2s_channel_read(rx_handle, samples, sizeof(samples), &bytes_read, portMAX_DELAY);
    if (err == ESP_OK && bytes_read > 0) {
      long sum = 0;
      int count = bytes_read / sizeof(int16_t);

      for (int i = 0; i < count; i++) {
        int32_t amplified = samples[i] * 3;
        amplified = constrain(amplified, -32768, 32767);
        samples[i] = amplified;
        sum += abs(samples[i]);
      }

      int avg_amplitude = sum / count;

      if (millis() - lastDot > 1000) {
        Serial.print(". ");
        lastDot = millis();
      }

      digitalWrite(LED_BUILTIN, avg_amplitude > 200 ? HIGH : LOW);
    }
  }

  digitalWrite(LED_BUILTIN, LOW);
  i2s_channel_disable(rx_handle);
  i2s_del_channel(rx_handle);
  Serial.println("\nMicrophone test completed!");
}

void runSpeakerTest() {
  pinMode(27, INPUT);  // Release DIN for I2S control

  Serial.println("\n--- Speaker Output Test ---");
  Serial.println("Enter frequency for test tone (300–4000 Hz):");

  while (!Serial.available()) delay(10);
  int frequency = Serial.parseInt();
  while (Serial.available() > 0) Serial.read();

  if (frequency < 300 || frequency > 4000) {
    Serial.println("Invalid frequency. Using 1000 Hz.");
    frequency = 1000;
  }

  Serial.printf("Playing %d Hz tone (5s): ", frequency);
  unsigned long testStart = millis();
  unsigned long lastDot = millis();

  i2s_chan_handle_t tx_handle = NULL;
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT_SPK, I2S_ROLE_MASTER);
  esp_err_t err = i2s_new_channel(&chan_cfg, &tx_handle, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed to create I2S channel: %s\n", esp_err_to_name(err));
    return;
  }

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = I2S_SCK,
      .ws   = I2S_WS,
      .dout = I2S_SPK_DOUT,
      .din  = I2S_GPIO_UNUSED,
      .invert_flags = { false, false, false }
    }
  };

  err = i2s_channel_init_std_mode(tx_handle, &std_cfg);
  if (err != ESP_OK) {
    Serial.printf("Failed to init I2S: %s\n", esp_err_to_name(err));
    i2s_del_channel(tx_handle);
    return;
  }

  err = i2s_channel_enable(tx_handle);
  if (err != ESP_OK) {
    Serial.printf("Failed to enable I2S: %s\n", esp_err_to_name(err));
    i2s_del_channel(tx_handle);
    return;
  }

  int16_t samples[BUFFER_SIZE];
  size_t bytes_written;
  float phase = 0;
  float phase_inc = 2.0 * PI * frequency / SAMPLE_RATE;

  while (millis() - testStart < 5000) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
      samples[i] = (int16_t)(sin(phase) * 6000); // 50% volume
      phase += phase_inc;
      if (phase >= 2.0 * PI) phase -= 2.0 * PI;
    }

    err = i2s_channel_write(tx_handle, samples, sizeof(samples), &bytes_written, portMAX_DELAY);
    if (err != ESP_OK) {
      Serial.printf("I2S write error: %s\n", esp_err_to_name(err));
      break;
    }

    if (millis() - lastDot > 1000) {
      Serial.print(". ");
      lastDot = millis();
    }
  }

  i2s_channel_disable(tx_handle);
  i2s_del_channel(tx_handle);
  Serial.println("\nSpeaker test completed!");
}

