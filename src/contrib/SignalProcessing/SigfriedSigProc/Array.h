#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED                                                   


#include <stdlib.h>
#include <stdio.h>
#include "export.h"

////////////////////////////////////////////////////////////////////////////////////

typedef void *MEXMALLOC(size_t n, size_t size);


typedef void MEXFREE(void *ptr);


///////////////////////////////////////////////////////////////////////////////
/// Vector template class.
///////////////////////////////////////////////////////////////////////////////
template <class T> class CVector //: public Shared
{
  public:
//    typedef SharedPtr< CVector<T> > Ptr;

    CVector();
    CVector(T*,int);
    CVector(T*,int,int);
    CVector(T*,int,int,int);

    CVector(int,int,int,MEXMALLOC *mexmalloc=calloc, MEXFREE *mexfree=free);
    CVector(int,int,MEXMALLOC *mexmalloc=calloc, MEXFREE *mexfree=free);
    CVector(int,MEXMALLOC *mexmalloc=calloc, MEXFREE *mexfree=free);

    void Destroy();

    void resize(int,MEXMALLOC *mexmalloc=calloc, MEXFREE *mexfree=free);
    void resize(int,int,MEXMALLOC *mexmalloc=calloc, MEXFREE *mexfree=free);
    void resize(int,int,int,MEXMALLOC *mexmalloc=calloc, MEXFREE *mexfree=free);

    void set(T* pVector,int);
    void set(T* pVector,int,int);
    void set(T* pVector,int,int,int);


    void copy(CVector *psource, MEXMALLOC *mexmalloc=calloc, MEXFREE *mexfree=free);

    int save(char *szFile);
    int load(char *szFile);
    int save(FILE *pfile);
    int load(FILE *pfile);

//    int DecomposeCholesky(T *matrix_in, T *matrix_cholesky_out, int num_dimensions);
    int DecomposeCholesky(CVector<T> *matrix, CVector<T> *matrix_cholesky);

    int getDimension() { return m_nDimension; }
    T*  getPtr()       { return m_pVector;    }
    int getSize()      { return (m_nSize[1] * m_nSize[2] * m_nSize[3]); }


    T getMax(int *pindex=NULL)
    {

      int num_samples = getDimM() * getDimN() * getDimZ();
      int index_max   = 0;

      T num_max = 0;
      for (int index=0; index < num_samples; index++) {
        if (m_pVector[index] > num_max) {
          num_max   = m_pVector[index];
          index_max = index;
        }
      }  

      if (pindex != NULL) *pindex = index_max;

      return num_max;

    }

    T getMin(int *pindex=NULL)
    {

      int num_samples = getDimM() * getDimN() * getDimZ();
      int index_min   = 0;

      T num_min = getMax();
      for (int index=0; index < num_samples; index++) {
        if (m_pVector[index] < num_min) {
          num_min   = m_pVector[index];
          index_min = index;
        }
      }  

      if (pindex != NULL) *pindex = index_min;

      return num_min;

    }


    void zero() {
      
      for (int index=0; index < getDimZ() * getDimM() * getDimN(); index++) {
        m_pVector[index] = 0;
      }

    }

    T sum() {

      T total=0;

      for (int index=0; index < getDimZ() * getDimM() * getDimN(); index++) {
        total+=m_pVector[index];
      }

      return total;
    }


    ~CVector();

    ///////////////////////////////////////////////////////////////////////////////
    /// Returns Element of {@link #m_pVector} with the index <CODE>nIndex</CODE>.
    /// @param  nIndex Index of element to be returned.
    /// @return Element of {@link #m_pVector} indexed by <CODE>nIndex</CODE>.
    ///////////////////////////////////////////////////////////////////////////////
    T &operator[](int nIndex) 
    {
      return m_pVector[nIndex];
    }

    T &operator()(int nIndex1) 
    {
#ifdef DEBUG
      if (nIndex1 >= m_nSize[1] || nIndex1 < 0)  { 
        throw 1; 
      }
#endif
      return m_pVector[nIndex1];
    }

    T &operator()(int nIndex1, int nIndex2) 
    {
#ifdef DEBUG
      if (nIndex1 >= m_nSize[2] || nIndex2 < 0)  { 
        throw 1; 
      }
      if (nIndex2 >= m_nSize[1] || nIndex1 < 0)  { 
        throw 1; 
      }
#endif
      return *(m_pVector+nIndex2*m_nSize[2]+nIndex1);
    }
                 //   zeile        spalte       tiefe
    T &operator()(int nIndex1, int nIndex2, int nIndex3) 
    {
#ifdef DEBUG
      if (nIndex1 >= m_nSize[3] || nIndex3 < 0)  { 
        throw 1; 
      }
      if (nIndex2 >= m_nSize[2] || nIndex2 < 0)  { 
        throw 1; 
      }
      if (nIndex3 >= m_nSize[1] || nIndex1 < 0)  { 
        throw 1; 
      }
#endif
      
      unsigned int offset = nIndex3 * (m_nSize[2] * m_nSize[3]) + nIndex2 * m_nSize[3] + nIndex1;
     // return *(m_pVector+nIndex3*m_nSize[3]+nIndex2*m_nSize[2]+nIndex1);
      return m_pVector[offset];
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// Returns {@link #m_pVector}.
    /// @return {@link #m_pVector}.
    ///////////////////////////////////////////////////////////////////////////////
    operator T*() const 
    {
      return m_pVector;
    }

    int getDimM() { return m_nSize[2]; };
    int getDimN() { return m_nSize[1]; };
    int getDimZ() { return m_nSize[3]; };

  private:
    ///////////////////////////////////////////////////////////////////////////////
    /// Pointer to the elements of the template vector.
    ///////////////////////////////////////////////////////////////////////////////
    T *m_pVector;

    int m_nDimension;
    int m_nSize[4];
    int m_nSizeElement;

    int m_bFree;

    MEXMALLOC *m_mexmalloc;
    MEXFREE   *m_mexfree;

    void *m_this;

};


// standard constructor

template <class T> CVector<T>::CVector() 
{
  m_pVector       = NULL;
  m_bFree         = false;
  m_mexmalloc     = NULL;
  m_mexfree       = NULL;

  m_nDimension    = 0;
  m_nSize[1]      = 0;
  m_nSize[2]      = 0;
  m_nSize[3]      = 0;
  m_nSizeElement  = sizeof(T);
  m_this          = this;
}

// one dimension 

template <class T> void CVector<T>::resize(int dim1, MEXMALLOC *mexmalloc, MEXFREE *mexfree)
{
  if (m_bFree && m_mexfree && m_this == this) {
    m_mexfree(m_pVector);
  }

  m_pVector       = (T*)mexmalloc(dim1, sizeof(T));
  m_bFree         = true;
  m_mexmalloc     = mexmalloc;
  m_mexfree       = mexfree;

  m_nDimension    = 1;
  m_nSize[1]      = dim1;
  m_nSize[2]      = 1;
  m_nSize[3]      = 1;
  m_nSizeElement  = sizeof(T);
}

template <class T> CVector<T>::CVector(int dim1, MEXMALLOC *mexmalloc, MEXFREE *mexfree) 
{ 
  m_bFree         = false;
  resize(dim1,mexmalloc,mexfree);
  m_this          = this;
}

template <class T> CVector<T>::CVector(T* pVector, int dim1) 
{
  m_bFree         = false;
  m_mexmalloc     = NULL;
  m_mexfree       = NULL;
  m_this          = this;

  set(pVector,dim1);
/* 
  m_pVector       = pVector;
  m_bFree         = false;
  m_mexmalloc     = NULL;
  m_mexfree       = NULL;

  m_nDimension    = 1;
  m_nSize[1]      = dim1;
  m_nSize[2]      = 1;
  m_nSize[3]      = 1;
  m_nSizeElement  = sizeof(T);
  m_this          = this;
*/
}

template <class T> void CVector<T>::set(T* pVector, int dim1) 
{
  if (m_bFree && m_mexfree != NULL && m_this == this) {
    m_mexfree(m_pVector);
  }

  m_pVector       = pVector;
  m_bFree         = false;

  m_nDimension    = 1;
  m_nSize[1]      = dim1;
  m_nSize[2]      = 1;
  m_nSize[3]      = 1;
  m_nSizeElement  = sizeof(T);
}


// two dimensions

template <class T> void CVector<T>::resize(int dim2, int dim1,  MEXMALLOC *mexmalloc, MEXFREE *mexfree)
{
  if (m_bFree && m_mexfree != NULL && m_this == this) {
    mexfree(m_pVector);
  }

  m_pVector       = (T*)mexmalloc(dim2 * dim1, sizeof(T));
  m_bFree         = true;
  m_mexmalloc     = mexmalloc;
  m_mexfree       = mexfree;

  m_nDimension    = 2;
  m_nSize[1]      = dim1;  // spalten
  m_nSize[2]      = dim2;  // zeilen
  m_nSize[3]      = 1;
  m_nSizeElement  = sizeof(T);

}

template <class T> CVector<T>::CVector(int dim2, int dim1,  MEXMALLOC *mexmalloc, MEXFREE *mexfree) 
{ 
  m_bFree         = false;
  resize(dim2,dim1,mexmalloc,mexfree);
  m_this          = this;
}

template <class T> CVector<T>::CVector(T* pVector, int dim2, int dim1) 
{ 
  m_bFree         = false;
  m_mexmalloc     = NULL;
  m_mexfree       = NULL;
  m_this          = this;

  set(pVector,dim2,dim1);

/*
  m_pVector       = pVector;
  m_bFree         = false;
  m_mexmalloc     = NULL;
  m_mexfree       = NULL;

  m_nDimension    = 2;
  m_nSize[1]      = dim1;  // spalten
  m_nSize[2]      = dim2;  // zeilen
  m_nSize[3]      = 1;
  m_nSizeElement  = sizeof(T);
  m_this          = this;
*/
}

template <class T> void CVector<T>::set(T* pVector, int dim2, int dim1) 
{ 
  if (m_bFree && m_mexfree != NULL && m_this == this) {
    m_mexfree(m_pVector);
  }

  m_pVector       = pVector;
  m_bFree         = false;
  m_mexmalloc     = NULL;
  m_mexfree       = NULL;

  m_nDimension    = 2;
  m_nSize[1]      = dim1;  // spalten
  m_nSize[2]      = dim2;  // zeilen
  m_nSize[3]      = 1;
  m_nSizeElement  = sizeof(T);
  m_this          = this;
}


// three dimensions

template <class T> void CVector<T>::resize(int dim3, int dim2, int dim1,  MEXMALLOC *mexmalloc, MEXFREE *mexfree)
{
  if (m_bFree && m_mexfree != NULL && m_this == this) {
    mexfree(m_pVector);
  }

  m_pVector       = (T*)mexmalloc(dim3 * dim2 * dim1, sizeof(T));
  m_bFree         = true;
  m_mexmalloc     = mexmalloc;
  m_mexfree       = mexfree;

  m_nDimension    = 3;
  m_nSize[1]      = dim1; // tiefe
  m_nSize[2]      = dim2; // spalten
  m_nSize[3]      = dim3; // zeilen
  m_nSizeElement  = sizeof(T);

}



template <class T> CVector<T>::CVector(int dim3, int dim2, int dim1, MEXMALLOC *mexmalloc, MEXFREE *mexfree)
{ 
  m_bFree         = false;
  resize(dim3,dim2,dim1,mexmalloc,mexfree);
  m_this          = this;
}

template <class T> CVector<T>::CVector(T* pVector, int dim3, int dim2, int dim1) 
{ 
  m_bFree         = false;
  m_mexmalloc     = NULL;
  m_mexfree       = NULL;
  m_this          = this;

  set(pVector,dim3,dim2,dim1);

/*
  m_pVector       = pVector;
  m_bFree         = false;
  m_mexmalloc     = NULL;
  m_mexfree       = NULL;

  m_nDimension    = 3;
  m_nSize[1]      = dim1; // tiefe
  m_nSize[2]      = dim2; // spalten
  m_nSize[3]      = dim3; // zeilen
  m_nSizeElement  = sizeof(T);
  m_this          = this;
*/
}


template <class T> void CVector<T>::set(T* pVector, int dim3, int dim2, int dim1) 
{ 
  if (m_bFree && m_mexfree != NULL && m_this == this) {
    m_mexfree(m_pVector);
  }
  
  m_pVector       = pVector;
  m_bFree         = false;
  m_mexmalloc     = NULL;
  m_mexfree       = NULL;

  m_nDimension    = 3;
  m_nSize[1]      = dim1; // tiefe
  m_nSize[2]      = dim2; // spalten
  m_nSize[3]      = dim3; // zeilen
  m_nSizeElement  = sizeof(T);
  m_this          = this;
}



///////////////////////////////////////////////////////////////////////////////
/// Destroys the instance of the class.
///////////////////////////////////////////////////////////////////////////////
template <class T> CVector<T>::~CVector()
{
  if(m_this == this) {

    if (m_bFree && m_mexfree != NULL && getSize() > 0) m_mexfree(m_pVector);

  }
}

template <class T> void CVector<T>::Destroy()
{
  if (m_bFree && m_mexfree != NULL) m_mexfree(m_pVector);

  m_pVector       = NULL;
  m_bFree         = false;

  m_nDimension    = 0;
  m_nSize[1]      = 0;
  m_nSize[2]      = 0;
  m_nSize[3]      = 0;
  m_nSizeElement  = sizeof(T);

}


template <class T> void CVector<T>::copy(CVector *psource, MEXMALLOC *mexmalloc, MEXFREE *mexfree) {
/*
  if (psource->getDimension() != getDimension() || 
      psource->getDimN()      != getDimN()      ||
      psource->getDimM()      != getDimM()      ||
      psource->getDimZ()      != getDimZ())
  {
*/
    if (psource->getDimension() == 1) {
      resize(psource->getDimN(),mexmalloc,mexfree);
    } else if (psource->getDimension() == 2) {
      resize(psource->getDimM(),psource->getDimN(),mexmalloc,mexfree);
    } else if (psource->getDimension() == 3) {
      resize(psource->getDimZ(),psource->getDimM(),psource->getDimN(),mexmalloc,mexfree);
    } else {
      // do nothing
    } 
//  }

  memcpy(m_pVector,psource->getPtr(),m_nSizeElement * psource->getDimZ() * psource->getDimM() * psource->getDimN());

}



////////////////////////////////////////////////////////////////////////////////////

template <class T> void SolveLowerTriangular(CVector<T> *pmatA, CVector<T> *pmatX) 
{
  int m,n,i,j,spalte;
  T factor;

  m = pmatA->getDimM();
  n = pmatA->getDimN();

  if (n != m) return;



  // initialize matX with unity matrix
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      if (i==j) {
        (*pmatX)(i,j) = 1;
      } else {
        (*pmatX)(i,j) = 0;
      }
    }
  }

  // divide with pivot elements
  for (i=0; i<n; i++) {

    (*pmatX)(i,i) = (*pmatX)(i,i) / (*pmatA)(i,i);    
    
    for (j=0; j<=i; j++) {
      (*pmatA)(i,j) = (*pmatA)(i,j) / (*pmatA)(i,i);
    }
  }

  // solve lower triangular matrix
  for (spalte=0; spalte<n; spalte++) {
    for (i=spalte+1; i<n; i++) {
      factor = (*pmatA)(i,spalte);

      for (j=0; j<=spalte; j++) {
       (*pmatA)(i,j) = (*pmatA)(i,j) - factor * (*pmatA)(spalte,j);
       (*pmatX)(i,j) = (*pmatX)(i,j) - factor * (*pmatX)(spalte,j);
      }
    }
  }

}

