////////////////////////////////////////////////////////////////////////////////
//
// File: VisList.h
//
// Description: A class template that handles lists of visualization parameters.
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date:   May 6, 2005
//
////////////////////////////////////////////////////////////////////////////////
#ifndef VisListH
#define VisListH

#include <iostream>
#include <vector>

template<typename T> class VisList: public std::vector<T>
{
  public:

    VisList() {}

    void WriteToStream( std::ostream& ) const;
    void ReadFromStream( std::istream& );
  private:
    static const char beginOfList = '{';
    static const char endOfList = '}';
};

template<typename T>
void
VisList<T>::ReadFromStream( std::istream& is )
{
  clear();
  is >> ws;
  if( is.peek() == beginOfList )
    is.get();
  else
    is.setstate( ios::failbit );
  is >> ws;

  T t;
  while( is.peek() != endOfList && is >> t >> ws )
    push_back( t );

  if( is.peek() == endOfList )
    is.get();
  else
    is.setstate( ios::failbit );
}

template<typename T>
void
VisList<T>::WriteToStream( std::ostream& os ) const
{
  os << beginOfList;
  for( const_iterator i = begin(); i != end(); ++i )
    os << *i << " ";
  os << endOfList;
}

template<typename T>
inline std::ostream&
operator<<( std::ostream& s, const VisList<T>& v )
{
  v.WriteToStream( s );
  return s;
}

template<typename T>
inline std::istream&
operator>>( std::istream& s, VisList<T>& v )
{
  v.ReadFromStream( s );
  return s;
}

#endif // VisListH

