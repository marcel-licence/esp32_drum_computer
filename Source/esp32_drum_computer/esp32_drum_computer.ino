/*
 * Copyright (c) 2022 Marcel Licence
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
 * veröffentlichten Version, weiter verteilen und/oder modifizieren.
 *
 * Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
 * OHNE JEDE GEWÄHR,; sogar ohne die implizite
 * Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Einzelheiten.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
 */

/**
 * @file ml_synth_basic_example.ino
 * @author Marcel Licence
 * @date 27.03.2021
 *
 * @brief   This file is the main project file which can be opened and compiled with arduino
 *
 * The project is shown in this video: @see https://youtu.be/vvA7vfouk84
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif

/*
 * global project configuration
 * stays on top of multi-file-compilation
 */
#include "config.h"


#include <Arduino.h>
#include <FS.h>

//#include <SD_MMC.h>
//#include <WiFi.h>

//#define SPI_DISP_ENABLED

/* requires the ML_SynthTools library: https://github.com/marcel-licence/ML_SynthTools */
#include <ml_reverb.h>
#ifdef OLED_OSC_DISP_ENABLED
#include <ml_scope.h>
#endif


void setup()
{
    // put your setup code here, to run once:
    delay(500);

    Serial.begin(115200);

    Serial.println();

    Serial.printf("Loading data\n");

    Serial.printf("Firmware started successfully\n");

#ifdef BLINK_LED_PIN
    Blink_Setup();
#endif

    Audio_Setup();

    Sampler_Init();
    Effect_Init();
    Sequencer_Init();

    /*
     * setup midi module / rx port
     */
    Midi_Setup();

    /*
     * Initialize reverb
     * The buffer shall be static to ensure that
     * the memory will be exclusive available for the reverb module
     */
    //static float revBuffer[REV_BUFF_SIZE];
    static float *revBuffer = (float *)malloc(sizeof(float) * REV_BUFF_SIZE);
    Reverb_Setup(revBuffer);

#ifdef ESP32
    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());

    Serial.printf("Total heap: %d\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

    /* PSRAM will be fully used by the looper */
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
#endif

#ifdef OLED_OSC_DISP_ENABLED
    Core0TaskInit();
#endif
}


#ifdef ESP32
/*
 * Core 0
 */
/* this is used to add a task to core 0 */
TaskHandle_t Core0TaskHnd;

inline
void Core0TaskInit()
{
    /* we need a second task for the terminal output */
    xTaskCreatePinnedToCore(Core0Task, "CoreTask0", 8000, NULL, 999, &Core0TaskHnd, 0);
}

inline
void Core0TaskSetup()
{
    /*
     * init your stuff for core0 here
     */

#ifdef OLED_OSC_DISP_ENABLED
    ScopeOled_Setup();
#endif
}

void Core0TaskLoop()
{
    /*
     * put your loop stuff for core0 here
     */

#ifdef OLED_OSC_DISP_ENABLED
    ScopeOled_Process();
#endif
}

void Core0Task(void *parameter)
{
    Core0TaskSetup();

    while (true)
    {
        Core0TaskLoop();

        /* this seems necessary to trigger the watchdog */
        delay(1);
        yield();
    }
}
#endif /* ESP32 */

static float fl_sample[SAMPLE_BUFFER_SIZE];
static float fr_sample[SAMPLE_BUFFER_SIZE];

inline void audio_task()
{
#ifdef AUDIO_PASS_THROUGH
    memset(fl_sample, 0, sizeof(fl_sample));
    memset(fr_sample, 0, sizeof(fr_sample));
#ifdef ESP32_AUDIO_KIT
    Audio_Input(fl_sample, fr_sample);
#endif
#else
    memset(fl_sample, 0, sizeof(fl_sample));
    memset(fr_sample, 0, sizeof(fr_sample));
#endif

    /* prepare out samples for processing */
    for (int n = 0; n < SAMPLE_BUFFER_SIZE; n++)
    {


        Sampler_Process(&fl_sample[n], &fr_sample[n]);
        Effect_Process(&fl_sample[n], &fr_sample[n]);
        Sequencer_Process(&fl_sample[n], &fr_sample[n]);
    }

    Reverb_Process(fl_sample, SAMPLE_BUFFER_SIZE);
    memcpy(fr_sample,  fl_sample, sizeof(fr_sample));

    Audio_Output(fl_sample, fr_sample);

#ifdef OLED_OSC_DISP_ENABLED
    ScopeOled_AddSamples(fl_sample, fr_sample, SAMPLE_BUFFER_SIZE);
#endif
}

inline
void loop_1Hz(void)
{
    Blink_Process();
}

void loop()
{
    static uint32_t loop_cnt;

    loop_cnt += SAMPLE_BUFFER_SIZE;
    if (loop_cnt >= SAMPLE_RATE)
    {
        loop_cnt = 0;
        loop_1Hz();
    }

    Midi_Process();

    audio_task();
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

void Synth_NoteOn(uint8_t ch, uint8_t note, float vol)
{
    float volF = vol;
    volF *= 127;
    Sequencer_NoteOn(note, volF);
    Sampler_SelectNote(note);
}

void Synth_NoteOff(uint8_t ch, uint8_t note)
{
    Sequencer_NoteOff(note);
}

