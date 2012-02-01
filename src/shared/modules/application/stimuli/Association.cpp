////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Association is a class that associates sets of stimuli with
//   sets of targets.
//   AssociationMap is a map that maps stimulus codes to Associations, and
//   sorts targets according to classification results given over stimulus
//   codes.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Association.h"
#include <algorithm>

// Association
Association::Association()
: mStimulusDuration( -1 ),
  mISIMinDuration( -1 ),
  mISIMaxDuration( -1 )
{
}

Association&
Association::SetStimulusDuration( int i )
{
  mStimulusDuration = i;
  return *this;
}

int
Association::StimulusDuration() const
{
  return mStimulusDuration;
}

Association&
Association::SetISIMinDuration( int i )
{
  mISIMinDuration = i;
  return *this;
}

int
Association::ISIMinDuration() const
{
  return mISIMinDuration;
}

Association&
Association::SetISIMaxDuration( int i )
{
  mISIMaxDuration = i;
  return *this;
}

int
Association::ISIMaxDuration() const
{
  return mISIMaxDuration;
}

Association&
Association::Clear()
{
  mStimuli.clear();
  mTargets.clear();
  return *this;
}

Association&
Association::DeleteObjects()
{
  mStimuli.DeleteObjects();
  mTargets.DeleteObjects();
  return *this;
}

Association&
Association::Add( Stimulus* s )
{
  mStimuli.Add( s );
  return *this;
}

Association&
Association::Remove( Stimulus* s )
{
  mStimuli.Remove( s );
  return *this;
}

bool
Association::Contains( Stimulus* s ) const
{
  return mStimuli.Contains( s );
}

bool
Association::Intersects( const SetOfStimuli& s ) const
{
  return mStimuli.Intersects( s );
}

Association&
Association::Add( Target* t )
{
  mTargets.Add( t );
  return *this;
}

Association&
Association::Remove( Target* t )
{
  mTargets.Remove( t );
  return *this;
}

bool
Association::Contains( Target* t ) const
{
  return mTargets.Contains( t );
}

bool
Association::Intersects( const SetOfTargets& s ) const
{
  return mTargets.Intersects( s );
}

Association&
Association::Present()
{
  mStimuli.Present();
  return *this;
}

Association&
Association::Conceal()
{
  mStimuli.Conceal();
  return *this;
}

Association&
Association::Select()
{
  mTargets.Select();
  return *this;
}

SetOfStimuli&
Association::Stimuli()
{
  return mStimuli;
}

const SetOfStimuli&
Association::Stimuli() const
{
  return mStimuli;
}

SetOfTargets&
Association::Targets()
{
  return mTargets;
}

const SetOfTargets&
Association::Targets() const
{
  return mTargets;
}

// TargetClassification
Target*
TargetClassification::MostLikelyTarget() const
{
  Target* result = empty() ? NULL : begin()->first;
  double maxValue = empty() ? 0 : begin()->second;
  const_iterator i = begin();
  while( i != end() )
  {
    if( i->second > maxValue )
    {
      result = i->first;
      maxValue = i->second;
    }
    ++i;
  }
  return result;
}

// AssociationMap
//
// Given classifier output over stimulus codes, channels, and epochs,
// determine a score value for each target.
// For each target, we compute the average score value taken over all
// stimulus codes, channels, and epoch results.
// When scores are outputs of a linear classifier, they are linear
// functions of log-likelihood ratios, so selecting the target with the
// largest score minimizes overall misclassification risk.
TargetClassification
AssociationMap::ClassifyTargets( const ClassResult& inResult )
{
  TargetClassification result,
                       count;
  for( ClassResult::const_iterator i = inResult.begin(); i != inResult.end(); ++i )
  {
    int stimulusCode = i->first;
    const SetOfTargets& targets = ( *this )[ stimulusCode ].Targets();
    const ClassResult::mapped_type& stimResults = i->second;
    for( SetOfTargets::const_iterator j = targets.begin(); j != targets.end(); ++j )
    {
      Target* pTarget = *j;
      for( size_t epoch = 0; epoch < stimResults.size(); ++epoch )
        for( int channel = 0; channel < stimResults[ epoch ].Channels(); ++channel )
          for( int element = 0; element < stimResults[ epoch ].Elements(); ++element )
        {
          result[ pTarget ] += stimResults[ epoch ]( channel, element );
          ++count[ pTarget ];
        }
    }
  }
  for( TargetClassification::iterator i = result.begin(); i != result.end(); ++i )
    i->second /= count[ i->first ];
  return result;
}

// Intersection of all sets of stimuli that are associated with a given target.
SetOfStimuli
AssociationMap::TargetToStimuli( Target* in_pTarget ) const
{
  SetOfStimuli result;
  const_iterator i = begin();
  while( !i->second.Contains( in_pTarget ) && i != end() )
    ++i;
  if( i != end() )
    result = i->second.Stimuli();
  while( i != end() )
  {
    if( i->second.Contains( in_pTarget ) )
      for( SetOfStimuli::iterator j = result.begin(); j != result.end(); )
        /* Only works in VS
        if( !i->second.Stimuli().Contains( *j ) )
          j = result.erase( j );
        else
          ++j;
        */
        // This appears to be portable
        if( !i->second.Stimuli().Contains( *j ) )
          result.erase( j++ );
        else
          ++j;
    ++i;
  }
  return result;
}

bool
AssociationMap::StimuliIntersect( int s1, int s2 )
{
  return s1 == s2 || ( *this )[ s1 ].Intersects( ( *this )[ s2 ].Stimuli() );
}

bool
AssociationMap::TargetsIntersect( int s1, int s2 )
{
  return s1 == s2 || ( *this )[ s1 ].Intersects( ( *this )[ s2 ].Targets() );
}


