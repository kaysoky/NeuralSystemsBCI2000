/******************************************************************************
 * Program:   BCI2000                                                         *
 * Module:    UParameter.cpp                                                  *
 * Comment:   This unit provides support for system-wide parameters           *
 *            and parameter lists                                             *
 * Version:   0.17                                                            *
 * Author:    Gerwin Schalk                                                   *
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
 * V0.17 - 01/31/2003 - fixed bug in SaveParameterList()                      *
 ******************************************************************************/

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <sysutils.hpp>

#include "UParameter.h"
#include "UCoremessage.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)


// **************************************************************************
// Function:   PARAMLIST
// Purpose:    the constructor for the PARAMLIST object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
PARAMLIST::PARAMLIST()
{
 param_list=new TList;
}


// **************************************************************************
// Function:   ~PARAMLIST
// Purpose:    The destructor for the PARAMLIST object. It deletes both
//             all PARAM objects in the list and also the list object itself
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
PARAMLIST::~PARAMLIST()
{
int     i;
PARAM   *cur_param;

  // Clean up – must free memory for the items as well as the list
  for (i=0; i<param_list->Count; i++)
   {
   cur_param=(PARAM *)param_list->Items[i];
   delete cur_param;
   }

 delete param_list;
}


// **************************************************************************
// Function:   SortCompare
// Purpose:    Is the comparison function for the TList sort command
// Parameters: Item1 + Item2 ... pointers to items in the list
// Returns:    <0 ... Item1 < Item2
//             =0 ... Item1 = Item2
//             >0 ... Item1 > Item2
// **************************************************************************
int __fastcall SortCompare(void * Item1, void * Item2)
{
const char *name1, *name2;

 name1=((PARAM *)Item1)->GetName();
 name2=((PARAM *)Item2)->GetName();

 if (!name1 || !name2)
    return(0);

 return(stricmp(name1, name2));
}


// **************************************************************************
// Function:   Sort
// Purpose:    This function sorts the parameters in the list alphabetically
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void PARAMLIST::Sort()
{
 param_list->Sort(SortCompare);
}

#if 0
// **************************************************************************
// Function:   PublishParameters
// Purpose:    this method publishes this module's parameters
// Parameters: Socket - socket descriptor for the connection to the operator
// Returns:    always 0
// **************************************************************************
int PARAMLIST::PublishParameters(TCustomWinSocket *Socket)
{
COREMESSAGE     *coremessage;
FILE            *fp;
char            line[512];
int i;
PARAM           *cur_param;

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_PARAMETER);

 for (i=0; i<GetNumParameters(); i++)
  {
  cur_param=GetParamPtr(i);                                     // get the i'th parameter
  strcpy(line, cur_param->GetParamLine());                      // copy its ASCII representation to variable line
  strncpy(coremessage->GetBufPtr(), line, strlen(line));        // copy line into the coremessage
  coremessage->SetLength((unsigned short)strlen(line));         // set the length of the coremessage
  coremessage->SendCoreMessage((TCustomWinSocket *)Socket);     // and send it out
  }

 delete coremessage;
 return(0);
}
#endif

// **************************************************************************
// Function:   GetNumParameters
// Purpose:    Returns the number of parameters in the list
// Parameters: N/A
// Returns:    the number of parameters
// **************************************************************************
int PARAMLIST::GetNumParameters() const
{
return(param_list->Count);
}


// **************************************************************************
// Function:   GetParamPtr
// Purpose:    given a parameter name, returns the pointer to a PARAM object
// Parameters: name - name of the parameter
// Returns:    pointer to a PARAM object or
//             NULL, if no parameter with this name exists in the list
// **************************************************************************
const PARAM   *PARAMLIST::GetParamPtr(const char *name) const
{
const char* paramname;
int         i;

 for (i=0; i<GetNumParameters(); i++)
  {
  paramname=GetParamPtr(i)->GetName();
  if (strcmpi(name, paramname) == 0)
     return(GetParamPtr(i));
  }

 return(NULL);
}

