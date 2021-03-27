/*
 * a simple implementation to use midi
 *
 * Author: Marcel Licence
 */


/*
 * look for midi interface using 1N136
 * to convert the MIDI din signal to
 * a uart compatible signal
 */
#define RXD2 16 /* U2RRXD */
#define TXD2 17

/* use define to dump midi data */
//#define DUMP_SERIAL2_TO_SERIAL

/* constant to normalize midi value to 0.0 - 1.0f */
#define NORM127MUL	0.007874f

inline void Midi_NoteOn(uint8_t note, uint8_t vol)
{
    //Sampler_NoteOn(note, vol);
    Sequencer_NoteOn(note, vol);
    Sampler_SelectNote(note);
}

inline void Midi_NoteOff(uint8_t note)
{
    //Sampler_NoteOff(note);
    Sequencer_NoteOff(note);
}

struct midiControllerMapping
{
    uint8_t channel;
    uint8_t data1;
    const char *desc;
    void(*callback)(uint8_t ch, uint8_t data1, uint8_t data2);
};


/* mapping table to connect controller elements to callback functions */
struct midiControllerMapping edirolMapping[] =
{
    { 0x8, 0x52, "back", NULL},
    { 0xD, 0x52, "stop", Sequencer_Stop},
    { 0xe, 0x52, "start", Sequencer_Start},
    { 0xa, 0x52, "rec", NULL},

    /* upper row of buttons */
    { 0x0, 0x50, "A1", Sequencer_ClickOnOff},
    { 0x1, 0x50, "A2", Sequencer_ClearAll},
    { 0x2, 0x50, "A3", NULL},
    { 0x3, 0x50, "A4", NULL},

    { 0x4, 0x50, "A5", Sequencer_Max1},
    { 0x5, 0x50, "A6", Sequencer_Max2},
    { 0x6, 0x50, "A7", Sequencer_Max3},
    { 0x7, 0x50, "A8", Sequencer_Max4},

    { 0x0, 0x53, "A9", Sequencer_Max5},

    /* lower row of buttons */
    { 0x0, 0x51, "B1", Sequencer_ModeIdle},
    { 0x1, 0x51, "B2", Sequencer_ModeRecord},
    { 0x2, 0x51, "B3", Sequencer_DeleteTrack},
    { 0x3, 0x51, "B4", NULL},

    { 0x4, 0x51, "B5", Sequencer_ModeSolo},
    { 0x5, 0x51, "B6", Sequencer_ModeMute},
    { 0x6, 0x51, "B7", NULL},
    { 0x7, 0x51, "B8", NULL},

    { 0x1, 0x53, "B9", NULL},

    /* pedal */
    { 0x0, 0x0b, "VolumePedal", NULL},

    { 0x0, 0x11, "S1", NULL},
    { 0x1, 0x11, "S2", NULL},
    { 0x2, 0x11, "S3", NULL},
    { 0x3, 0x11, "S4", NULL},

    { 0x4, 0x11, "S5", NULL},
    { 0x5, 0x11, "S6", Sampler_SetDecay},
    { 0x6, 0x11, "S7", NULL},
    { 0x7, 0x11, "S8", NULL},
};


/*
 * this function will be called when a control change message has been received
 */
inline void Midi_ControlChange(uint8_t channel, uint8_t data1, uint8_t data2)
{
    for (int i = 0; i < (sizeof(edirolMapping) / sizeof(edirolMapping[0])); i++)
    {
        if ((edirolMapping[i].channel == channel) && (edirolMapping[i].data1 == data1) && (edirolMapping[i].callback != NULL))
        {
            edirolMapping[i].callback(channel, data1, data2);
        }
    }

#if 0 /* use this to identify cc messages */
    Serial.printf("CC %x %02x %02x\n", channel, data1, data2);
#endif
    if (data1 == 17)
    {
        if (channel < 10)
        {
            Synth_SetSlider(channel,  data2 * NORM127MUL);
        }
    }
    if ((data1 == 18) && (channel == 1))
    {
        Synth_SetSlider(8,  data2 * NORM127MUL);
    }

    if ((data1 == 16) && (channel < 9))
    {
        Synth_SetRotary(channel, data2 * NORM127MUL);

    }
    if ((data1 == 18) && (channel == 0))
    {
        Synth_SetRotary(8,  data2 * NORM127MUL);
    }
}

/*
 * function will be called when a short message has been received over midi
 */
inline void HandleShortMsg(uint8_t *data)
{
    uint8_t ch = data[0] & 0x0F;

    switch (data[0] & 0xF0)
    {
    /* note on */
    case 0x90:
        Midi_NoteOn(data[1], data[2]);
        break;
    /* note off */
    case 0x80:
        Midi_NoteOff(data[1]);
        break;
    case 0xb0:
        Midi_ControlChange(ch, data[1], data[2]);
        break;
    }
}

void Midi_Setup()
{
    Serial2.begin(31250, SERIAL_8N1, RXD2, TXD2);
    pinMode(RXD2, INPUT_PULLUP);  /* 25: GPIO 16, u2_RXD */
}

inline void Midi_LiveMessage(uint8_t msg)
{
    Sequencer_LiveMessage(msg);
}

/*
 * this function should be called continuously to ensure that incoming messages can be processed
 */
inline
void Midi_Process()
{
    /*
     * watchdog to avoid getting stuck by receiving incomplete or wrong data
     */
    static uint32_t inMsgWd = 0;
    static uint8_t inMsg[3];
    static uint8_t inMsgIndex = 0;

    //Choose Serial1 or Serial2 as required

    if (Serial2.available())
    {
        uint8_t incomingByte = Serial2.read();

#ifdef DUMP_SERIAL2_TO_SERIAL
        Serial.printf("%02x", incomingByte);
#endif
        /* ignore live messages */
        if ((incomingByte & 0xF0) == 0xF0)
        {
            Midi_LiveMessage(incomingByte);
            return;
        }

        if (inMsgIndex == 0)
        {
            if ((incomingByte & 0x80) != 0x80)
            {
                inMsgIndex = 1;
            }
        }

        inMsg[inMsgIndex] = incomingByte;
        inMsgIndex += 1;

        if (inMsgIndex >= 3)
        {
#ifdef DUMP_SERIAL2_TO_SERIAL
            Serial.printf(">%02x %02x %02x\n", inMsg[0], inMsg[1], inMsg[2]);
#endif
            HandleShortMsg(inMsg);
            inMsgIndex = 0;
        }

        /*
         * reset watchdog to allow new bytes to be received
         */
        inMsgWd = 0;
    }

    else
    {
        if (inMsgIndex > 0)
        {
            inMsgWd++;
            if (inMsgWd == 0xFFF)
            {
                inMsgIndex = 0;
            }
        }
    }

}

