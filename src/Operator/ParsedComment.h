//////////////////////////////////////////////////////////////////////
//
// File: ParsedComment.h
//
// Date: Dec 30, 2004
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A class that handles parsing a parameter's comment
//       for display purposes.
//
///////////////////////////////////////////////////////////////////////
#ifndef ParsedCommentH
#define ParsedCommentH

#include <string>
#include <vector>

class PARAM;

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

      listGeneric,
      matrixGeneric,
    } Kind_type;
  private:
    typedef std::vector<std::string> Values_type;

  // The public interface.
  public:
    ParsedComment( const PARAM& );
    // The parameter name.
    const std::string& Name() const      { return mName; }
    // The parameter comment as modified by the interpretation.
    const std::string& Comment() const   { return mComment; }

    Kind_type          Kind() const      { return mKind; }
    const Values_type& Values() const    { return mValues; }

    // This is only relevant for the singleValuedEnum type and represents
    // the numerical parameter value of the first enumeration entry.
    int                IndexBase() const { return mIndexBase; }

  // Private members.
  private:
    bool ExtractEnumValues( const PARAM& p );
    bool IsBooleanEnum() const;

    std::string mName;
    std::string mComment;
    Kind_type   mKind;
    Values_type mValues;
    int         mIndexBase;
};

#endif // ParsedCommentH
