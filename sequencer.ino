/*
 * this file includes the implementation of a simple sequencer
 *
 * Author: Marcel Licence
 */

#define SEQ_TRACK_CNT	12
#define SEQ_STEPS		16

/*
 * 1: only 4th notes
 * 2: for 8th notes
 */
#define SEQ_SUBSTEP_MUL	2

struct seq_track_s
{
    uint8_t note; /*!< associated note to play of the track */
    uint8_t sequence[SEQ_STEPS]; /*!< sequence of this track containing velocity values */

    bool solo; /*!< play sound if active and only other solo tracks */
    bool mute; /*!< do not play sound of this track */
};

enum seqModeE
{
    seq_mode_idle,
    seq_mode_record,
    seq_mode_delete,
    seq_mode_solo,
    seq_mode_mute,
};

static enum seqModeE seqMode = seq_mode_record;

struct seq_track_s seq_track[SEQ_TRACK_CNT];

void Sequencer_Init(void)
{
    memset(seq_track, 0, sizeof(seq_track));

    for (int i = 0; i < SEQ_TRACK_CNT; i++)
    {
        seq_track[i].note = i;
        seq_track[i].mute = false;
        seq_track[i].solo = false;
    }
}

#define DEFAULT_BPM		140

/*
 * using * 2 would allow playing 8th
 *
 * otherwise we have 140BPM means 140 4th per Bar
 */

uint32_t seq_prescaler = 60.0f * 44100.0f / (2.0f * SEQ_SUBSTEP_MUL * (float)DEFAULT_BPM);
uint32_t seq_prescaler_next = 0;
uint32_t seq_pos = 0;
uint32_t seq_counter = 0;
bool seq_click = true;
bool seq_active = true;
float seq_shuffle = 0.5;


inline void Sequencer_Stop(uint8_t ch, uint8_t data1, uint8_t data2)
{
    seq_active = false;
}

inline void Sequencer_Start(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        seq_counter = 0;
        seq_pos = 0;
        seq_active = true;
    }
}


inline void Sequencer_LiveMessage(uint8_t msg)
{
    //Serial.printf("LiveMsg: %02x\n", msg);
}


inline
void Sequencer_Process(float *left, float *right)
{
    if (seq_active)
    {
        seq_counter ++;

        if (seq_counter > seq_prescaler_next)
        {
            seq_counter = 0;
            Sequencer_Tick();

            /* using (seq_pos & 4) == 0 allows nice rhythm manipulation */
            float add = ((seq_pos + 1) & 2) == 0 ? (2 * (1 - seq_shuffle)) : (2 * seq_shuffle);

            seq_prescaler_next = add * seq_prescaler;
        }

        if (seq_click)
        {
            if (seq_pos == 1)
            {
                *left += ((float)((seq_counter & 32) / 32)) * 0.1f;
                *right += ((float)((seq_counter & 32) / 32)) * 0.1f;
            }
            else if ((seq_pos % (4 * SEQ_SUBSTEP_MUL)) == 1)
            {
                *left += ((float)((seq_counter & 64) / 64)) * 0.1f;
                *right += ((float)((seq_counter & 64) / 64)) * 0.1f;
            }
        }
    }
}

uint8_t step_divider = 0;

inline
void Sequencer_Tick(void)
{
    uint8_t soloCnt = 0;

    seq_pos += 1;
    if (seq_pos >= ((SEQ_STEPS * 2) >> step_divider))
    {
        seq_pos = 0;
    }

    //Serial.printf("%d, %d, %d\n", seq_pos, seq_pos>>1, seq_pos%2);
    if ((seq_pos % 2) == 1)
    {
        for (int i = 0; i < SEQ_TRACK_CNT; i++)
        {
            if (seq_track[i].solo)
            {
                soloCnt += 1;
            }
        }

        for (int i = 0; i < SEQ_TRACK_CNT; i++)
        {
            /*
             * process channel if there is no solo active and not muted
             *
             * or it has been set to solo
             */
            if (((soloCnt == 0) && (seq_track[i].mute == false)) || (seq_track[i].solo == true))
            {
                Sequencer_TrackProcess(&seq_track[i]);
            }
        }
    }
}