PARAM   *PARAMLIST::GetParamPtr(const char *name)
{
  // Sorry for this hack.
  const PARAMLIST* p = this;
  return ( PARAM* )p->GetParamPtr( name );
}

// **************************************************************************
// Function:   GetParamPtr
// Purpose:    given an index (0..GetListCount()-1), returns the pointer to a PARAM object
// Parameters: idx - index of the parameter
// Returns:    pointer to a PARAM object or
//             NULL, if the specified index is out of range
// **************************************************************************
const PARAM *PARAMLIST::GetParamPtr(int idx) const
{
 if ((idx < param_list->Count) && (idx >= 0))
    return((PARAM *)param_list->Items[idx]);
 else
    return(NULL);
}

PARAM *PARAMLIST::GetParamPtr(int idx)
{
  // Sorry for this hack, too.
  const PARAMLIST* p = this;
  return ( PARAM* )p->GetParamPtr( idx );
}

// **************************************************************************
// Function:   ClearParamList
// Purpose:    deletes all parameters in the parameter list
//             as a result, the list still exists, but does not contain any parameter
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void PARAMLIST::ClearParamList()
{
PARAM   *cur_param;

 while (param_list->Count > 0)
  {
  cur_param=(PARAM *)param_list->Items[0];
  param_list->Delete(0);
  delete cur_param;
  }

 param_list->Clear();
}


// **************************************************************************
// Function:   SaveParameterList
// Purpose:    Saves the current list of paramters in a parameter file
// Parameters: char *filename - filename to save the list to
// Returns:    true - successful
//             false - error (disc full, etc.)
// **************************************************************************
bool PARAMLIST::SaveParameterList(const char *filename) const
{
 return(SaveParameterList(filename, false));
}


// **************************************************************************
// Function:   SaveParameterList
// Purpose:    Saves the current list of paramters in a parameter file
// Parameters: char *filename - filename to save the list to
//             usetags - if usetags is true, then the "tag" value in each parameter determines whether the parameter should be saved
//                       if the tag value in the parameter is true, then the parameter will NOT be saved
//                       if usetags is false, then all parameters are saved
// Returns:    true - successful
//             false - error (disc full, etc.)
// **************************************************************************
bool PARAMLIST::SaveParameterList(const char *filename, bool usetags) const
{
const char  *paramline;
int         i;
FILE        *fp;

 fp=fopen(filename, "wb");
 if (!fp) return(false);

 for (i=0; i<GetNumParameters(); i++)
  {
  if ((usetags && !GetParamPtr(i)->tag) || (!usetags))
     {
     paramline=GetParamPtr(i)->GetParamLine();
     if (fprintf(fp, "%s\r\n", paramline) == EOF)
        {
        fclose(fp);
        return(false);
        }
     }
  }

 fclose(fp);
 return(true);
}


// **************************************************************************
// Function:   LoadParameterList
// Purpose:    Loads the current list of parameters from a parameter file
//             It does NOT load system critical dynamic parameters (e.g., ports, IP addresses)
// Parameters: char *filename - filename of the parameterlist
// Returns:    true - successful
//             false - error
// **************************************************************************
bool PARAMLIST::LoadParameterList(const char *filename)
{
 return(LoadParameterList(filename, false, true));
}


