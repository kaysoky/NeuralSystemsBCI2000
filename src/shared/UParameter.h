/******************************************************************************
 * Program:   BCI2000                                                         *
 * Module:    UParameter.h                                                  *
 * Comment:   This unit provides support for system-wide parameters           *
 *            and parameter lists                                             *
 * Version:   0.22                                                            *
 * Authors:   Gerwin Schalk, Juergen Mellinger                                *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.08 - 03/30/2000 - First commented version                               *
 * V0.10 - 05/12/2000 - added AddParam2List(char *paramline)                  *
 *                      CloneParameter2List now updates the values of an      *
 *                      existing parameter                                    *
 * V0.11 - 06/09/2000 - Archive flag to each parameter                        *
 *                      better validity check in ParseParameter()             *
 * V0.13 - 08/09/2000 - Parameter supports datatype matrix                    *
 * V0.14 - 09/25/2000 - load and save parameter files                         *
 * V0.16 - 04/30/2001 - sorting of parameter lists; numerous other changes    *
 * V0.17 - 06/20/2002 - introduction of const, private/protected              *
 *                      juergen.mellinger@uni-tuebingen.de                    *
 * V0.18 - 01/31/2003 - fixed bug in SaveParameterList()                      *
 * V0.19 - 01/09/2003 - completely rewrote implementation based on STL,       *
 *                      juergen.mellinger@uni-tuebingen.de                    *
 * V0.20 - 05/07/2003 - Added textual index labels for matrices and lists, jm *
 * V0.21 - 05/15/2003 - Fixed invalid iterator problem in SaveParameterList(),*
 *                      jm                                                    *
 * V0.22 - 05/30/2003 - Fixed index synchronization bug in                    *
 *                      PARAM::SetNumValues(), jm                             *
 * V0.23 - 11/24/2003 - Fixed parsing of matrices with 0x0 size               *
 *                      Preserve existing values in SetDimensions, jm         *
 * V0.24 - 11/28/2003 - Added aliases for some functions that contain         *
 *                      dimension names, e.g. PARAM::GetNumRows(),            *
 *                      PARAM::RowLabels(), jm                                *
 ******************************************************************************/
#ifndef UParameterH
#define UParameterH

#include <locale>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdlib.h>

class PARAM
{
  friend class PARAMLIST;

 public:
   // A string class that allows for transparent handling of
   // character codes using the % sign.
   class encodedString : public std::string
   {
    public:
     encodedString() {}
     encodedString( const std::string& s ) : std::string( s ) {}
     encodedString( const char* s ) : std::string( s ) {}
     std::ostream& WriteToStream( std::ostream&, const std::string& = "" ) const;
     std::istream& ReadFromStream( std::istream& );
   };

   // A helper class to handle string labels for indexing matrices
   // and lists.
   typedef std::map<encodedString, size_t> indexer_base;
   typedef std::vector<indexer_base::key_type> indexer_reverse;
   class labelIndexer
   {
     friend class PARAM;
     private:
      labelIndexer() : needSync( false ) { clear(); }
      ~labelIndexer() {}
      
     public:
      // Forward lookup.
      indexer_base::mapped_type operator[]( const std::string& ) const;
      // We need this because the above operator must return 0 for nonexisting
      // labels.
      bool Exists( const std::string& ) const;
      // A reverse lookup operator.
      const std::string& operator[]( size_t ) const;
      std::string& operator[]( size_t );

      bool IsTrivial() const;
      static const std::string& TrivialLabel( size_t );

      // Stream I/O.
      std::ostream& WriteToStream( std::ostream& ) const;
      std::istream& ReadFromStream( std::istream& );

     private:
      void resize( size_t );
      size_t size() const { return reverseIndex.size(); }
      void sync() const;
      void clear()
      {
        reverseIndex.clear();
        forwardIndex.clear();
        resize( 1 );
      }

     private:
      // This is the maintained index.
      indexer_reverse reverseIndex;
      // This is a cache for the most probable lookup direction.
      mutable bool needSync;
      mutable indexer_base forwardIndex;
   };

 private:
          encodedString section,
                        name,
                        type,
                        defaultvalue,
                        lowrange,
                        highrange;
          std::string   comment;
          labelIndexer  dim1_index,
                        dim2_index;
          typedef std::vector<encodedString> values_type;
          values_type values;

 public:
        PARAM();
        PARAM( const char* name,
               const char* section,
               const char* type,
               const char* value,
               const char* defaultvalue,
               const char* lowrange,
               const char* highrange,
               const char* comment );
        PARAM( const char* paramstring );
        ~PARAM() {}

        PARAM& operator=( const PARAM& );

