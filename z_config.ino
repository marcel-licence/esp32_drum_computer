/*
 * Copyright (c) 2021 Marcel Licence
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
 * @file z_config.ino
 * @author Marcel Licence
 * @date 12.05.2021
 *
 * @brief This file contains the mapping configuration
 * Put all your project configuration here (no defines etc)
 * This file will be included at the and can access all
 * declarations and type definitions
 *
 * @see ESP32 Arduino DIY Synthesizer Projects - Little startup guide to get your MIDI synth working - https://youtu.be/ZNxGCB-d68g
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif




/*
 * this mapping is used for the edirol pcr-800
 * this should be changed when using another controller
 */
struct midiControllerMapping edirolMapping[] =
{
    /* transport buttons */
    { 0x8, 0x52, "back", NULL, NULL, 0},
    { 0xD, 0x52, "stop", Sequencer_Stop, NULL, 0},
    { 0xe, 0x52, "start", Sequencer_Start, NULL, 0},
    { 0xa, 0x52, "rec", NULL, NULL, 0},

    /* upper row of buttons */
    { 0x0, 0x50, "A1", Sequencer_ClickOnOff, NULL, 0},
    { 0x1, 0x50, "A2", Sequencer_ClearAll, NULL, 0},
    { 0x2, 0x50, "A3", NULL, NULL, 0},
    { 0x3, 0x50, "A4", NULL, NULL, 0},

    { 0x4, 0x50, "A5", Sequencer_Max1, NULL, 0},
    { 0x5, 0x50, "A6", Sequencer_Max2, NULL, 0},
    { 0x6, 0x50, "A7", Sequencer_Max3, NULL, 0},
    { 0x7, 0x50, "A8", Sequencer_Max4, NULL, 0},

    { 0x0, 0x53, "A9", Sequencer_Max5, NULL, 0},

    /* lower row of buttons */
    { 0x0, 0x51, "B1", Sequencer_ModeIdle, NULL, 0},
    { 0x1, 0x51, "B2", Sequencer_ModeRecord, NULL, 0},
    { 0x2, 0x51, "B3", Sequencer_DeleteTrack, NULL, 0},
    { 0x3, 0x51, "B4", NULL, NULL, 0},

    { 0x4, 0x51, "B5", Sequencer_ModeSolo, NULL, 0},
    { 0x5, 0x51, "B6", Sequencer_ModeMute, NULL, 0},
    { 0x6, 0x51, "B7", NULL, NULL, 0},
    { 0x7, 0x51, "B8", NULL, NULL, 0},

    { 0x1, 0x53, "B9", NULL, NULL, 0},

    /* pedal */
    { 0x0, 0x0b, "VolumePedal", NULL, NULL, 0},

    /* slider */
    { 0x0, 0x11, "S1", NULL, Synth_SetSlider, 0},
    { 0x1, 0x11, "S2", NULL, Synth_SetSlider, 1},
    { 0x2, 0x11, "S3", NULL, Synth_SetSlider, 2},
    { 0x3, 0x11, "S4", NULL, Synth_SetSlider, 3},

    { 0x4, 0x11, "S5", NULL, Synth_SetSlider, 4},
    { 0x5, 0x11, "S6", Sampler_SetDecay, NULL, 5},
    { 0x6, 0x11, "S7", NULL, Synth_SetSlider, 6},
    { 0x7, 0x11, "S8", NULL, Synth_SetSlider, 7},

    { 0x1, 0x12, "S9", NULL, Synth_SetSlider, 8},

    /* rotary */
    { 0x0, 0x10, "R1", NULL, Synth_SetRotary, 0},
    { 0x1, 0x10, "R2", NULL, Synth_SetRotary, 1},
    { 0x2, 0x10, "R3", NULL, Synth_SetRotary, 2},
    { 0x3, 0x10, "R4", NULL, Synth_SetRotary, 3},

    { 0x4, 0x10, "R5", NULL, Synth_SetRotary, 4},
    { 0x5, 0x10, "R6", NULL, Synth_SetRotary, 5},
    { 0x6, 0x10, "R7", NULL, Synth_SetRotary, 6},
    { 0x7, 0x10, "R8", NULL, Synth_SetRotary, 7},

    { 0x0, 0x12, "R9", NULL, Reverb_SetLevel, 0},

    /* Central slider */
    //{ 0x0, 0x13, "H1", NULL, Synth_SetMidiMasterTempo, 0},
    { 0x0, 0x13, "H1", NULL, NULL, 0},
};

struct midiMapping_s midiMapping =
{
    NULL,
    Synth_NoteOn,
    Synth_NoteOff,
    NULL,
    NULL,
    NULL, /* assign program change callback here! */
    NULL,
    NULL,
    edirolMapping,
    sizeof(edirolMapping) / sizeof(edirolMapping[0]),
};