// **************************************************************************
// Function:   LoadParameterList
// Purpose:    Loads the current list of parameters from a parameter file
//             It does NOT load system critical dynamic parameters (e.g., ports, IP addresses)
// Parameters: char *filename - filename of the parameterlist
//             usetags - if usetags is true, then the "tag" value in each parameter determines whether the parameter should be loaded
//                       if the tag value in the parameter is "true", then the parameter will NOT be loaded
//                       if usetags is false, then all parameters are loaded
//             nonexisting - if true, load parameters, even if they currently do not exist in the list
// Returns:    true - successful
//             false - error
// **************************************************************************
bool PARAMLIST::LoadParameterList(const char *filename, bool usetags, bool importnonexisting)
{
char    paramline[10000], paramname[256];
int     idx;
bool    dont;
FILE    *fp;
PARAM   dummyparam;

 fp=fopen(filename, "rb");
 if (!fp) return(false);

 while (!feof(fp))
  {
  fgets(paramline, 10000, fp);
  dont=false;
  idx=0;
  idx=dummyparam.get_argument(idx, paramname, paramline, 10000);   // section
  idx=dummyparam.get_argument(idx, paramname, paramline, 10000);   // data type
  idx=dummyparam.get_argument(idx, paramname, paramline, 10000);   // parameter name
  // do not import, if we use tags (and the parameter exists) and the tag says don't import
  if ((GetParamPtr(paramname) != NULL) && (usetags))
     if (GetParamPtr(paramname)->tag)
        dont=true;
  // do not import, if we choose to import only existing parameters and if the parameter does not already exist
  if ((GetParamPtr(paramname) == NULL) && (!importnonexisting)) dont=true;
  // do not import, if one of these system parameters
  if (stricmp(paramline, "ApplicationPort") == 0) dont=true;
  if (stricmp(paramline, "ApplicationIP") == 0) dont=true;
  if (stricmp(paramline, "SignalProcessingPort") == 0) dont=true;
  if (stricmp(paramline, "SignalProcessingIP") == 0) dont=true;
  if (stricmp(paramline, "EEGsourcePort") == 0) dont=true;
  if (stricmp(paramline, "EEGsourceIP") == 0) dont=true;
  if (stricmp(paramline, "StateVectorLength") == 0) dont=true;
  if (!dont)
     AddParameter2List(paramline, strlen(paramline));
  }

 fclose(fp);

 // let's re-sort all the parameters
 Sort();
 return(true);
}


// **************************************************************************
// Function:   AddParameter2List
// Purpose:    adds a new parameter to the list of parameters
//             if a parameter with the same name already exists,
//             it updates the currently stored parameter with the provided values
// Parameters: paramstring - ASCII string, as defined in project description,
//                           defining this parameter
// Returns:    N/A
// **************************************************************************
void PARAMLIST::AddParameter2List(const char *paramstring, int paramlen)
{
PARAM   *new_param;
int lengthToConsider = paramlen;
 if( lengthToConsider == 0 )
  lengthToConsider = strlen( paramstring );

 new_param=new PARAM();
 new_param->ParseParameter(paramstring, lengthToConsider);
 if (new_param->valid)
    CloneParameter2List(new_param);                        // it clones the newly created parameter to the list

 delete new_param;
}


// **************************************************************************
// Function:   MoveParameter2List
// Purpose:    adds a new parameter to the list of parameters
//             this function assumes that the specified parameter object
//             will not be freed anywhere else (but in the destructor of the
//             paramlist object). The difference to CloneParameter2List() is
//             that it does not actually copy the whole param object
//             This function will not add a parameter to the list, if one
//             with the same name already exists
// Parameters: param - pointer to an existing PARAM object
// Returns:    N/A
// **************************************************************************
void PARAMLIST::MoveParameter2List(PARAM *newparam)
{
 // if we can find a parameter with the same name, do not add this parameter to the list
 if (GetParamPtr(newparam->GetName()))
    return;

 param_list->Add(newparam);
}


// **************************************************************************
// Function:   DeleteParam
// Purpose:    deletes a parameter of a given name in the list of parameters
//             it also frees the memory for this particular parameter
//             it does not do anything, if the parameter does not exist
// Parameters: name - name of the parameter
// Returns:    N/A
// **************************************************************************
void PARAMLIST::DeleteParam(const char *name)
{
int     i;
PARAM   *cur_param;

 // Clean up – must free memory for the item as well as the list
 for (i=0; i<param_list->Count; i++)
  {
  cur_param=(PARAM *)param_list->Items[i];
  if (strcmpi(name, cur_param->GetName()) == 0)
     {
     param_list->Delete(i);
     delete cur_param;
     }
  }
}


