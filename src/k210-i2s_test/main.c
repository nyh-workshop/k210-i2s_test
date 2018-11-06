/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <devices.h>
#include <string.h>
#include <stdio.h>
#include <pin_cfg.h>
#include <FreeRTOS.h>
#include <task.h>
#include "wavetable.h"

// Kendryte K210 Dock (Sipeed) I2S sample code for FreeRTOS SDK.
// Ng YH (www.github.com/uncle-yong)
// Acknowledgement to YangFangFei for the assistance!
// Also thanks to Kendryte and Sipeed team for the help too!

// Buffer length in samples:
#define BUFFER_LENGTH 529

const fpioa_cfg_t g_fpioa_cfg = 
{
    // Kendryte K210 Dock:
    .version = PIN_CFG_VERSION,
    .functions_count = 4,
    .functions[0] = {34, FUNC_I2S2_OUT_D1},
    .functions[1] = {33, FUNC_I2S2_WS},
    .functions[2] = {35, FUNC_I2S2_SCLK},
    
    .functions[3] = {39, FUNC_GPIOHS23}
};

const audio_format_t audio = { AUDIO_FMT_PCM, 16, 44100, 2 };

unsigned int buffer_pp[BUFFER_LENGTH];

uint8_t *buffer = NULL;
size_t frames;
handle_t i2s0;
handle_t gio;

void init_i2s(void)
{
    i2s_stop(i2s0);
    i2s_config_as_render(i2s0, &audio, 12, I2S_AM_RIGHT, 0xc);
    i2s_start(i2s0);
}

void generateSineTask(void *pvParameter) {

    unsigned int accum1t = 0;
    unsigned int tuningWord1t = 42949673;

   while (1)
   {
       // Generate a sine wave in a buffer.
       for (unsigned int n = 0; n < BUFFER_LENGTH; n++)
       {
           accum1t += tuningWord1t;
           buffer_pp[n] = (wavetable[accum1t >> 20]) & 0x0000ffff;
       }

       i2s_get_buffer(i2s0, &buffer, &frames);
       size_t buffer_size = frames * 4;
       memcpy(buffer, buffer_pp, buffer_size);
       i2s_release_buffer(i2s0, frames);
    }
}

int main()
{

    gio = io_open("/dev/gpio0");
    configASSERT(gio);
    
    i2s0 = io_open("/dev/i2s2");
    configASSERT(i2s0);
    init_i2s();

    gpio_set_drive_mode(gio, 23, GPIO_DM_OUTPUT);
    gpio_set_pin_value(gio, 23, GPIO_PV_HIGH);

    xTaskCreate(generateSineTask, "generateSineTask", 512, NULL, 3, NULL);

    while (1)
        ;
}