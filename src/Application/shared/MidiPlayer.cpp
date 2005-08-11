/////////////////////////////////////////////////////////////////////////////
//
// File: MidiPlayer.cpp
//
// Date: Oct 29, 2001
//
// Author: Juergen Mellinger
//
// Description: The WIN32 implementation of the TMidiPlayer interface.
//
// Changes: Jan 9, 2004, juergen.mellinger@uni-tuebingen.de:
//           Added copy constructor, assignment operator and related private
//           member functions.
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include <assert>

#include "MidiPlayer.h"
#include "UBCIError.h"

int         TMidiPlayer::deviceUsage = 0;
HMIDIOUT    TMidiPlayer::deviceHandle = NULL;

bool        TMidiPlayer::channelsInUse[] =
{
    false,  // 1
    false,  // 2
    false,  // 3
    false,  // 4
    false,  // 5
    false,  // 6
    false,  // 7
    false,  // 8
    false,  // 9
    true,   // 10 is reserved for GM percussion
    false,  // 11
    false,  // 12
    false,  // 13
    false,  // 14
    false,  // 15
    false   // 16
};

TMidiPlayer::TMidiPlayer(   int inGMInstrument,
                            int inGMVolume,
                            int inMidiNote,
                            int inNoteLength,
                            int inBalance )
: gmInstrument( inGMInstrument - 1 ),
  gmVolume( inGMVolume ),
  curNote( inMidiNote ),
  curLength( inNoteLength ),
  gmBalance( inBalance ),
  seqTimerID( NULL ),
  noteSeq( NULL ),
  curSeqPos( NULL ),
  channel( GetChannel() )
{
  Construct();
}

TMidiPlayer::TMidiPlayer()
: gmInstrument( defaultInstrument - 1 ),
  gmVolume( defaultVolume ),
  curNote( defaultNote ),
  curLength( defaultLength ),
  gmBalance( defaultBalance ),
  seqTimerID( NULL ),
  noteSeq( NULL ),
  curSeqPos( NULL ),
  channel( GetChannel() )
{
  Construct();
}

TMidiPlayer::TMidiPlayer( const TMidiPlayer& inOriginal )
: gmInstrument( defaultInstrument - 1 ),
  gmVolume( defaultVolume ),
  curNote( defaultNote ),
  curLength( defaultLength ),
  gmBalance( defaultBalance ),
  seqTimerID( NULL ),
  noteSeq( NULL ),
  curSeqPos( NULL ),
  channel( GetChannel() )
{
  Construct();
  Assign( inOriginal );
}

TMidiPlayer&
TMidiPlayer::operator=( const TMidiPlayer& inOriginal )
{
  if( &inOriginal != this )
  {
    Destruct();
    Assign( inOriginal );
  }
  return *this;
}

TMidiPlayer::~TMidiPlayer()
{
  Destruct();
  ReleaseChannel( channel );
  ReleaseDevice();
}

void
TMidiPlayer::Construct()
{
    GetDevice();
    Initialize();
}

void
TMidiPlayer::Initialize()
{
    // Instruments ("Programs") are also numbered from 1 to 128 but represented from
    // 0 to 127.
    if( gmInstrument >= maxInstrument || gmInstrument < 0 )
        gmInstrument = defaultInstrument - 1;
    if( gmVolume > maxVolume || gmVolume < 0 )
        gmVolume = defaultVolume;
    if( curNote > maxNote || curNote < 0 )
        curNote = defaultNote;
    if( gmBalance > 127 || gmBalance < 0 )
        gmBalance = defaultBalance;

    if( curLength < 0 )
        curLength = 0;

    // Set the instrument for our channel.
    ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                    mProgramChange, channel, gmInstrument, mUnused ) );
    // If there is an error, we can't do much about it anyway.

    // Set the MIDI volume for our channel.
    // MSB of channel volume.
    ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                    mControlModeChange, channel, mChannelVolume | mMSB, gmVolume ) );

    // LSB of channel volume (always 0).
    ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                    mControlModeChange, channel, mChannelVolume | mLSB, 0 ) );

    // MSB of channel balance.
    ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                    mControlModeChange, channel, mPan | mMSB, gmBalance ) );

    // LSB of channel balance (always 0).
    ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                    mControlModeChange, channel, mPan | mLSB, 0 ) );
}

