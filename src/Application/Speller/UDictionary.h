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
        int     LoadDictionary(char *dictionaryfile, bool eraseold);
        void    DeleteWords();
        void    AddWord(char *word);
        char    *GetWord(int idx);
        int     GetNumWords();
        int     GetNumMatching(char *prefix, bool casesensitive);
        char    *GetMatchingWord(char *prefix, bool casesensitive, int idx);
};
#endif
