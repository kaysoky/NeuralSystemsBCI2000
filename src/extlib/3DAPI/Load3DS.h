////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: shzeng, schalk@wadsworth.org
// Description: This class handles all of the loading code
//   to load .3ds file
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef C_LOAD_3DS_H
#define C_LOAD_3DS_H

#include "component.h"

//------- The maximum amount of textures to load --------//
#define MAX_TEXTURES 100
extern UINT g_Texture[MAX_TEXTURES];

void Load3dsFile(t3DModel&, const std::string& fileName);
void ClearModel(t3DModel&);


/********************PRIMARY LOAD CODE FOR LOADING 3DS FILE********************/
// This class handles all of the loading code
class CLoad3DS
{
public:
       //    CLoad3DS();                     // This inits the data members

    // This is the function that you call to load the 3DS
    bool Import3DS(t3DModel& pModel,const char *strFileName);

private:
    // This reads in a string and saves it in the char array passed in
    int GetString(char *readin);

    // This reads the next chunk
    void ReadChunk(tChunk& myChunk);

    // This reads the next large chunk
    void ProcessNextChunk(t3DModel& pModel, tChunk&);

    // This reads the object chunks
    void ProcessNextObjectChunk(t3DModel& pModel, t3DObject& pObject, tChunk&);

    // This reads the material chunks
    void ProcessNextMaterialChunk(t3DModel& pModel, tChunk&);

    // This reads the RGB value for the object's color
    void ReadColorChunk(tMaterialInfo& pMaterial, tChunk& pChunk);

    // This reads the objects vertices
    void ReadVertices(t3DObject& pObject, tChunk&);

    // This reads the objects face information
    void ReadVertexIndices(t3DObject& pObject, tChunk&);

    // This reads the texture coodinates of the object
    void ReadUVCoordinates(t3DObject& pObject, tChunk&);

    // This reads in the material name assigned to the object and sets the materialID
    void ReadObjectMaterial(t3DModel& pModel, t3DObject& pObject, tChunk& pPreviousChunk);

    // This computes the vertex normals for the object (used for lighting)
    void ComputeNormals(t3DModel& pModel);

    // This frees memory and closes the file
    void CleanUp();

    // The file pointer
    std::FILE *m_FilePointer;
};


#endif // C_LOAD_3DS_H