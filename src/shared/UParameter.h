//---------------------------------------------------------------------------
#ifndef UParameterH
#define UParameterH

#define LENGTH_SECTION          30
#define LENGTH_TYPE             30
#define LENGTH_NAME             30
#define LENGTH_NUMVALUES        3
#define LENGTH_VALUE            50
#define LENGTH_DEFAULTVALUE     50
#define LENGTH_LOWRANGE         50
#define LENGTH_HIGHRANGE        50
#define LENGTH_COMMENT          255
#define LENGTH_PARAMLINE        65536

#define ERRPARAM_NOERR                  0
#define ERRPARAM_INCONSISTENTNUMVAL     1
#define ERRPARAM_INVALIDPARAM           2

#include <ScktComp.hpp>


class LSTVALUE
{
public:		// User declarations
        char    value[LENGTH_VALUE+1];
};


class PARAM
{
private: 	// User declarations
        char    buffer[LENGTH_PARAMLINE];
        char    section[LENGTH_SECTION+1];      // length + 1 because of zero byte
        char    type[LENGTH_TYPE+1];
        char    name[LENGTH_NAME+1];
        int     numvalues;
        int     dimension1, dimension2;
        TList   *value_list;
        char    defaultvalue[LENGTH_DEFAULTVALUE+1];
        char    lowrange[LENGTH_LOWRANGE+1];
        char    highrange[LENGTH_HIGHRANGE+1];
        char    comment[LENGTH_COMMENT+1];
public:		// User declarations
        PARAM::PARAM();
        PARAM::PARAM(char *name, char *section, char *type, char *value, char *defaultvalue, char *lowrange, char *highrange, char *comment);
        PARAM::PARAM(char *paramstring);
        PARAM::~PARAM();
        int     get_argument(int ptr, char *buf, char *line, int maxlen);
        void    SetSection(char *);
        void    SetType(char *);
        void    SetName(char *);
        void    SetValue(char *);
        void    SetValue(char *, int);
        void    SetValue(char *, int, int);
        void    SetNumValues(int new_numvalues);
        char    *GetSection();
        char    *GetType();
        char    *GetName();
        char    *GetComment();
        char    *GetParamLine();
        TList   *GetListPtr();
        void    SetListPtr(TList *);
        int     GetNumValues();
        int     GetNumValuesDimension1();
        int     GetNumValuesDimension2();
        void    SetDimensions(int new_dimension1, int new_dimension2);
        char    *GetValue();
        char    *GetValue(int idx);
        char    *GetValue(int idx1, int idx2);
        int     ParseParameter(char *line, int length);
        int     ConstructParameterLine();
        bool    valid;
        bool    archive;
        bool    tag;                    // important for parameter save/load filters
};


class PARAMLIST
{
private: 	// User declarations
        TList   *param_list;
public:		// User declarations
        PARAMLIST::PARAMLIST();
        PARAMLIST::~PARAMLIST();
        int     GetNumParameters();
        PARAM   *GetParamPtr(int idx);
        PARAM   *GetParamPtr(char *name);
        void    CloneParameter2List(PARAM *param);
        void    MoveParameter2List(PARAM *param);
        void    ClearParamList();
        void    Sort();
        void    AddParameter2List(char *paramstring, int paramlen);
        void    DeleteParam(char *name);
        bool    SaveParameterList(char *filename);
        bool    SaveParameterList(char *filename, bool usetags);
        bool    LoadParameterList(char *filename);
        bool    LoadParameterList(char *filename, bool usetags, bool importnonexisting);
        int     PublishParameters(TCustomWinSocket *Socket);
};

//---------------------------------------------------------------------------
#endif



