//---------------------------------------------------------------------------
#ifndef UParameterH
#define UParameterH

#define LENGTH_SECTION          30
#define LENGTH_TYPE             30
#define LENGTH_NAME             30
#define LENGTH_NUMVALUES        3
#define LENGTH_VALUE            100
#define LENGTH_DEFAULTVALUE     100
#define LENGTH_LOWRANGE         100
#define LENGTH_HIGHRANGE        100
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
  friend class PARAMLIST;
  friend class TfConfig; // Let's get rid of these friends as soon as possible...
  friend class TfShowParameters;
  
private: 	// User declarations
        mutable char    buffer[LENGTH_PARAMLINE];
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
        PARAM::PARAM(const char *name, const char *section, const char *type, const char *value, const char *defaultvalue, const char *lowrange, const char *highrange, const char *comment);
        PARAM::PARAM(const char *paramstring);
        PARAM::~PARAM();
        void    SetSection(const char *);
        void    SetType(const char *);
        void    SetName(const char *);
        void    SetValue(const char *);
        void    SetValue(const char *, int);
        void    SetValue(const char *, int, int);
        void    SetNumValues(int new_numvalues);
        const char    *GetSection() const;
        const char    *GetType() const;
        const char    *GetName() const;
        const char    *GetComment() const;
        const char    *GetParamLine() const;
        int     GetNumValues() const;
        int     GetNumValuesDimension1() const;
        int     GetNumValuesDimension2() const;
        void    SetDimensions(int new_dimension1, int new_dimension2);
        const char    *GetValue() const;
        const char    *GetValue(int idx) const;
        const char    *GetValue(int idx1, int idx2) const;
        int     ParseParameter(const char *line, int length);
        bool    Valid() { return valid; }

  protected:
        int     get_argument(int ptr, char *buf, const char *line, int maxlen) const;
        TList   *GetListPtr();
        const TList* GetListPtr() const;
        void    SetListPtr(TList *);
        bool    valid;
        bool    archive;
        bool    tag;                    // important for parameter save/load filters

  private:
        int     ConstructParameterLine() const;
};


class PARAMLIST
{
private: 	// User declarations
        TList   *param_list;
public:		// User declarations
        PARAMLIST::PARAMLIST();
        PARAMLIST::~PARAMLIST();
        int     GetNumParameters() const;
        PARAM   *GetParamPtr(int idx);
        const PARAM   *GetParamPtr(int idx) const;
        PARAM   *GetParamPtr(const char *name);
        const PARAM   *GetParamPtr(const char *name) const;
        void    CloneParameter2List(const PARAM *param);
        void    MoveParameter2List(PARAM *param);
        void    ClearParamList();
        void    Sort();
        void    AddParameter2List(const char *paramstring, int paramlen = 0);
        void    DeleteParam(const char *name);
        bool    SaveParameterList(const char *filename) const;
        bool    SaveParameterList(const char *filename, bool usetags) const;
        bool    LoadParameterList(const char *filename);
        bool    LoadParameterList(const char *filename, bool usetags, bool importnonexisting);
};

//---------------------------------------------------------------------------
#endif



