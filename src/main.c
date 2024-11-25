#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define PIN_CLK     GPIO_NUM_32
#define PIN_DT      GPIO_NUM_33
#define PIN_SW      GPIO_NUM_25
#define PIN_GRN_LED GPIO_NUM_23
#define PIN_RED_LED GPIO_NUM_22

// Event types
typedef enum {
    EVENT_ENCODER,
    EVENT_BUTTON
} event_type_t;

typedef struct {
    event_type_t type;
    int value;
} encoder_event_t;

// Global variables
static volatile uint8_t last_CLK_state = 0;
static QueueHandle_t event_queue = NULL;

// GPIO interrupt handler for encoder
static void IRAM_ATTR gpio_isr_handler() {
    const uint8_t CLK_state = gpio_get_level(PIN_CLK);
    const uint8_t DT_state = gpio_get_level(PIN_DT);

    if (CLK_state != last_CLK_state) {
        int encoder_increment = 0;
        if (DT_state != CLK_state)
        {
            encoder_increment = 1;
            gpio_set_level(PIN_GRN_LED, 1);
            gpio_set_level(PIN_RED_LED, 0);
        }
        else
        {
            encoder_increment = -1;
            gpio_set_level(PIN_GRN_LED, 0);
            gpio_set_level(PIN_RED_LED, 1);
        }
        const encoder_event_t evt = {
            .type = EVENT_ENCODER,
            .value = encoder_increment
        };
        xQueueSendFromISR(event_queue, &evt, NULL);
    }
    last_CLK_state = CLK_state;
}

// Button interrupt handler
static void IRAM_ATTR button_isr_handler() {
    static bool button_value = false;
    button_value = !button_value;

    encoder_event_t evt = {
        .type = EVENT_BUTTON,
        .value = button_value
    };
    xQueueSendFromISR(event_queue, &evt, NULL);
}

void app_main(void) {
    // Create event queue
    event_queue = xQueueCreate(40, sizeof(encoder_event_t));

    // Configure GPIO pins
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };

    // Set bit mask for encoder pins
    io_conf.pin_bit_mask = (1ULL << PIN_CLK) | (1ULL << PIN_DT);
    gpio_config(&io_conf);

    // Configure button pin separately
    gpio_config_t button_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    button_conf.pin_bit_mask = (1ULL << PIN_SW);
    gpio_config(&button_conf);

    gpio_config_t led_conf = {
        .mode = GPIO_MODE_OUTPUT
    };
    led_conf.pin_bit_mask = (1ULL << PIN_GRN_LED) | (1ULL << PIN_RED_LED);
    gpio_config(&led_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(0);

    // Add ISR handlers
    gpio_isr_handler_add(PIN_CLK, gpio_isr_handler, NULL);
    gpio_isr_handler_add(PIN_SW, button_isr_handler, NULL);

    // Initialize last state
    last_CLK_state = gpio_get_level(PIN_CLK);

    // Track values
    int encoder_value = 0;
    bool button_state = false;
    encoder_event_t evt;

    // Print initial values
    printf("\033[2J");  // Clear screen
    printf("\033[H");   // Move cursor to home position
    printf("Button state: %d | Encoder value: %d", button_state, encoder_value);
    fflush(stdout);

    // Main loop
    while (1) {
        if (xQueueReceive(event_queue, &evt, pdMS_TO_TICKS(1000))) {
            bool should_update = false;

            switch (evt.type) {
                case EVENT_ENCODER:
                    encoder_value += evt.value;
                    should_update = true;
                    break;

                case EVENT_BUTTON:
                    button_state = evt.value;
                    should_update = true;
                    break;
            }

            if (should_update) {
                printf("\r");  // Return cursor to start of line
                printf("Button state: %d | Encoder value: %d", button_state, encoder_value);
                printf("\033[K");  // Clear to end of line
                fflush(stdout);
            }
        }
    }
}