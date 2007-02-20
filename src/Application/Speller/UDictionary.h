/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UDictionaryH
#define UDictionaryH
//---------------------------------------------------------------------------

#define MAX_WORDS       2000
#define MAX_WORDLENGTH  20

class DICTIONARY
{
private: 	// User declarations
        char    words[MAX_WORDS][MAX_WORDLENGTH];
        int     numwords;
public:		// User declarations
        DICTIONARY::DICTIONARY();
        DICTIONARY::~DICTIONARY();
        int     LoadDictionary(const char *dictionaryfile, bool eraseold);
        void    DeleteWords();
        void    AddWord(const char *word);
        const char    *GetWord(int idx) const;
        int     GetNumWords();
        int     GetNumMatching(const char *prefix, bool casesensitive);
        const char    *GetMatchingWord(const char *prefix, bool casesensitive, int idx) const;
};
#endif


