////////////////////////////////////////////////////////////////////////////////
// $Id: VisDisplayLayer.cpp 3307 2011-06-03 18:30:49Z mellinger $
// Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
// Description: Defines a layer of rendering into a VisDisplayWindow
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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

#include "VisDisplayLayer.h"
#include "BciError.h"
#include <QLayout>

using namespace std;

VisDisplayLayer::VisDisplayLayer( const std::string& inVisID )
: VisDisplayBase( inVisID )
{
  // Set widget properties and add it to the window 
  this->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  this->parentWidget()->layout()->addWidget( this );
  SetConfig( Visconfigs()[ mVisID ] );

  // Sort sibling layers
  VisContainerBase layers;
  for( VisContainerBase::iterator vitr = Visuals().begin(); vitr != Visuals().end(); vitr++ )
    if( vitr->first.find( mVisID.DominatingLayerVisID() ) != string::npos )
      layers[ VisID( vitr->first ).LayerID() ] = vitr->second; 
  for( VisContainerBase::iterator sitr = layers.begin(); sitr != layers.end(); sitr++ )
    if( sitr->second ) sitr->second->raise();
}

VisDisplayLayer::~VisDisplayLayer()
{
  // Determine if this was the last layer and remove the window if necessary
  string base = mVisID.substr( 0, mVisID.find( ":" ) + 1 );
  VisContainerBase layers;
  for( VisContainerBase::iterator vitr = Visuals().begin(); vitr != Visuals().end(); vitr++ )
    if( vitr->first != mVisID && vitr->first.find( base ) != string::npos )
      layers[ vitr->first.substr( vitr->first.find( ":" ) ).substr( 1 ) ] = vitr->second; 
  if( layers.empty() )
  {
    Visuals()[ base ]->deleteLater();
    Visuals()[ base ] = NULL;
  }
}