static bool ignore = false; /* maybe useless in this version, was required in previous code */

inline void Sequencer_TrackProcess(struct seq_track_s *track)
{
    if (track->sequence[seq_pos >> 1] > 0)
    {
        ignore = true;
        Sampler_NoteOn(track->note, track->sequence[seq_pos >> 1]);
        ignore = false;
    }
}

inline void Sequencer_NoteOn(uint8_t note, uint8_t vol)
{
    /* do not play self triggered notes */
    if (ignore)
    {
        return;
    }
    note = note % SEQ_TRACK_CNT;

    if (seqMode == seq_mode_record)
    {
        seq_track[note].sequence[seq_pos >> 1] = vol;

        if ((seq_pos % 2) == 1) /* avoid fast delay */
        {
            Sampler_NoteOn(note, vol);
        }
    }

    if (seqMode == seq_mode_delete)
    {
        for (int i = 0; i < SEQ_STEPS; i++)
        {
            seq_track[note].sequence[i] = 0;
        }
    }

    if (seqMode ==  seq_mode_idle)
    {
        Sampler_NoteOn(note, vol);
    }

    if (seqMode == seq_mode_solo)
    {
        Serial.printf("Seq Solo on: %d\n", note);
        seq_track[note].solo = true;
    }

    if (seqMode == seq_mode_mute)
    {
        Serial.printf("Seq Mute on: %d\n", note);
        seq_track[note].mute = true;
    }
}

inline void Sequencer_NoteOff(uint8_t note)
{
    note = note % SEQ_TRACK_CNT;

    if (seqMode == seq_mode_solo)
    {
        Serial.printf("Seq Solo off: %d\n", note);
        seq_track[note].solo = false;
    }
    if (seqMode == seq_mode_mute)
    {
        Serial.printf("Seq Mute off: %d\n", note);
        seq_track[note].mute = false;
    }
}

inline void Sequencer_SetSpeed(float value)
{
    float min_val = 60;
    float max_val = 240;

    value = min_val + value * (max_val - min_val);

    seq_prescaler = 60.0f * 44100.0f / (2.0f * SEQ_SUBSTEP_MUL * value);

    Serial.printf("Sequencer_SetSpeed: %d\n", (uint32_t)value);
}

inline void Sequencer_SetShuffle(float value)
{
    seq_shuffle = 0.25f + (value * 0.5f);

    Serial.printf("Sequencer_SetShuffle: %0.2f\n", seq_shuffle);
}

void Sequencer_ClickOnOff(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Sequencer_ClickOnOff\n");
        seq_click = seq_click ? false : true;
    }
}

void Sequencer_ClearAll(uint8_t ch, uint8_t data1, uint8_t data2)
{
    Serial.printf("Sequencer_ClearAll\n");
    for (int i = 0; i < SEQ_TRACK_CNT; i++)
    {
        memset(&seq_track[i].sequence, 0, sizeof(seq_track[i].sequence));
    }
}

void Sequencer_ModeIdle(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq Mode Idle\n");
        seqMode = seq_mode_idle;
    }
}

void Sequencer_ModeRecord(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq Mode Record\n");
        seqMode = seq_mode_record;
    }
}

void Sequencer_DeleteTrack(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq Mode Delete\n");
        seqMode = seq_mode_delete;
    }
}

void Sequencer_ModeSolo(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq Mode Solo\n");
        seqMode = seq_mode_solo;
    }
}

void Sequencer_ModeMute(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq Mode Mute\n");
        seqMode = seq_mode_mute;
    }
}

void Sequencer_Max1(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq step div 1\n");
        step_divider = 1;
        seq_pos = 0;
    }
}

void Sequencer_Max2(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq step div 2\n");
        step_divider = 2;
        seq_pos = 0;
    }
}

void Sequencer_Max3(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq step div 3\n");
        step_divider = 3;
        seq_pos = 0;
    }
}

void Sequencer_Max4(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq step div 4\n");
        step_divider = 4;
        seq_pos = 0;
    }
}

void Sequencer_Max5(uint8_t ch, uint8_t data1, uint8_t data2)
{
    if (data2 > 0)
    {
        Serial.printf("Seq complete seq\n");
        step_divider = 0;
        seq_pos = 0;
    }
}