// **************************************************************************
// Function:   CloneParameter2List
// Purpose:    adds a new parameter to the list of parameters
//             The difference to MoveParameter2List() is that it actually
//             physically copies the param object (the specified param
//             object can then be freed elsewhere)
//             This function will UPDATE the values for a parameter in the list,
//             if one with the same name already exists
// Parameters: param - pointer to an existing PARAM object
// Returns:    N/A
// **************************************************************************
void PARAMLIST::CloneParameter2List(const PARAM *param)
{
PARAM           *newparam;
LSTVALUE        *newlstvalue;
int             i;
TList           *bufferlst;

 // if we can find a parameter with the same name, delete the old parameter and
 // thereafter add the new one
 if (GetParamPtr(param->GetName()))
    DeleteParam(param->GetName());

 // create a new parameter and copy the content of the other one
 newparam=new PARAM;
 bufferlst=newparam->GetListPtr();         // well, I know that this is a little crude
 memcpy(newparam, param, sizeof(PARAM));   // first, we save the listptr, then we copy the whole object onto the new one
 newparam->SetListPtr(bufferlst);          // and here we restore it back

 // copy the contents of the LSTVALUE list
 for (i=0; i<param->GetListPtr()->Count; i++)
  {
  newlstvalue=new LSTVALUE;
  memcpy(newlstvalue, (LSTVALUE *)(((PARAM*)param)->GetListPtr()->Items[i]), sizeof(LSTVALUE));
  newparam->GetListPtr()->Add(newlstvalue);     // add the value to the list of values
  }

 MoveParameter2List(newparam);   // now, add the whole new parameter to our list
}


// **************************************************************************
// Function:   SetDimensions
// Purpose:    Sets the dimensions of a matrix parameter
//             It does not do anything, if the parameter is not a matrix
// Parameters: new_dimension1 - size in dimension 1
//             new_dimension2 - size in dimension 2
// Returns:    N/A
// **************************************************************************
void PARAM::SetDimensions(int new_dimension1, int new_dimension2)
{
 // at first, check whether this is a matrix parameter
 // if not, just return
 if (strcmpi(type, "matrix") != 0)
    return;

 dimension1=new_dimension1;
 dimension2=new_dimension2;
 SetNumValues(dimension1*dimension2);
}


// **************************************************************************
// Function:   PARAM
// Purpose:    The constructor for the PARAM object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
PARAM::PARAM()
{
 value_list=new TList;
 numvalues=0;
 tag=false;
}


// **************************************************************************
// Function:   PARAM
// Purpose:    Constructs and initializes a parameter object
// Parameters: self-explanatory
// Returns:    N/A
// **************************************************************************
PARAM::PARAM(const char *name, const char *section, const char *type,
             const char *value, const char *my_defaultvalue, const char *my_lowrange,
             const char *my_highrange, const char *my_comment)
{
 value_list=new TList;
 numvalues=1;
 tag=false;

 if (name)    SetName(name);
 if (section) SetSection(section);
 if (type)    SetType(type);
 if (value)   SetValue(value);

 if (my_defaultvalue) strncpy(defaultvalue, my_defaultvalue, LENGTH_DEFAULTVALUE);
 if (my_lowrange) strncpy(lowrange, my_lowrange, LENGTH_LOWRANGE);
 if (my_highrange) strncpy(highrange, my_highrange, LENGTH_HIGHRANGE);
 if (my_comment) strncpy(comment, my_comment, LENGTH_COMMENT);

 if ((!name) || (!section) || (!type) ||(!value))
    valid=false;
 else
    valid=true;
}


// **************************************************************************
// Function:   PARAM
// Purpose:    Constructs and initializes a parameter object, based upon
//             a parameter string, as defined in the project outline
// Parameters: char *paramstring
// Returns:    N/A
// **************************************************************************
PARAM::PARAM(const char *paramstring)
{
 tag=false;
 value_list=new TList;

 ParseParameter(paramstring, strlen(paramstring));
}


