#ifndef EXPORT_H_INCLUDED
#define EXPORT_H_INCLUDED                                                   

//#include "array.h"

#ifdef WIN32 
  #include <winsock.h>
#else
  #include <arpa/inet.h>
#endif

//template <typename T> int load_matlab_matrix(mxArray **parray, FILE *pfile);
//int save_matlab_matrix(mxArray *array, FILE *pfile);

template <typename T> int write_file(T *pvalue, int num_elements, FILE *pfile) 
{
  T value_networkorder;

  for (int index=0; index < num_elements; index++) {
    value_networkorder = htonl((int)pvalue[index]);
    fwrite(&value_networkorder,sizeof(T),1,pfile);  
  }

  return(num_elements);
}

template <typename T> int read_file(T *pvalue, int num_elements, FILE *pfile) 
{
  T value_networkorder;

  for (int index=0; index < num_elements; index++) {
    fread(&value_networkorder,sizeof(T),1,pfile);
    pvalue[index] = ntohl(value_networkorder);
  }

  return(num_elements);
}



#endif