template <class T> void MatTransp(CVector<T> *pmat1, CVector<T> *pmatX) 
{
  int i,j,m1,mx,n1,nx;

  m1 = pmat1->getDimM();
  n1 = pmat1->getDimN();
  mx = pmatX->getDimM();
  nx = pmatX->getDimN();

  if (n1 != nx) return;
  if (m1 != mx) return;

  for (i=0; i<m1; i++) {
    for (j=0; j<n1; j++) {
        (*pmatX)(i,j) = (*pmat1)(j,i);
    }
  }

}


template <class T> void MatMul(CVector<T> *pmat1, CVector<T> *pmat2, CVector<T> *pmatX) 
{
  int i,j,k,m1,m2,mx,n1,n2,nx;

  m1 = pmat1->getDimM();
  n1 = pmat1->getDimN();
  m2 = pmat2->getDimM();
  n2 = pmat2->getDimN();
  mx = pmatX->getDimM();
  nx = pmatX->getDimN();

  if (n1 != m2) return;
  if (m1 != mx) return;
  if (n2 != nx) return;


  for (i=0; i<mx; i++) {
    for (j=0; j<nx; j++) {
      (*pmatX)(i,j) = 0;
    }
  }

  for (i=0; i<m1; i++) {
    for (j=0; j<n2; j++) {
      for (k=0; k<n1; k++) {
        (*pmatX)(i,j) = (*pmatX)(i,j) + (*pmat1)(i,k) * (*pmat2)(k,j);
      }
    }
  }

}

