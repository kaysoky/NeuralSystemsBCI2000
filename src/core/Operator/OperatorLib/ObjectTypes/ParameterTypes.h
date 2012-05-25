////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Parameter-related object types for the script interpreter.
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
#ifndef PARAMETER_TYPES_H
#define PARAMETER_TYPES_H

#include "ObjectType.h"
#include "ParamRef.h"

namespace Interpreter {

class ParameterType : public ObjectType
{
 protected:
  virtual const char* Name() const { return "Parameter"; }
  virtual const MethodEntry* MethodTable() const { return sMethodTable; }

 public:
  static bool Set( CommandInterpreter& );
  static bool Get( CommandInterpreter& );
  static bool Insert( CommandInterpreter& );
  static bool List( CommandInterpreter& );
  static bool Exists( CommandInterpreter& );

 private:
  static ParamRef GetParamRef( CommandInterpreter& );
  static size_t GetIndex( const std::string&, const LabelIndex& );

  static const MethodEntry sMethodTable[];
  static ParameterType sInstance;
};

class ParametersType : public ObjectType
{
 protected:
  virtual const char* Name() const { return "Parameters"; }
  virtual const MethodEntry* MethodTable() const { return sMethodTable; }

 public:
  static bool Load( CommandInterpreter& );
  static bool List( CommandInterpreter& );
  static bool Apply( CommandInterpreter& );
  static bool Clear( CommandInterpreter& );

 private:
  static const MethodEntry sMethodTable[];
  static ParametersType sInstance;
};

class ParameterfileType : public ObjectType
{
 protected:
  virtual const char* Name() const { return "ParameterFile"; }
  virtual const MethodEntry* MethodTable() const { return sMethodTable; }

 public:
  static bool Load( CommandInterpreter& );

 private:
  static const MethodEntry sMethodTable[];
  static ParameterfileType sInstance;
};

} // namespace

#endif // PARAMETER_TYPES_H
