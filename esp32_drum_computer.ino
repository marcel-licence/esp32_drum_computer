/*
 * this file is the main project file which can be opened and compiled with arduino
 *
 * Author: Marcel Licence
 */

#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>

#include <WiFi.h>

#define SAMPLE_RATE	44100

volatile uint8_t midi_prescaler = 0;


void setup()
{
    // put your setup code here, to run once:
    delay(500);

    Serial.begin(115200);

    Serial.println();

    Serial.printf("Loading data\n");

    Serial.printf("Firmware started successfully\n");

    Blink_Setup();

    setup_i2s();

    Midi_Setup();

#if 0
    setup_wifi();
#else
    WiFi.mode(WIFI_OFF);
#endif

    btStop();

    Sampler_Init();
    Effect_Init();
    Sequencer_Init();

    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());
}


inline void audio_task()
{
    /* prepare out samples for processing */
    float fl_sample = 0.0f;
    float fr_sample = 0.0f;

    Sampler_Process(&fl_sample, &fr_sample);
    Effect_Process(&fl_sample, &fr_sample);
    Sequencer_Process(&fl_sample, &fr_sample);

    if (!i2s_write_samples(fl_sample, fr_sample))   //Don’t block the ISR if the buffer is full
    {
        // error!
    }
}


inline
void loop_1Hz(void)
{
    Blink_Process();
}


void loop()
{

    audio_task();

    static uint32_t loop_cnt;

    loop_cnt ++;
    if (loop_cnt >= SAMPLE_RATE)
    {
        loop_cnt = 0;
        loop_1Hz();
    }

    midi_prescaler++;
    if (midi_prescaler >= 16)
    {
        Midi_Process();
        midi_prescaler = 0;
    }
}

void Synth_SetSlider(uint8_t id, float value)
{
    switch (id)
    {
    case 0:
        Sampler_SetPlaybackSpeed(value);
        break;

    case 1:
        Effect_SetBiCutoff(value);
        break;
    case 2:
        Effect_SetBiReso(value);
        break;
    case 3:
        Effect_SetBitCrusher(value);
        break;

    case 4:
        Sampler_SetSoundPitch(value);
        break;

    default:
        break;
    }
}


void Synth_SetRotary(uint8_t id, float value)
{
    switch (id)
    {
    case 0:
        Sequencer_SetSpeed(value);
        break;
    case 1:
        Sequencer_SetShuffle(value);
        break;

    default:
        break;
    }
}