template <class T> void MatSub(CVector<T> *pmat1, CVector<T> *pmat2, CVector<T> *pmatX) 
{
  int i,j,m1,m2,mx,n1,n2,nx;

  m1 = pmat1->getDimM();
  n1 = pmat1->getDimN();
  m2 = pmat2->getDimM();
  n2 = pmat2->getDimN();
  mx = pmatX->getDimM();
  nx = pmatX->getDimN();

  if (m1 != m2) return;
  if (n1 != n2) return;
  if (n1 != nx) return;
  if (m1 != mx) return;

  for (i=0; i<mx; i++) {
    for (j=0; j<nx; j++) {
      (*pmatX)(i,j) = (*pmat1)(i,j) - (*pmat2)(i,j);
    }
  }

}
/*
template <class T> int CVector<T>::save(char *szFile)  
{
  try {
    char szFileDim[1024];
    char szFileDat[1024];
    int  nBytes;


    strcat(strcpy(szFileDim,szFile),".dim");
    strcat(strcpy(szFileDat,szFile),".dat");

    FILE *pFile;
    pFile = fopen(szFileDim,"wb");
    if (NULL != pFile) {
      fwrite(&m_nDimension,sizeof(int),1,pFile);
      fwrite(&m_nSize[1],sizeof(int),3,pFile);
      fclose(pFile);
      pFile = fopen(szFileDat,"wb");
      if (NULL != pFile) {
        nBytes = fwrite(getPtr(),sizeof(T),getSize(),pFile);
        fclose(pFile);
        return nBytes;
      }
    }

    return -1;
  } catch (...) {
    return -1;
  }

}


template <class T> int CVector<T>::load(char *szFile)  
{
  try {
    char szFileDim[1024];
    char szFileDat[1024];
    int  nBytes;

    int nDimension;
    int nSize[4];

    strcat(strcpy(szFileDim,szFile),".dim");
    strcat(strcpy(szFileDat,szFile),".dat");
    
    FILE *pFile;
    pFile = fopen(szFileDim,"rb");
    if (NULL != pFile) {
      fread(&nDimension,sizeof(int),1,pFile);
      fread(&nSize[1],sizeof(int),3,pFile);
      fclose(pFile);

      if (1 == nDimension) {
        resize(nSize[1]);
      } else if (2 == nDimension) {
        resize(nSize[2],nSize[1]);
      } else if (3 == nDimension) {
        resize(nSize[3],nSize[2],nSize[1]);       
      } else {
        return -1;
      } 

      pFile = fopen(szFileDat,"rb");
      if (NULL != pFile) {
        nBytes = fread(getPtr(),sizeof(T),getSize(),pFile);
        fclose(pFile);
        return nBytes;
      }
    }

    return -1;
  } catch (...) {
    return -1;
  }


}

*/
template <class T> int CVector<T>::save(FILE *pfile)  
{
  int  nBytes = 0;

//  nBytes+=fwrite(&m_nDimension,sizeof(int),1        ,pfile);
//  nBytes+=fwrite(&m_nSize[1]  ,sizeof(int),3        ,pfile);
//  nBytes+=fwrite(getPtr()     ,sizeof(T)  ,getSize(),pfile);

  nBytes+=write_file<int>(&m_nDimension,1        ,pfile);
  nBytes+=write_file<int>(&m_nSize[1]  ,3        ,pfile);
  nBytes+=write_file<T>  (getPtr()     ,getSize(),pfile);

  return nBytes;
}


