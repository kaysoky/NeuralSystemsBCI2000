////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
// Description: Does a simple diff between two data streams, spits out problems
// Alternate name: GriffDiff (awesome, yes?)
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

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;

const double cSignificantError = 1.5;

int main( int argc, char *argv[] )
{
  // Welcome message
  cout << "bci_datadiff" << endl << "BCI2000 Project 2009-2011" << endl << endl;

  // User has no idea how to use the program.
  if ( argc < 3 )
  {
    cerr << "Usage: bci_datadiff testFile refFile [outputFile]" << endl;
    return( 1 );
  }

  // Try to open the test file
  ifstream in_test( argv[1] );
  if( !in_test.is_open() )
  {
    cerr << "Could not open test file, " << argv[1] << ", for reading." << endl;
    return( 1 );
  }

  // Try to open the reference
  ifstream in_ref( argv[2] );
  if( !in_ref.is_open() )
  {
    cerr << "Could not open reference file, " << argv[2] << ", for reading." << endl;
    return( 1 );
  }

  // If the user specified an output file, we should try to open that too.
  ofstream out_file;
  bool output = false;
  if( argc > 3 )
  {
    out_file.open( argv[3] );
    if( !out_file.is_open() )
    {
      cerr << "Could not open output file, " << argv[3] << ", for writing." << endl;
      return( 1 );
    }
    output = true;
  }

  // Some of the data at the top of the file may be different
  // but not important.  We'll skip past that.
  bool skipforward = true;
  while( skipforward )
  {
    // Get the input from the test and the ref
    string testLine;
    getline( in_test, testLine );
    string refLine;
    getline( in_ref, refLine );

    // Find out if we've skipped ahead far enough
    string state;
    stringstream ss( testLine );
    ss >> state;
    if( state == "VisSignalProperties" )
      skipforward = false;
  }

  // We havn't found differences yet
  int numDifferences = 0;
  int numSignificantDifferences = 0;

  // Read the files
  cout << "Comparing test: " << argv[1] << " vs ref: " << argv[2] << " ..." << endl
       << "------------------------------------------" << endl;
  if( output )
    out_file << "Comparing test: " << argv[1] << " vs ref: " << argv[2] << " ..." << endl
             << "------------------------------------------" << endl;
  while( !in_test.eof() || !in_ref.eof() )
  {
    // Get the input from the test and the ref
    string testLine;
    getline( in_test, testLine );
    string refLine;
    getline( in_ref, refLine );

    // Test to see if it's one of the states we should ignore
    string state;
    stringstream ss( testLine );
    ss >> state;
    if( state == "SourceTime:" )
      continue;
    if( state == "StimulusTime:" )
      continue;
    if( state == "TestLoggerCounter:" )
      continue;
    // For signal data, compute a relative error
    if( state == "VisSignal" )
    {
      getline( in_test, testLine );
      getline( in_ref, refLine );
      getline( in_test, testLine );
      getline( in_ref, refLine );
      getline( in_test, testLine, '}' );
      getline( in_ref, refLine, '}' );
      stringstream refStream( refLine ),
                   testStream( testLine );
      double refVal = 0.0,
             testVal = 0.0;
      while( refStream >> refVal && testStream >> testVal )
      {
        double err = ::fabs( testVal - refVal );
        if( err > 0 )
        {
          ostringstream oss;
          oss << "Difference: signal value \"" << testVal << "\" should be \"" << refVal << "\"." << endl;
          cout << oss.str();
          if( output )
            out_file << oss.str();
          ++numDifferences;
        }
        if( err >= cSignificantError )
          ++numSignificantDifferences;
      }
    }
    // Compare the two lines
    else if( testLine != refLine )
    {
      cout << "Difference: \"" << testLine << "\" should be \"" << refLine << "\"." << endl;
      if( output )
        out_file << "Difference: \"" << testLine << "\" should be \"" << refLine << "\"." << endl;
      numDifferences++;
      numSignificantDifferences++;
    }
  }

  cout << "------------------------------------------" << endl;
  if( output )
    out_file << "------------------------------------------" << endl;

  // Output the number of differences to the file
  if( output )
    out_file << "Number of Differences: " << numDifferences << endl
             << "Number of Significant Differences: " << numSignificantDifferences << endl;

  // Tell the user what's up.
  if( numDifferences != 0 )
  {
    cout << "Differences have been found." << endl
         << "There is/are " << numDifferences << "/" << numSignificantDifferences
         << " total/significant difference(s) between the ref and test file. " << endl;
  }
  else
    cout << "No differences found!" << endl;

  // Cleanup
  if( output )
    out_file.close();
  in_test.close();
  in_ref.close();
  return numSignificantDifferences;
}