void
TMidiPlayer::Destruct()
{
    // Stop sequence processing.
    if( seqTimerID != NULL )
    {
        ::timeKillEvent( seqTimerID );
        seqTimerID = NULL;
    }
    if( noteSeq != NULL )
    {
        ::free( noteSeq );
        noteSeq = NULL;
        curSeqPos = NULL;
    }

    // Stop all playing notes.
    ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                    mControlModeChange, channel, mAllNotesOff, 0 ) );
}

void
TMidiPlayer::Assign( const TMidiPlayer& inOriginal )
{
  gmInstrument = inOriginal.gmInstrument;
  gmVolume = inOriginal.gmVolume;
  gmBalance = inOriginal.gmBalance;
  curNote = inOriginal.curNote;
  curLength = inOriginal.curLength;
  Initialize();
}

void
TMidiPlayer::Play(  int inMidiNote,
                    int inNoteLength,
                    int inVelocity )
{
    curNote = inMidiNote;
    curLength = inNoteLength;
    if( curLength == 0 )
        return;
    if( curNote > maxNote || curNote < 0 )
        curNote = defaultNote;

    if( deviceHandle == NULL )
        return;

    if( inVelocity > maxVelocity )
      inVelocity = maxVelocity;
    if( inVelocity < 0 )
      inVelocity = 0;

    // Switch the MIDI note on.
    ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                    mNoteOn, channel, curNote, inVelocity ) );

    // Set up the timer event that will switch the note off after
    // curLength milliseconds.
    DWORD       msg = ShortMidiMsg( mNoteOff, channel, curNote, 0 );
    MMRESULT    timerID = ::timeSetEvent( curLength, timerResolution,
                            SendMidiMsg, msg, TIME_ONESHOT );
    if( timerID == NULL )
      // Setting the timer didn't work; switch the note off at once.
      ::midiOutShortMsg( deviceHandle, msg );
}


void
CALLBACK
TMidiPlayer::SendMidiMsg( UINT inTimerID, UINT, DWORD inMsg, DWORD, DWORD )
{
    // If the last instance of TMidiPlayer was deleted since
    // the timer was set up,
    // the device will have been closed and the device handle will be invalid.
    // This should be ok.
    MMRESULT    err = ::midiOutShortMsg( deviceHandle, inMsg );

    if( err == MIDIERR_NOTREADY )
    // The device is busy with something else. This should only happen at interrupt time
    // if the timer interrupt intercepts another call to SendMidiMsg.
    // We need to try again. We can't use a while loop here because while we are inside
    // the interrupt handler the pending SendMidiMsg call cannot return.
    // We set up a new timer and hope that this will not happen too often.
        ::timeSetEvent( retryDelay, timerResolution,
                            SendMidiMsg, inMsg, TIME_ONESHOT );
}

int
TMidiPlayer::GetChannel()
{
    int outChannel = 0;
    // Channels are numbered from 1 to 16 but their binary representation
    // is from 0 to 15.

    // When all channels are used up new midi players will be assigned channel 16.
    // This may lead to undesired effects.
    while( channelsInUse[ outChannel ] && ( outChannel < 15 ) )
        outChannel++;

    if( channelsInUse[ outChannel ] )
        bciout << "A MIDI channel is used by more than one TMidiPlayer instance"
               << std::endl;

    channelsInUse[ outChannel ] = true;
    return outChannel;
}

void
TMidiPlayer::ReleaseChannel( int inChannel )
{
    if( inChannel >= 0 && inChannel < 16 && inChannel != 9 )
        channelsInUse[ inChannel ] = false;
}

void*
TMidiPlayer::GetDevice()
{
    if( deviceHandle == NULL )
    {
        MMRESULT    err;
        err = ::midiOutOpen( &deviceHandle, MIDI_MAPPER, NULL, 0, CALLBACK_NULL );
        if( err != MMSYSERR_NOERROR )
            deviceHandle = NULL;
    }
    deviceUsage++;
    return ( void* )deviceHandle;
}

void
TMidiPlayer::ReleaseDevice()
{
    assert( deviceUsage > 0 && "Someone called ReleaseDevice() more often than GetDevice()" );
    deviceUsage--;
    if( deviceUsage < 1 )
    {
        ::midiOutClose( deviceHandle );
        deviceHandle = NULL;
    }
}

