//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <string.h>

#include "UDictionary.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


// **************************************************************************
// Function:   DICTIONARY
// Purpose:    This is the constructor of the DICTIONARY class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
DICTIONARY::DICTIONARY()
{
 numwords=0;
}


// **************************************************************************
// Function:   DICTIONARY
// Purpose:    This is the constructor of the DICTIONARY class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
DICTIONARY::~DICTIONARY()
{
 DeleteWords();
}


// **************************************************************************
// Function:   LoadDictionary
// Purpose:    This function loads a dictionary file
// Parameters: dictionaryfile - filename of the dictionary file
//             eraseold - true  ... deletes all existing words in the dictionary
//                        false ... does not delete existing words in the dictionary
// Returns:    0 ... error
//             1 ... OK
// **************************************************************************
int DICTIONARY::LoadDictionary(char *dictionaryfile, bool eraseold)
{
char    line[MAX_WORDLENGTH];
FILE    *fp;
int     i;

 if (eraseold) DeleteWords();

 fp=fopen(dictionaryfile, "rb");
 if (!fp) return(0);

 while (!feof(fp))
  {
  fgets(line, MAX_WORDLENGTH, fp);
  line[MAX_WORDLENGTH-1]=0;
  for (i=0; i<(int)strlen(line); i++)
   if ((line[i] == ' ') || (line[i] == '\r') || (line[i] == '\n'))
      line[i]=0;
  AddWord(line);
  }
 fclose(fp);

 return(1);
}


int DICTIONARY::GetNumWords()
{
 return(numwords);
}


void DICTIONARY::DeleteWords()
{
 numwords=0;
}


void DICTIONARY::AddWord(char *word)
{
 if (numwords >= MAX_WORDS-2) return;

 strcpy(&words[numwords][0], word);
 numwords++;
}


char *DICTIONARY::GetWord(int idx)
{
 if (idx >= numwords) return(NULL);

 return(&words[idx][0]);
}


int DICTIONARY::GetNumMatching(char *prefix, bool casesensitive)
{
int     i, count;

 if (numwords == 0) return(0);

 count=0;
 for (i=0; i<numwords; i++)
  {
  if (casesensitive)
     if (strncmp(GetWord(i), prefix, strlen(prefix)) == 0)
        count++;
  if (!casesensitive)
     if (strnicmp(GetWord(i), prefix, strlen(prefix)) == 0)
        count++;
  }

 return(count);
}


char *DICTIONARY::GetMatchingWord(char *prefix, bool casesensitive, int idx)
{
int     i, count;
char    *ptr;

 count=-1;
 ptr=NULL;
 for (i=0; i<numwords; i++)
  {
  if (casesensitive)
     if (strncmp(GetWord(i), prefix, strlen(prefix)) == 0)
        count++;
  if (!casesensitive)
     if (strnicmp(GetWord(i), prefix, strlen(prefix)) == 0)
        count++;
  if (count == idx)
     {
     ptr=GetWord(i);
     break;
     }
  }

 return(ptr);
}

