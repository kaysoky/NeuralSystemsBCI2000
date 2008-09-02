//---------------------------------------------------------------------------


#pragma hdrstop

#include "functions.h"

//------------------------------------------------------------------
char *current_directory(char *path)
{
  strcpy(path, "X:\\");      /* fill string with form of response: X:\ */
  path[0] = 'A' + getdisk();    /* replace X with current drive letter */
  getcurdir(0, path+3);  /* fill rest of string with current directory */
  return(path);
}


bool getNextLine(ifstream &in, vector<string> &tokens, string delimiters)
{
    bool found = false;
    size_t pos;
    string line;

    if (in.eof())
        return false;
        
    while (!found && !in.eof())
    {
        tokens.clear();
        line.erase();

        
        if (in.eof())
			return false;

        //in.getline(line, 500, '\n');
       // getline(in, line, '\n');
       getline(in, line);

       //remove comments
       pos = line.find("//");
       if (pos != string::npos)
       {
            line.erase(pos);
       }

       //remove leading spaces or tabs
       pos = line.find_first_not_of(" \t");
		if (pos != string::npos)
		{
			//cout << "erasing leading spaces..."<<endl;
			line.erase(0, pos);
		}

		// remove trailing spaces
		pos = line.find_last_not_of(" \t");
		if (pos != string::npos)
		{
			
			//cout <<"erasing trailing spaces..."<<endl;
			//cout << "Pos = "<<pos<<endl;
			line.erase(pos+1);
		}

		if (line.length() < 1)
		{
			//cout <<"going to next line..."<<endl;
			// get the next line
			continue;
		}
        if (line.length() == 1)
		{
			//cout <<"LENGTH = 1. Char is "<<(int)line[0]<<endl;
		}

		if (line.length() < 1)
		{
			//cout <<"Nothing left, going to next line..."<<endl;
			// get next line
			continue;
		}
        stringSplit(line, tokens, delimiters);
		//getchar();
		found = true;
    }
    return found;
}

void stringSplit(const string& str, vector<string>& tokens, string delimiters)
{

	tokens.clear();
	// Find the first position in the string that is not a delimiter
	size_t start = str.find_first_not_of(delimiters);
	size_t endStr = 0;

	//cout <<"String: ["<<str<<"]"<<endl;
	//cout <<"Start: "<<start<<endl;
	
	while (start != string::npos)
	{
		endStr = str.find_first_of(delimiters, start+1);
		if (endStr == string::npos)
			endStr = str.length();

		tokens.insert(tokens.end(), str.substr(start, endStr - start));

		start = str.find_first_not_of(delimiters, endStr+1);
		//cout << str.substr(start, endStr - start) <<endl;
	}
	

	return;
}

string lowerCase(string str)
{
	string returnStr = str;
	for (unsigned int i = 0; i < returnStr.length(); i++)
	{
		returnStr[i] = tolower(returnStr[i]);
	}
	return returnStr;
}

//---------------------------------------------------------------------------
void removeAt(vector<string> &str, int pos)
{
    if (pos >= int( str.size() ) || pos < 0)
        return;

    vector<string> newStr = str;
    str.clear();
    for (int i=0; i < int( newStr.size() ); i++)
    {
        if (i != pos)
            str.push_back(newStr[i]);
    }
}
//---------------------------------------------------------------------------

#pragma package(smart_init)
