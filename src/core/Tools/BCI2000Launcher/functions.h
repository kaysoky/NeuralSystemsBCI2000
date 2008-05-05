//---------------------------------------------------------------------------

#include <vector>
#include <string>

#include <dir.h>
#include <iostream>
#include <fstream>
using namespace std;
#ifndef functionsH
#define functionsH
bool ismember(string str, vector<string> strs);
void removeAt(vector<string> &str, int pos);
bool getNextLine(ifstream &in, vector<string> &tokens, string delimiters);
void stringSplit(const string& str, vector<string>& tokens, string delimiters);
string lowerCase(string str);
char *current_directory(char *path);
//---------------------------------------------------------------------------
#endif