// Sequence playing functions.
void
TMidiPlayer::PlaySequence( const TMidiNote* inNoteSequence )
{
    StopSequence();
    if( inNoteSequence->duration == 0 ) // No notes to be played.
        return;
    // Count the length of the sequence and get memory for a copy.
    // We copy the sequence because we don't want to risk invalid
    // pointers inside the callback function.
    const TMidiNote *p = inNoteSequence;
    while( p->duration != 0 )
        ++p;
    int numNotes = p - inNoteSequence;
    noteSeq = ( TMidiNote* )::malloc( sizeof( TMidiNote ) * ( numNotes + 1 ) );
    if( noteSeq == NULL )
    {
      bcierr << "Out of memory" << std::endl;
      return;
    }
    ::memcpy( noteSeq, inNoteSequence, sizeof( TMidiNote ) * ( numNotes + 1 ) );

    // Start playing the sequence.
    curSeqPos = noteSeq;
    // If the first note isn't a pause, send a note on message for it.
    if( ( curSeqPos->note & ( 1 << 8 ) ) == 0 )
    {
        MMRESULT    err;
        err = ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                                        mNoteOn,
                                        channel,
                                        curSeqPos->note,
                                        midiVelocity ) );
        if( err != MMSYSERR_NOERROR )
        {
            curSeqPos = NULL;
            ::free( noteSeq );
            noteSeq = NULL;
            return;
        }
    }
    // Set up the timer for processing the next note.
    UINT    timerID = ::timeSetEvent( curSeqPos->duration, timerResolution,
                                        SeqCallback, ( DWORD )this, TIME_ONESHOT );
    seqTimerID = timerID;
    if( timerID == NULL ) // Setting the timer didn't work; switch the note off at once.
    {
        ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                                        mNoteOff,
                                        channel,
                                        curSeqPos->note,
                                        midiVelocity ) );
        curSeqPos = NULL;
        ::free( noteSeq );
        noteSeq = NULL;
    }
}

void
TMidiPlayer::StopSequence()
{
    // Stop sequence processing.
    if( seqTimerID != NULL )
    {
        ::timeKillEvent( seqTimerID );
        seqTimerID = NULL;
    }
    if( noteSeq != NULL )
    {
        ::free( noteSeq );
        noteSeq = NULL;
    }
    curSeqPos = NULL;

    // Stop all playing notes.
    ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                    mControlModeChange, channel, mAllNotesOff, 0 ) );
}

void
CALLBACK
TMidiPlayer::SeqCallback( UINT inID, UINT, DWORD inInstance, DWORD, DWORD )
{
    TMidiPlayer *caller = ( TMidiPlayer* )inInstance;
    TMidiNote   *curNote = caller->curSeqPos;
    if( curNote == NULL ) // This should not happen, but anyway...
        return;

    MMRESULT err;
    // If the current note isn't a pause, send a note off message for the current note.
    if( ( curNote->note & ( 1 << 8 ) ) == 0 )
    {
        err = ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                                    mNoteOff,
                                    caller->channel,
                                    curNote->note,
                                    midiVelocity ) );
        if( err != MMSYSERR_NOERROR )
        {
            caller->seqTimerID = NULL;
            caller->curSeqPos = NULL;
            return;
        }
    }
    // Get the next note.
    curNote = ++caller->curSeqPos;
    if( curNote->duration == 0 )
    {
        // End of sequence reached, stop.
        caller->seqTimerID = NULL;
        caller->curSeqPos = NULL;
        return;
    }
    // If the next note isn't a pause, send a note on message for it.
    if( ( curNote->note & ( 1 << 8 ) ) == 0 )
    {
        err = ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                                        mNoteOn,
                                        caller->channel,
                                        curNote->note,
                                        midiVelocity ) );
        if( err != MMSYSERR_NOERROR )
        {
            caller->seqTimerID = NULL;
            caller->curSeqPos = NULL;
            return;
        }
    }
    // Set up the timer for processing the next note.
    UINT    timerID = ::timeSetEvent( curNote->duration, timerResolution,
                                        SeqCallback, inInstance, TIME_ONESHOT );
    caller->seqTimerID = timerID;
    if( timerID == NULL ) // Setting the timer didn't work; switch the note off at once.
        ::midiOutShortMsg( deviceHandle, ShortMidiMsg(
                                        mNoteOff,
                                        caller->channel,
                                        curNote->note,
                                        midiVelocity ) );
}