template <class T> int CVector<T>::load(FILE *pfile)  
{
  int   nBytes = 0;
  int   nDimension;
  int   nSize[4];

//  nBytes+=fread(&nDimension   ,sizeof(int),1        ,pfile);
//  nBytes+=fread(&nSize[1]     ,sizeof(int),3        ,pfile);

  nBytes+=read_file<int>(&nDimension   ,1        ,pfile);
  nBytes+=read_file<int>(&nSize[1]     ,3        ,pfile);


  if (1 == nDimension) {
    resize(nSize[1]);
  } else if (2 == nDimension) {
    resize(nSize[2],nSize[1]);
  } else if (3 == nDimension) {
    resize(nSize[3],nSize[2],nSize[1]);       
  } else {
    return -1;
  } 

//  nBytes+=fread(getPtr(),sizeof(T),getSize(),pfile);
  nBytes+=read_file<T>(getPtr(),getSize(),pfile);


  return nBytes;

}


///////////////////////////////////////////////////////////////////////////////
/// Performs a cholesky decomposition. 
/// The result is a lower triangular matrix. 
/// @param matrix_in Square matrix that is to be decomposed. 
/// @param matrix_cholesky_out Square output matrix for the decomposition.
/// @param num_dimensions Number of dimensions of the square matrixes. 
/// @return 0 if successfull else 1 
/// @warning A returnvalue of 1 means that this matrix is sigular and therefore
///          no cholesky decomposition could be found. 
///////////////////////////////////////////////////////////////////////////////
//template <class T> int CVector<T>::DecomposeCholesky(T *matrix_in, T *matrix_cholesky_out, int num_dimensions) 
template <class T> int CVector<T>::DecomposeCholesky(CVector<T> *matrix, CVector<T> *matrix_cholesky) 
{
  int           i;
  int           j;
  int           k;
  float         sum;

  if (matrix->getDimension() != 2 || matrix_cholesky->getDimension() != 2) return -1;
  if (matrix->getDimM() != matrix->getDimN() || matrix_cholesky->getDimM() != matrix_cholesky->getDimN()) return -1;
  if (matrix->getDimM() != matrix_cholesky->getDimM()) return -1;

  int num_dimensions = matrix->getDimM();

  //CVector<T> matrix         (matrix_in,          num_dimensions,num_dimensions);
  //CVector<T> matrix_cholesky(matrix_cholesky_out,num_dimensions,num_dimensions);

  // initialize output matrix with zeros
  for (i=0; i<num_dimensions*num_dimensions; i++) (*matrix_cholesky)[i] = 0;
  
  // perform cholesky decomposition according to numerical recepies algorithm 
  for (i=0; i<num_dimensions; i++) {
    for (j=i; j<num_dimensions; j++) { 
      sum = (*matrix)(j,i);

      for (k=i-1; k>=0; k--) sum -= (*matrix_cholesky)(k,i) * (*matrix_cholesky)(k,j); 
      if (i==j) {
        if (sum <=0) return(1); // matrix is singular 
        (*matrix_cholesky)(i,i) = (T)sqrt(sum);
      } else {
        (*matrix_cholesky)(i,j) = sum/(*matrix_cholesky)(i,i);
      }
    }
  }

  
  return 0; // for sucess
}


#endif 
