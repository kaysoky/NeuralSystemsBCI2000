/////////////////////////////////////////////////////////////////////////////
//
// File: MidiPlayer.h
//
// Date: Oct 29, 2001
//
// Author: Juergen Mellinger
//
// Description: A MIDI output interface class.
//
// Changes:
//
//////////////////////////////////////////////////////////////////////////////

#ifndef MIDIPLAYERH
#define MIDIPLAYERH

#include <mmsystem.h>

#define LITTLE_ENDIAN 1

class TMidiPlayer
{
  public:
    static const int    maxInstrument = 128,
                        maxVolume = 127,
                        maxNote = 127,
                        maxVelocity = 127,
                        defaultInstrument = 1, // Grand Piano
                        defaultVolume = 127,   // Maximum volume
                        defaultNote = 60,      // c'
                        defaultBalance = 64,   // center balance
                        defaultLength = 100,   // in ms
                        timerResolution = 10,  // accuracy of the note length timer
                        retryDelay = 10,       // retry delay for sending a MIDI message
                                               // when a midi device is busy
                        midiVelocity = 64;     // a mean velocity

    enum EMidiStatus
    {
        mUnused = 0x00,
        mNoteOff = 0x80,
        mNoteOn = 0x90,
        mPolyphonicAftertouch = 0xa0,
        mControlModeChange = 0xb0,
        mProgramChange = 0xc0,
        mChannelAftertouch = 0xd0,
        mPitchWheelControl = 0xe0,
        mSystem = 0xf0,
        mSystemReset = 0xff
    };

    enum EMidiControl
    {
        mMSB = 0x00,
        mLSB = 0x20,
        mChannelVolume = 0x07,
        mBalance = 0x08,
        mPan = 0x0a,
        mAllNotesOff = 0x7b
    };

    typedef struct TMidiNote
    {
        signed char     note;       // Midi note, set bit 7 for a pause (e.g., by setting note to -1)
        unsigned short  duration;   // Length in ms;
                                    // specify a length of 0 for the terminating element in a sequence.
    } TMidiNote;

                        TMidiPlayer( int inGMInstrument = defaultInstrument,
                                     int inGMVolume = defaultVolume,
                                     int inMidiNote = defaultNote,
                                     int inNoteLength = defaultLength,
                                     int inBalance = defaultBalance );

    virtual             ~TMidiPlayer();

                void    Play();
                void    Play(   int inMidiNote );
                void    Play(   int inMidiNote,
                                int inNoteLength,
                                int inVelocity = midiVelocity );
                // Play a sequence of MIDI notes.
                void    PlaySequence( const TMidiNote*  inNoteSequence );
                // Stop playing a sequence.
                void    StopSequence();

        // Returns a free channel and marks it as used.
        static  int     GetChannel();
        static  void    ReleaseChannel( int inChannel );

        // Returns an OS handle for the standard MIDI device.
        static  void*   GetDevice();
        static  void    ReleaseDevice();

        // A utility function for specifying MIDI messages in human readable
        // form; defined inline below.
        static  long    ShortMidiMsg(   int inStatus,
                                        int inChannel,
                                        int inArg1,
                                        int inArg2 );

  private:

            int         gmInstrument,
                        gmVolume,
                        gmBalance,
                        curNote,
                        curLength,
                        channel;

    // Every instance gets its own MIDI channel until all 15 available channels
    // are used up.
    static  bool        channelsInUse[];

    static  int         deviceUsage;

    // The timer callback function to switch off a note after its
    // duration has expired.
    static  void    CALLBACK SendMidiMsg(   UINT inID,
                                            UINT,
                                            DWORD inMsg,
                                            DWORD,
                                            DWORD );
    // Playing a sequence of notes:
    // WIN32 MIDI streaming does not work reliably - we need to do this
    // ourselves.

    // The timer callback function to send the next message when playing
    // a sequence.
    static  void    CALLBACK SeqCallback(   UINT inID,
                                            UINT,
                                            DWORD inInstance,
                                            DWORD,
                                            DWORD );
            UINT        seqTimerID;
            TMidiNote   *noteSeq,
                        *curSeqPos;

    static  HMIDIOUT    deviceHandle;
};


inline
long
TMidiPlayer::ShortMidiMsg(  int inStatus,
                            int inChannel,
                            int inArg1,
                            int inArg2 )
{
#if LITTLE_ENDIAN
    return inStatus | inChannel | ( inArg1 << 8 ) | ( inArg2 << 16 );
#else
    return ( ( inStatus | inChannel ) << 24 ) | ( inArg1 << 16 ) | ( inArg2 << 8 );
#endif // LITTLE_ENDIAN
}

inline
void
TMidiPlayer::Play()
{
    Play( curNote, curLength );
}

inline
void
TMidiPlayer::Play( int inMidiNote )
{
    Play( inMidiNote, curLength );
}

#endif // MIDIPLAYERH