// **************************************************************************
// Function:   ~PARAM
// Purpose:    The destructor for the PARAM object
//             it deletes the list of values that this parameter holds
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
PARAM::~PARAM()
{
int     i;
LSTVALUE   *cur_value;

  // Clean up – must free memory for the items as well as the list
  for (i=0; i<value_list->Count; i++)
   {
   cur_value=(LSTVALUE *)value_list->Items[i];
   delete cur_value;
   }

 delete value_list;
}


// **************************************************************************
// Function:   GetValue
// Purpose:    Returns a pointer to the FIRST value of this parameter
//             (most parameters, except the *list parameter types, only
//             have one value anyways)
// Parameters: N/A
// Returns:    char pointer to the value
// **************************************************************************
const char *PARAM::GetValue() const
{
 return(((LSTVALUE *)((TList*)GetListPtr())->Items[0])->value);
}


// **************************************************************************
// Function:   GetNumValues
// Purpose:    Returns the number of values in this parameter
//             for most types of parameters, this will be one
// Parameters: N/A
// Returns:    number of values in this parameter
// **************************************************************************
int PARAM::GetNumValues() const
{
 return(numvalues);
}


// **************************************************************************
// Function:   SetNumValues
// Purpose:    Sets the number of values in this parameter
// Parameters: new number of values in this parameter
// Returns:    N/A
// **************************************************************************
void PARAM::SetNumValues(int new_numvalues)
{
 numvalues=new_numvalues;
}


// **************************************************************************
// Function:   GetDimension1
// Purpose:    Returns the number of values in dimension1 of this parameter
//             except for parameters of type matrix, this will always be one
// Parameters: N/A
// Returns:    number of values in dimension1 of this parameter
// **************************************************************************
int PARAM::GetNumValuesDimension1() const
{
 return(dimension1);
}


// **************************************************************************
// Function:   GetDimension2
// Purpose:    Returns the number of values in dimension2 of this parameter
//             except for parameters of type matrix, this will always be one
// Parameters: N/A
// Returns:    number of values in dimension2 of this parameter
// **************************************************************************
int PARAM::GetNumValuesDimension2() const
{
 return(dimension2);
}


// **************************************************************************
// Function:   GetValue
// Purpose:    Returns a pointer to the i'th value of this parameter
//             (most parameters, except the *list parameter types, only
//             have one value anyways)
// Parameters: idx ... index of the value
// Returns:    char pointer to the value
//             if idx is out of bounds, it returns a pointer to the first value
// **************************************************************************
const char *PARAM::GetValue(int idx) const
{
 if ((idx < 0) || (idx >= value_list->Count))
    idx=0;

 return(((LSTVALUE *)((TList*)GetListPtr())->Items[idx])->value);
}


// **************************************************************************
// Function:   GetValue
// Purpose:    Returns a pointer to the x1'th/x2'th value of this parameter
//             this call only makes sense, if the parameter is of type matrix
// Parameters: x1, x2 ... index of the value for both dimensions
// Returns:    char pointer to the value
//             if the idxs are out of bounds, it returns a pointer to the first value
// **************************************************************************
const char *PARAM::GetValue(int x1, int x2) const
{
int idx;

 idx=x1*dimension2+x2;
 return(GetValue(idx));
}


// **************************************************************************
// Function:   SetSection
// Purpose:    sets the section name of the parameter
// Parameters: char pointer to the section name
// Returns:    N/A
// **************************************************************************
void PARAM::SetSection(const char *src)
{
 strncpy(section, src, LENGTH_SECTION);
}


// **************************************************************************
// Function:   SetType
// Purpose:    sets the type of the parameter
// Parameters: char pointer to the type name
// Returns:    N/A
// **************************************************************************
void PARAM::SetType(const char *src)
{
 strncpy(type, src, LENGTH_TYPE);
}


// **************************************************************************
// Function:   SetName
// Purpose:    sets the name of the parameter
//             Note: parameter names are case insensitive
// Parameters: char pointer to the name
// Returns:    N/A
// **************************************************************************
void PARAM::SetName(const char *src)
{
 strncpy(name, src, LENGTH_NAME);
}


