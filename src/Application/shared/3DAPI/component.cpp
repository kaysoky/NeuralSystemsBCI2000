/*This is the object classes that store all the data structure needed to load .3ds file*/
#include "header.h"

/*************************CHUNK OF THE 3DS FILE********************************/
// This holds the chunk info
struct tChunk
{
	unsigned short int ID;					// The chunk's ID		
	unsigned int length;					// The length of the chunk
	unsigned int bytesRead;					// The amount of bytes read within that chunk
};


/********************************3D POINT CLASS********************************/
// This is our 3D point class.  This will be used to store the vertices of our model.
class CVector3 
{
public:
	float x, y, z;
};




/********************************2D POINT CLASS********************************/
// This is our 2D point class.  This will be used to store the UV coordinates.
class CVector2 
{
public:
	float x, y;
};



/*******************************FACE STRUCTURE*********************************/
// This is our face structure.  This is is used for indexing into the vertex 
// and texture coordinate arrays.  From this information we know which vertices
// from our vertex array go to which face, along with the correct texture coordinates.
struct tFace
{
	int vertIndex[3];			// indicies for the verts that make up this triangle
	int coordIndex[3];			// indicies for the tex coords to texture this face
};

/***************************MATERIALINFO***************************************/
// This holds the information for a material.  It may be a texture map of a color.
// Some of these are not used, but I left them because you will want to eventually
// read in the UV tile ratio and the UV tile offset for some models.
struct tMaterialInfo
{
	char  strName[255];			// The texture name
	char  strFile[255];			// The texture file name (If this is set it's a texture map)
	BYTE  color[3];				// The color of the object (R, G, B)
	int   texureId;				// the texture ID
	float uTile;				// u tiling of texture  (Currently not used)
	float vTile;				// v tiling of texture	(Currently not used)
	float uOffset;			        // u offset of texture	(Currently not used)
	float vOffset;				// v offset of texture	(Currently not used)
} ;


/*******************************OBJECT3D***************************************/
// This holds all the information for our model/scene.
// You should eventually turn into a robust class that 
// has loading/drawing/querying functions like:
// LoadModel(...); DrawObject(...); DrawModel(...); DestroyModel(...);
struct t3DObject 
{
	int  numOfVerts;			// The number of verts in the model
	int  numOfFaces;			// The number of faces in the model
	int  numTexVertex;			// The number of texture coordinates
	int  materialID;			// The texture ID to use, which is the index into our texture array
	bool bHasTexture;			// This is TRUE if there is a texture map for this object
	char strName[255];			// The name of the object
	CVector3  *pVerts;			// The object's vertices
	CVector3  *pNormals;		        // The object's normals
	CVector2  *pTexVerts;		        // The texture's UV coordinates
	tFace *pFaces;				// The faces information of the object
};

/***********************************MODEL3D************************************/
// This holds our model information.  This should also turn into a robust class.
// We use STL's (Standard Template Library) vector class to ease our link list burdens. :)
struct t3DModel 
{
	int numOfObjects;					// The number of objects in the model
	int numOfMaterials;					// The number of materials for the model
	vector<tMaterialInfo> pMaterials;	// The list of material information (Textures and colors)
	vector<t3DObject> pObject;			// The object list for our model
};


/********************PRIMARY LOAD CODE FOR LOADING 3DS FILE********************/
// This class handles all of the loading code
class CLoad3DS
{
public:
       //	CLoad3DS();								// This inits the data members

	// This is the function that you call to load the 3DS
	bool Import3DS(t3DModel *pModel,const char *strFileName);

private:
	// This reads in a string and saves it in the char array passed in
	int GetString(char *readin);

	// This reads the next chunk
	void ReadChunk(tChunk *myChunk);

	// This reads the next large chunk
	void ProcessNextChunk(t3DModel *pModel, tChunk *);

	// This reads the object chunks
	void ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *);

	// This reads the material chunks
	void ProcessNextMaterialChunk(t3DModel *pModel, tChunk *);

	// This reads the RGB value for the object's color
	void ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk);

	// This reads the objects vertices
	void ReadVertices(t3DObject *pObject, tChunk *);

	// This reads the objects face information
	void ReadVertexIndices(t3DObject *pObject, tChunk *);

	// This reads the texture coodinates of the object
	void ReadUVCoordinates(t3DObject *pObject, tChunk *);

	// This reads in the material name assigned to the object and sets the materialID
	void ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk);
	
	// This computes the vertex normals for the object (used for lighting)
	void ComputeNormals(t3DModel *pModel);

	// This frees memory and closes the file
	void CleanUp();
	
	// The file pointer
	FILE *m_FilePointer;
};


/*

*/
class model3Dclass
{
public:
        int elementID;
        int primitiveID;
        bool isOn;
        GLfloat X;
        GLfloat Y;
        GLfloat Z;

        GLfloat NewX;
        GLfloat NewY;
        GLfloat NewZ;
        //Self rotatoin angel of each axis
        GLfloat xaxisAngle;
        GLfloat yaxisAngle;
        GLfloat zaxisAngle;


        //Rotation with respect to reference point
        GLfloat rotPointAngleXY;  //rotational angle of XY plane
        GLfloat rotPointAngleYZ;  //rotational angle of YZ plane

        GLfloat rotPointX;      //rotational reference point coordinate
        GLfloat rotPointY;
        GLfloat rotPointZ;

        //3ds file name
        string filename3ds;

        t3DModel mod;
        
        model3Dclass()
        {
                filename3ds = "";
                elementID = 0;
                primitiveID = 9;
                isOn = true;
                X = 0;
                Y = 0;
                Z = 0;
                rotPointAngleXY = 0;
                rotPointAngleYZ = 0;
                rotPointX = 0;
                rotPointY = 0;
                rotPointZ = 0;
        }

};