        void    SetSection( const std::string& s )
                { section = s; }
        void    SetType( const std::string& s )
                { type = s; tolower( type ); }
 private:
        // Changing the name without changing its index in the list would be
        // a bad idea, so this function is private.
        void    SetName( const std::string& s )
                { name = s; }
 public:
        void    SetNumValues( size_t n );
        void    SetValue( const std::string& s )
                { SetValue( s, 0 ); }
        void    SetValue( const std::string&, size_t );
        void    SetValue( const std::string& s, size_t n, size_t m )
                { SetValue( s, n * GetNumValuesDimension2() + m ); }
        void    SetValue( const std::string& s,
                          const std::string& label )
                { return SetValue( s, dim1_index[ label ] ); }
        void    SetValue( const std::string& s,
                          const std::string& label_dim1, const std::string& label_dim2 )
                { return SetValue( s, dim1_index[ label_dim1 ], dim2_index[ label_dim2 ] ); }
        void    SetValue( const std::string& s,
                          size_t index_dim1, const std::string& label_dim2  )
                { return SetValue( s, index_dim1, dim2_index[ label_dim2 ] ); }
        void    SetValue( const std::string& s,
                          const std::string& label_dim1, size_t index_dim2 )
                { return SetValue( s, dim1_index[ label_dim1 ], index_dim2 ); }

  const char*   GetSection() const
                { return section.c_str(); }
  const char*   GetType() const
                { return type.c_str(); }
  const char*   GetName() const
                { return name.c_str(); }
  const char*   GetDefaultValue() const
                { return defaultvalue.c_str(); }
  const char*   GetLowRange() const
                { return lowrange.c_str(); }
  const char*   GetHighRange() const
                { return highrange.c_str(); }
  const char*   GetComment() const
                { return comment.c_str(); }
        size_t  GetNumValues() const
                { return values.size(); }
        size_t  GetNumValuesDimension1() const
                { return GetNumValues() / GetNumValuesDimension2(); }
        size_t  GetNumRows() const
                { return GetNumValuesDimension1(); }
        size_t  GetNumValuesDimension2() const
                { return dim2_index.size(); }
        size_t  GetNumColumns() const
                { return GetNumValuesDimension2(); }
        void    SetDimensions( size_t, size_t );
  const char*   GetValue() const
                { return GetValue( 0 ); }
  const char*   GetValue( size_t ) const;
  const char*   GetValue( size_t n, size_t m ) const
                { return GetValue( n * GetNumValuesDimension2() + m ); }
  const char*   GetValue( const std::string& label ) const
                { return GetValue( dim1_index[ label ] ); }
  const char*   GetValue( const std::string& label_dim1, const std::string& label_dim2 ) const
                { return GetValue( dim1_index[ label_dim1 ], dim2_index[ label_dim2 ] ); }
  const char*   GetValue( size_t index_dim1, const std::string& label_dim2  ) const
                { return GetValue( index_dim1, dim2_index[ label_dim2 ] ); }
  const char*   GetValue( const std::string& label_dim1, size_t index_dim2 ) const
                { return GetValue( dim1_index[ label_dim1 ], index_dim2 ); }
  labelIndexer& LabelsDimension1()
                { changed = true; return dim1_index; }
  labelIndexer& RowLabels()
                { changed = true; return LabelsDimension1(); }
  const labelIndexer& LabelsDimension1() const
                { return dim1_index; }
  const labelIndexer& RowLabels() const
                { return LabelsDimension1(); }
  labelIndexer& LabelsDimension2()
                { changed = true; return dim2_index; }
  labelIndexer& ColumnLabels()
                { changed = true; return LabelsDimension2(); }
  const labelIndexer& LabelsDimension2() const
                { return dim2_index; }
  const labelIndexer& ColumnLabels() const
                { return LabelsDimension2(); }
  labelIndexer& Labels()
                { changed = true; return dim1_index; }
  const labelIndexer& Labels() const
                { return dim1_index; }
        bool    Valid() const
                { return valid; }
        bool    Changed() const
                { return changed; }
        void    Unchanged()
                { changed = false; }

        std::ostream& WriteToStream( std::ostream& ) const;
        std::istream& ReadFromStream( std::istream& );
        std::ostream& WriteBinary( std::ostream& ) const;
        std::istream& ReadBinary( std::istream& );

 private:
        bool    valid;
        bool    changed;

 public: // These will become private.
        bool    archive;
        bool    tag;  // important for parameter save/load filters