// **************************************************************************
// Function:   SetValue
// Purpose:    sets the (first) value of the parameter
// Parameters: char pointer to the value
// Returns:    N/A
// **************************************************************************
void PARAM::SetValue(const char *src)
{
char *buf;
int  length;
LSTVALUE        *lstvalue;

 length=strlen(src);
 if (length > LENGTH_VALUE) length=LENGTH_VALUE;

 // does at least one value exist ?
 if (value_list->Count > 0)
    {
    lstvalue=((LSTVALUE *)GetListPtr()->Items[0]);
    strncpy(lstvalue->value, src, length+1);
    }
 else
    {
    lstvalue=new LSTVALUE;
    strncpy(lstvalue->value, src, length+1);
    value_list->Add(lstvalue);
    }
}


// **************************************************************************
// Function:   SetValue
// Purpose:    sets the idx'th value of the parameter
// Parameters: src - char pointer to the value
//             idx - index of the value (0...GetNumValues()-1)
// Returns:    N/A
// **************************************************************************
void PARAM::SetValue(const char *src, int idx)
{
char            *buf;
int             length, i, num2add;
LSTVALUE        *lstvalue;

 length=strlen(src);
 if (length > LENGTH_VALUE) length=LENGTH_VALUE;

 if (idx < 0) return;

 num2add=idx-value_list->Count+1;
 if (num2add > 0)
    {
    for (i=0; i<num2add; i++)
     {
     lstvalue=new LSTVALUE;
     strncpy(lstvalue->value, "0", 2);
     value_list->Add(lstvalue);
     }
    // numvalues+=num2add;
    }

 lstvalue=((LSTVALUE *)GetListPtr()->Items[idx]);
 if (src[0] != '\0')
    strncpy(lstvalue->value, src, length+1);
 else
    strncpy(lstvalue->value, "0", 2);
}


// **************************************************************************
// Function:   SetValue
// Purpose:    sets the x1'th/x2'th value of the parameter
//             only makes sense, if the type of the parameter is matrix
// Parameters: src - char pointer to the value
//             x1 - index of the value (0...GetNumValuesDimension1()-1)
//             x2 - index of the value (0...GetNumValuesDimension2()-1)
// Returns:    N/A
// **************************************************************************
void PARAM::SetValue(const char *src, int x1, int x2)
{
int idx;

 idx=x1*dimension2+x2;
 SetValue(src, idx);
}


// **************************************************************************
// Function:   GetSection
// Purpose:    Returns a pointer to this parameter's section name
// Parameters: N/A
// Returns:    char pointer to the section name
// **************************************************************************
const char *PARAM::GetSection() const
{
 return(section);
}


// **************************************************************************
// Function:   GetType
// Purpose:    Returns a pointer to this parameter's type
// Parameters: N/A
// Returns:    char pointer to the type
// **************************************************************************
const char *PARAM::GetType() const
{
 return(type);
}


// **************************************************************************
// Function:   GetName
// Purpose:    Returns a pointer to this parameter's name
//             Note: parameter names are case insensitive
// Parameters: N/A
// Returns:    char pointer to the name
// **************************************************************************
const char *PARAM::GetName() const
{
 return(name);
}


// **************************************************************************
// Function:   GetComment
// Purpose:    Returns a pointer to this parameter's comment
// Parameters: N/A
// Returns:    char pointer to the comment
// **************************************************************************
const char *PARAM::GetComment() const
{
 return(comment);
}


// **************************************************************************
// Function:   GetListPtr
// Purpose:    Returns a pointer to this parameter's list of values
// Parameters: N/A
// Returns:    pointer to the list
// **************************************************************************
TList   *PARAM::GetListPtr()
{
 return (value_list);
}

const TList   *PARAM::GetListPtr() const
{
 return (value_list);
}


