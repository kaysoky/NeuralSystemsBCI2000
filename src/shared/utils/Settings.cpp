////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A descendant of the Qt QSettings class with a default
//   constructor that reads/writes from/to an ini file in the application
//   directory.
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
#include "Settings.h"
#include <QCoreApplication>
#include <QDir>

QString Settings::sFilePath;

Settings::Settings()
: QSettings( sFilePath, QSettings::IniFormat )
{
}

void
Settings::SetFile( const QString& inFilePath )
{
  if( inFilePath == "" )
    sFilePath = QCoreApplication::applicationDirPath() 
              + QDir::separator() 
              + QCoreApplication::applicationName() 
              + ".ini";
  else
    sFilePath = inFilePath;
}
