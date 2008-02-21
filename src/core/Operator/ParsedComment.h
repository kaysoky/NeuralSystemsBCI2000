//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that handles parsing a parameter's comment
//       for display purposes.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef ParsedCommentH
#define ParsedCommentH

#include <string>
#include <vector>

class Param;

class ParsedComment
{
  // Type declarations.
  public:
    // Possible interpretation results.
    typedef enum
    {
      unknown = 0,
      singleEntryGeneric,
        singleEntryEnum,      // Possible parameter values are from a pre-defined set.
        singleEntryBoolean,   // The parameter represents an on/off switch.
                              // Logically, this is a specialization of singleValuedEnum.
        singleEntryInputFile, // Parameter values are full paths to files.
        singleEntryOutputFile,
        singleEntryDirectory, // Parameter values are directory paths.
        singleEntryColor,     // Parameter values are RGB colors in hex string notation.

      listGeneric,
      matrixGeneric,
    } Kind_type;
  private:
    typedef std::vector<std::string> Values_type;

  // The public interface.
  public:
    ParsedComment( const Param& );
    // The parameter name.
    const std::string& Name() const        { return mName; }
    // A parameter's help context, typically derived from its subsection.
    const std::string& HelpContext() const { return mHelpContext; }
    // The parameter comment as modified by the interpretation.
    const std::string& Comment() const     { return mComment; }

    Kind_type          Kind() const        { return mKind; }
    const Values_type& Values() const      { return mValues; }

    // This is only relevant for the singleValuedEnum type and represents
    // the numerical parameter value of the first enumeration entry.
    int                IndexBase() const   { return mIndexBase; }

  // Private members.
  private:
    bool ExtractEnumValues( const Param& p );
    bool IsBooleanEnum() const;

    std::string mName,
                mHelpContext,
                mComment;
    Kind_type   mKind;
    Values_type mValues;
    int         mIndexBase;
};

#endif // ParsedCommentH