// **************************************************************************
// Function:   SetListPtr
// Purpose:    Sets this parameter's list of values
// Parameters: pointer to the list
// Returns:    N/A
// **************************************************************************
void    PARAM::SetListPtr(TList *temp_list)
{
 value_list=temp_list;
}


// **************************************************************************
// Function:   get_argument
// Purpose:    parses the parameter line that is being sent in the core
//             communication, or as stored in any BCI2000 .prm file
//             it returns the next token that is being delimited by either
//             a ' ' or '='
// Parameters: ptr - index into the line of where to start
//             buf - destination buffer for the token
//             line - the whole line
//             maxlen - maximum length of the line
// Returns:    the index into the line where the returned token ends
// **************************************************************************
int PARAM::get_argument(int ptr, char *buf, const char *line, int maxlen) const
{
 // skip trailing spaces, if any
 while ((line[ptr] == '=') || (line[ptr] == ' ') && (ptr < maxlen))
  ptr++;

 // go through the string, until we either hit a space, a '=', or are at the end
 while ((line[ptr] != '=') && (line[ptr] != ' ') && (line[ptr] != '\n') && (line[ptr] != '\r') && (ptr < maxlen))
  {
  *buf=line[ptr];
  ptr++;
  buf++;
  }

 *buf=0;
 return(ptr);
}


// **************************************************************************
// Function:   GetParamLine
// Purpose:    Returns a parameter line in ASCII format
//             Tis parameter line is constructed, based upon the current
//             values in the PARAM object
// Parameters: N/A
// Returns:    a pointer to the parameter line
// **************************************************************************
const char *PARAM::GetParamLine() const
{
 // construct the parameter line
 ConstructParameterLine();
 return(buffer);
}

// **************************************************************************
// Function:   ConstructParamLine
// Purpose:    Construct a parameter line, based upon the current values
//             in the PARAM object
// Parameters: N/A
// Returns:    ERRPARAM_NOERR
// **************************************************************************
int PARAM::ConstructParameterLine() const
{
int             i;
LSTVALUE        *cur_value;

 // if it is of data type *list, write the numvalues parameter, otherwise not
 if ((strcmpi(type, "intlist") == 0) || (strcmpi(type, "floatlist") == 0) || (strcmpi(type, "matrix") == 0))
    {
    if (strcmpi(type, "matrix") == 0)
       sprintf(buffer, "%s %s %s= %d %d ", section, type, name, dimension1, dimension2);
    else
       sprintf(buffer, "%s %s %s= %d ", section, type, name, numvalues);
    }
 else
    sprintf(buffer, "%s %s %s= ", section, type, name);

 for (i=0; i<numvalues; i++)
  {
  cur_value=(LSTVALUE *)value_list->Items[i];
  strncat(buffer, cur_value->value, LENGTH_VALUE);
  strcat(buffer, " ");
  }

 strncat(buffer, defaultvalue, LENGTH_DEFAULTVALUE);
 strcat(buffer, " ");
 strncat(buffer, lowrange, LENGTH_LOWRANGE);
 strcat(buffer, " ");
 strncat(buffer, highrange, LENGTH_HIGHRANGE);
 strcat(buffer, " // ");
 strncat(buffer, comment, LENGTH_COMMENT);

 return(ERRPARAM_NOERR);
}