 // A class that allows for convenient automatic type conversions when
 // accessing parameter values.
 public:
  class type_adapter
  {
   private:
    type_adapter& operator=( const type_adapter& );
   public:
    type_adapter()
    : p( NULL ), i( 0 ), j( 0 ) {}
    type_adapter( PARAM* param, size_t row, size_t column )
    : p( param ), i( row ), j( column ) {}
    // Assignment operators for write access.
    type_adapter& operator=( const char* s )
    { if( p ) p->SetValue( s, i, j ); return *this; }
    type_adapter& operator=( double d )
    { std::ostringstream os; os << d; if( p ) p->SetValue( os.str().c_str(), i, j ); return *this; }
    // Conversion operators for read access.
    operator const char*() const
    { return p ? p->GetValue( i, j ) : ""; }
    operator double() const
    { return p ? atof( p->GetValue( i, j ) ) : 0.0; }

    // We need to override operators to avoid ambiguities
    // when the compiler resolves expressions.
    double operator-( double d ) const
    { return double( *this ) - d; }
    double operator+( double d ) const
    { return double( *this ) + d; }
    double operator*( double d ) const
    { return double( *this ) * d; }
    double operator/( double d ) const
    { return double( *this ) / d; }

    bool operator==( double d ) const
    { return double( *this ) == d; }
    bool operator!=( double d ) const
    { return double( *this ) != d; }
    bool operator<( double d ) const
    { return double( *this ) < d; }
    bool operator>( double d ) const
    { return double( *this ) > d; }
    bool operator<=( double d ) const
    { return double( *this ) <= d; }
    bool operator>=( double d ) const
    { return double( *this ) >= d; }
    // Dereferencing operator for access to PARAM members.
    PARAM* operator->() const
    { return p ? p : &( null_p = PARAM() ); }

   private:
    PARAM* p;
    size_t i, j;
    static PARAM null_p;
  };

 // Case insensitive string handling components.
 private:
  static const std::ctype<char>& ct;

 public:
  static void tolower( std::string& s )
  { ct.tolower( s.begin(), s.end() ); }
  static void toupper( std::string& s )
  { ct.toupper( s.begin(), s.end() ); }

  static bool ciless( char a, char b )
  { return ct.toupper( a ) < ct.toupper( b ); }
  static bool ciequal( char a, char b )
  { return ct.toupper( a ) == ct.toupper( b ); }

  static bool strciless( const std::string& a, const std::string& b )
  { return namecmp()( a, b ); }
  static bool strciequal( const std::string& a, const std::string& b )
  { return !strciless( a, b ) && !strciless( b, a ); }

  class namecmp
  {
   public:
    bool operator()( const std::string& a, const std::string& b ) const
    { return std::lexicographical_compare(
                        a.begin(), a.end(), b.begin(), b.end(), ciless ); }
  };
};

typedef std::map<std::string, PARAM, PARAM::namecmp> param_container;

class PARAMLIST : public param_container
{
 public:
        PARAM*  GetParamPtr( const char* name );
  const PARAM*  GetParamPtr( const char* name ) const;

        size_t  GetNumParameters() const
                { return size(); }
        void    ClearParamList()
                { clear(); }
        void    DeleteParam( const char* name )
                { erase( name ); }

        void    AddParameter2List( const char* paramstring,
                                   size_t paramlen = 0 );
        bool    SaveParameterList( const char* filename,
                                   bool usetags = false ) const;
        bool    LoadParameterList( const char* filename,
                                   bool usetags = false,
                                   bool importnonexisting = true );

  // These contain all formatted I/O functionality.
        std::ostream& WriteToStream( std::ostream& ) const;
        std::istream& ReadFromStream( std::istream& );

  // These define binary I/O.
        std::ostream& WriteBinary( std::ostream& ) const;
        std::istream& ReadBinary( std::istream& );

  // These are for compatibility.
        PARAM*  GetParamPtr( size_t );
  const PARAM*  GetParamPtr( size_t ) const;
};

inline std::ostream& operator<<( std::ostream& s, const PARAM::labelIndexer& i )
{
  return i.WriteToStream( s );
}

inline std::istream& operator>>( std::istream& s, PARAM::labelIndexer& i )
{
  return i.ReadFromStream( s );
}

inline std::ostream& operator<<( std::ostream& s, const PARAM::encodedString& e )
{
  return e.WriteToStream( s );
}

inline std::istream& operator>>( std::istream& s, PARAM::encodedString& e )
{
  return e.ReadFromStream( s );
}

inline std::ostream& operator<<( std::ostream& s, const PARAM& p )
{
  return p.WriteToStream( s );
}

inline std::istream& operator>>( std::istream& s, PARAM& p )
{
  return p.ReadFromStream( s );
}

inline std::ostream& operator<<( std::ostream& s, const PARAMLIST& p )
{
  return p.WriteToStream( s );
}

inline std::istream& operator>>( std::istream& s, PARAMLIST& p )
{
  return p.ReadFromStream( s );
}

#endif // UParameterH

