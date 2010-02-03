/////////////////////////////////////////////////////////////////////////////
//
// File: SpellerTree.h
//
// Date: Dec 13, 2001
//
// Author: Juergen Mellinger
//
// Description: A class that reads and manages a speller tree
//              with arbitrary depth and an arbitrary number of
//              choices at each node.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef SPELLERTREEH
#define SPELLERTREEH

#include <vector>
#include <string>
#include <map>

#include "PresErrors.h"

class TSpellerTreeNode
{
  public:
    std::vector< std::string >  labels;
    std::vector< std::string >  texts;
    std::vector< unsigned int > choices;
    std::string                 audioLabel;
};

class TSpellerTree : private std::vector< TSpellerTreeNode >
{
  public:
                    TSpellerTree();
                    ~TSpellerTree();

    void            Reset();
    bool            Fail() const;
    bool            BackspaceFail() const;

    // From the current node, choose the option associated with the
    // target code given. Returns a reference to a string that is
    // associated with that choice.
    const std::string&  Choose( unsigned int inTargetCode );

    // Get the current node's user label for the target code given.
    const std::string&  GetLabel( unsigned int inTargetCode );

    // Get the current node's audio label file name.
    const std::string&  GetAudioLabel() const;

    // Return the target code for the choice that has to be chosen for the shortest
    // way to enter the text given in the argument.
    unsigned int        NextChoiceInShortestPath(   const std::string&  inCurrentText,
                                                    const std::string&  inTargetText );
    unsigned int        NextChoiceInShortestPath(   const iterator      inStartNode,
                                                    const iterator      inTargetNode );

    unsigned short      DictionaryProposalCode() const;

    TPresError          ReadFromFile( const char* inTreeFileName );

    // Adapt a given text to the capabilities of the current speller tree.
    TPresError          NormalizeText( std::string& ioText );

  private:
    enum TFailstate
    {
        good = 0,
        generalFail = 1 << 0,
        backspaceFail = 1 << 1
    };
    
    typedef unsigned long           TWeightType;
    static const unsigned long      infiniteWeight = 32000;
    TWeightType  Weight( const iterator, size_t ) const;

    struct TPathInfo
    {
        unsigned int    predecessorNode;
        TWeightType     pathLength;
    };
    
    std::map< unsigned int, unsigned int >  targetMap;
    std::vector< unsigned int >             targetCodes;
    std::map< std::string, unsigned int >   nodesByText;
    iterator                                currentNode;
    unsigned short                          dictionaryProposalCode;
    unsigned int                            textMaxLength;
    TWeightType                             backspaceWeight;
    TFailstate                              failstate;
};

inline
unsigned short
TSpellerTree::DictionaryProposalCode() const
{
    return dictionaryProposalCode;
}

inline
bool
TSpellerTree::Fail() const
{
    return failstate != good;
}

inline
bool
TSpellerTree::BackspaceFail() const
{
    return ( failstate & backspaceFail ) != 0;
}

inline
const std::string&
TSpellerTree::GetAudioLabel() const
{
    return currentNode != end() ? currentNode->audioLabel : std::string( "" );
}

#endif // SPELLERTREEH