// **************************************************************************
// Function:   ParseParameter
// Purpose:    This routine is called by coremessage->ParseMessage()
//             it parses the provided ASCII parameter line and initializes
//             the PARAM object accordingly, i.e., it fills in values, name,
//             section name, type, etc.
// Parameters: line - pointer to the ASCII parameter line
//             length - length of this parameter line
// Returns:    ERRPARAM_INVALIDPARAM if the parameter line is invalid, or
//             ERRPARAM_NOERR
// **************************************************************************
int PARAM::ParseParameter(const char *new_line, int length)
{
int     ptr, i;
char    *buf, *remptr, *filterptr;
char    line[LENGTH_PARAMLINE+1];
LSTVALUE *newlistvalue;
bool    dontcontinue;

 strncpy(line, new_line, length);
 line[length]=0;
 filterptr=strchr(line, '\r');
 if (filterptr) *filterptr=0;
 filterptr=strchr(line, '\n');
 if (filterptr) *filterptr=0;

 dontcontinue=false;
 valid=false;
 numvalues=1;
 section[0]=0;
 name[0]=0;
 dimension1=1;
 dimension2=1;

 // buf=new char[length+1];
 buf=(char *)malloc(length+1);

 ptr=0;
 ptr=get_argument(ptr, buf, line, length);
 strncpy(section, buf, LENGTH_SECTION);
 ptr=get_argument(ptr, buf, line, length);
 strncpy(type, buf, LENGTH_TYPE);
 ptr=get_argument(ptr, buf, line, length);
 strncpy(name, buf, LENGTH_NAME);
 if ((section[0] == 0) || (type[0] == 0) || (name[0] == 0))
    {
    free(buf);
    return(ERRPARAM_INVALIDPARAM);
    }

 // if it is of data type *list, expect more than one parameters
 if ((strcmpi(type, "intlist") == 0) || (strcmpi(type, "floatlist") == 0) || (strcmpi(type, "matrix") == 0))
    {
    if (strcmpi(type, "matrix") == 0)
       {
       ptr=get_argument(ptr, buf, line, length);
       dimension1=atoi(buf);
       ptr=get_argument(ptr, buf, line, length);
       dimension2=atoi(buf);
       numvalues=dimension1*dimension2;
       }
    else
       {
       ptr=get_argument(ptr, buf, line, length);
       numvalues=atoi(buf);
       }

    i=0;
    while (i < numvalues)
     {
     ptr=get_argument(ptr, buf, line, length);
     if ((buf[0] == '/') && (buf[1] == '/'))
        dontcontinue=true;
     newlistvalue=new LSTVALUE;
     if (!dontcontinue)
        strncpy(newlistvalue->value, buf, LENGTH_VALUE);
     else
        strcpy(newlistvalue->value, "0");
     value_list->Add(newlistvalue);
     i++;
     }
    }
 else
    {
    numvalues=1;
    ptr=get_argument(ptr, buf, line, length);
    newlistvalue=new LSTVALUE;
    strncpy(newlistvalue->value, buf, LENGTH_VALUE);
    newlistvalue->value[LENGTH_VALUE]=0;
    value_list->Add(newlistvalue);
    }

 if (strcmp(name, "BoardName") == 0)
    dontcontinue=dontcontinue;

 // now, parse default, lowrange and highrange
 // in case we find //, then insert default values
 ptr=get_argument(ptr, buf, line, length);
 if (((buf[0] == '/') && (buf[1] == '/')) || (dontcontinue))
    {
    strcpy(defaultvalue, "0");
    dontcontinue=true;
    }
 else
    strncpy(defaultvalue, buf, LENGTH_DEFAULTVALUE);
 ptr=get_argument(ptr, buf, line, length);
 if (((buf[0] == '/') && (buf[1] == '/')) || (dontcontinue))
    {
    strcpy(lowrange, "0");
    dontcontinue=true;
    }
 else
    strncpy(lowrange, buf, LENGTH_LOWRANGE);
 ptr=get_argument(ptr, buf, line, length);
 if (((buf[0] == '/') && (buf[1] == '/')) || (dontcontinue))
    {
    strcpy(highrange, "0");
    dontcontinue=true;
    }
 else
    strncpy(highrange, buf, LENGTH_HIGHRANGE);

 remptr=strchr(line, '/');
 comment[0]=0;                                  // set the first character to zero
 if (remptr != NULL)
    {
    i=1;
    while ((i < (int)strlen(remptr)) && ((remptr[i] == ' ') || (remptr[i] == '/')))
     i++;
    strncpy(comment, &remptr[i], LENGTH_COMMENT);
    }

 free(buf);
 // delete(buf);
 valid=true;
 return(ERRPARAM_NOERR);
}

