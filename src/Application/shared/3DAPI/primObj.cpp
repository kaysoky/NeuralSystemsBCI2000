/*
This "primObj" class resembls the objects in th 3D environment. There are two
primitive objects:
1.      Sphere
2.      Cuboids
The above index number are identical with these objects' "primID"
*/

//---------------------------------------------------------------------------
#include "header.h"

//---------------------------------------------------------------------------
class primObj
{
private:
        int elementID;          //The ID of the object
        int primID;             //sphere = 1, cuboid = 2, boundary (a very large triangle) = 3
                                //3D Text = 4
        int brightness;         //0~255
        int tranparency;        //0~255

        GLfloat RColor;         //0.0f~1.0f
        GLfloat BColor;
        GLfloat GColor;

        string  texture;        //texture file
        GLfloat rotAngleX;       //rotational angle with repect to x axis
        GLfloat rotAngleY;       //rotational angle                y axis
        GLfloat rotAngleZ;       //rotational angle                z axis

        int     rotAxis;        //rotational axis, Xaxis=1, Yaxis=2, Zaxis=3

        bool    isOn;
        bool    hasTexture;

        //Rotation with respect to reference point
        GLfloat rotPointAngleXY;  //rotational angle of XY plane
        GLfloat rotPointAngleYZ;  //rotational angle of YZ plane

        GLfloat rotPointX;      //rotational reference point coordinate
        GLfloat rotPointY;
        GLfloat rotPointZ;
   //     int     rotPointAxis;   //rotational axie at reference point
                                //Xaxis=1, Yaxis=2, Zaxis=3

        GLfloat coordAfterCameraX;
        GLfloat coordAfterCameraY;
        GLfloat coordAfterCameraZ;

        GLfloat coordNewX;
        GLfloat coordNewY;
        GLfloat coordNewZ;
public:
        GLuint textureData;     //texture data

/***************************CONSTRUCTOR****************************************/
        primObj()                               {setAll();}
        void setAll();
/****************************General Properties********************************/
        void setcoordAfterCameraX(GLfloat x)    {coordAfterCameraX = x;}
        void setcoordAfterCameraY(GLfloat y)    {coordAfterCameraY = y;}
        void setcoordAfterCameraZ(GLfloat z)    {coordAfterCameraZ = z;}
        void setcoordNewX(GLfloat x)            {coordNewX = x;}
        void setcoordNewY(GLfloat y)            {coordNewY = y;}
        void setcoordNewZ(GLfloat z)            {coordNewZ = z;}
        GLfloat getcoordAfterCameraX()          {return coordAfterCameraX;}
        GLfloat getcoordAfterCameraY()          {return coordAfterCameraY;}
        GLfloat getcoordAfterCameraZ()          {return coordAfterCameraZ;}
        GLfloat getcoordNewX()                  {return coordNewX;}
        GLfloat getcoordNewY()                  {return coordNewY;}
        GLfloat getcoordNewZ()                  {return coordNewZ;}


        void setStatus(bool status)             {isOn = status;}
        bool getStatus()                        {return isOn; }

        void setHasTexture (bool status)        {hasTexture = status;}
        bool getHasTexture()                    {return hasTexture;}

        void setElementID(int ID)               {elementID = ID;}
        int getElementID()                      {return elementID;}

        void setPrimitveID(int ID)              {primID = ID;}
        int getPrimitiveID()                    {return primID;}

        void setBrightness(int bri)             {brightness = bri;}
        int getBrightness()                     {return brightness;}

        void setTransparency(int tra)           {tranparency = tra;}
        int getTransparency()                   {return tranparency;}

        void setColor(GLfloat R, GLfloat G, GLfloat B)
        {
                RColor = R;
                GColor = G;
                BColor = B;
        }
        GLfloat getColorR()                     {return RColor;}
        GLfloat getColorG()                     {return GColor;}
        GLfloat getColorB()                     {return BColor;}

        void setTexture(string tex)             {texture = tex;        }
        string getTexture()                     {return texture;}

        void setRotAngX(GLfloat angle)           {rotAngleX = angle;}
        void setRotAngY(GLfloat angle)           {rotAngleY = angle;}
        void setRotAngZ(GLfloat angle)           {rotAngleZ = angle;}
        GLfloat getRotAngX()                     {return rotAngleX;}
        GLfloat getRotAngY()                     {return rotAngleY;}
        GLfloat getRotAngZ()                     {return rotAngleZ;}


        void setRotPointAngleXY(GLfloat ang)      {rotPointAngleXY = ang;}
        void setRotPointAngleYZ(GLfloat ang)      {rotPointAngleYZ = ang;}
        GLfloat getRotPointAngleXY()              {return rotPointAngleXY;}
        GLfloat getRotPointAngleYZ()              {return rotPointAngleYZ;}
        void setRotPointPosition(GLfloat X, GLfloat Y, GLfloat Z)
        {
                rotPointX = X;
                rotPointY = Y;
                rotPointZ = Z;
        }

        GLfloat getRPX()                        {return rotPointX;}
	GLfloat getRPY()                        {return rotPointY;}
	GLfloat getRPZ()                        {return rotPointZ;}

   //     void setRPAxis(int axis)                {rotPointAxis = axis;}
   //     int getRPAxis()                         {return rotPointAxis;}

        void setTextureData(GLuint data)        {textureData = data;}
        GLuint getTextureData()                 {return textureData;}



/****************************Plane Properties********************************/



};//primObj

void primObj::setAll()
{
        elementID = 0;
        primID = 0;
        brightness = 0;
        tranparency = 0;
        textureData = NULL;
        RColor = 0.0f;
        BColor = 0.0f;
        GColor = 0.0f;
        texture = "";
        rotAngleX = 0.0f;
        rotAngleY = 0.0f;
        rotAngleZ = 0.0f;
        rotAxis = 0;
        isOn = true;



        rotPointAngleXY = 0.0f;  //rotational angle with respect to reference point
        rotPointAngleYZ = 0.0f;  //rotational angle with respect to reference point

        rotPointX = 0.0f;      //rotational reference point coordinate
        rotPointY = 0.0f;
        rotPointZ = 0.0f;
                                //Xaxis=1, Yaxis=2, Zaxis=3

}//setALL


/*
*       sphere class
*       Extern from the primObj class
*       It holds the unique properties of 3D sphere
*/

//---------------------------------------------------------------------------



class sphere: public primObj
{
private:
        GLfloat sphereX;        //position coordinate
        GLfloat sphereY;
        GLfloat sphereZ;
        GLfloat sphereRad;      //radius of the sphere

public:
/*******************************CONSTRUCTOR************************************/
        sphere()                                        {sphereSetAll();}
        void sphereSetAll();

        void setSphere(GLfloat x, GLfloat y, GLfloat z, GLfloat rad)
        {
                sphereX = x;
                sphereY = y;
                sphereZ = z;
                sphereRad = rad;
        }
        GLfloat getSphereX()                            {return sphereX;}
        GLfloat getSphereY()                            {return sphereY;}
        GLfloat getSphereZ()                            {return sphereZ;}
        GLfloat getSphereRad()                          {return sphereRad;}

};//sphere

void sphere::sphereSetAll()
{
        sphereX = 0.0f;        //position coordinate
        sphereY = 0.0f;
        sphereZ = 0.0f;
        sphereRad = 0.0f;      //radius of the sphere
        setBrightness(255);
}//setAll



/*
*       cuboids class
*       Extern from the primObj class
*       It holds the unique properties of 3D sphere
*/

//---------------------------------------------------------------------------


class cuboids : public primObj
{
private:
        GLfloat cuboidX;
        GLfloat cuboidY;
        GLfloat cuboidZ;
        GLfloat cuboidW;        //width
        GLfloat cuboidH;        //height
        GLfloat cuboidD;        //depth


        GLfloat LBFX, LBFY, LBFZ;       //Left, Bottom, Front
        GLfloat LTFX, LTFY, LTFZ;       //Left, Top, Front
        GLfloat LTBX, LTBY, LTBZ;       //Left, Top, Back
        GLfloat LBBX, LBBY, LBBZ;       //Left, Bottom, Back

        GLfloat RBFX, RBFY, RBFZ;       //Right, Bottom, Front
        GLfloat RTFX, RTFY, RTFZ;       //Right, Top, Front
        GLfloat RTBX, RTBY, RTBZ;       //Right, Top, Back
        GLfloat RBBX, RBBY, RBBZ;       //Right, Bottom, Back

public:
/*******************************CONSTRUCTOR************************************/
        cuboids()                                        {cuboidsSetAll();}
        void cuboidsSetAll();
        void setVertex(GLfloat x, GLfloat y, GLfloat z);
/**************************Modifiers & Accessors*******************************/
        void setCuboid(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h,
        GLfloat d)
        {
                cuboidX = x;
                cuboidY = y;
                cuboidZ = z;
                cuboidH = h;
                cuboidW = w;
                cuboidD = d;
        }
        GLfloat getCuboidX()                    {return cuboidX;}
        GLfloat getCuboidY()                    {return cuboidY;}
        GLfloat getCuboidZ()                    {return cuboidZ;}
        GLfloat getCuboidW()                    {return cuboidW;}
        GLfloat getCuboidH()                    {return cuboidH;}
        GLfloat getCuboidD()                    {return cuboidD;}
        //left, bottom, front
        GLfloat getLBFX()                       {return LBFX;}
        GLfloat getLBFY()                       {return LBFY;}
        GLfloat getLBFZ()                       {return LBFZ;}
        void setLBFX(GLfloat input)             {LBFX = input;}
        void setLBFY(GLfloat input)             {LBFY = input;}
        void setLBFZ(GLfloat input)             {LBFZ = input;}
        //left, top, front
        GLfloat getLTFX()                       {return LTFX;}
        GLfloat getLTFY()                       {return LTFY;}
        GLfloat getLTFZ()                       {return LTFZ;}
        void setLTFX(GLfloat input)             {LTFX = input;}
        void setLTFY(GLfloat input)             {LTFY = input;}
        void setLTFZ(GLfloat input)             {LTFZ = input;}
        //left, top, back
        GLfloat getLTBX()                       {return LTBX;}
        GLfloat getLTBY()                       {return LTBY;}
        GLfloat getLTBZ()                       {return LTBZ;}
        void setLTBX(GLfloat input)             {LTBX = input;}
        void setLTBY(GLfloat input)             {LTBY = input;}
        void setLTBZ(GLfloat input)             {LTBZ = input;}
        //left, bottom, back
        GLfloat getLBBX()                       {return LBBX;}
        GLfloat getLBBY()                       {return LBBY;}
        GLfloat getLBBZ()                       {return LBBZ;}
        void setLBBX(GLfloat input)             {LBBX = input;}
        void setLBBY(GLfloat input)             {LBBY = input;}
        void setLBBZ(GLfloat input)             {LBBZ = input;}
        //right, bottom, front
        GLfloat getRBFX()                       {return RBFX;}
        GLfloat getRBFY()                       {return RBFY;}
        GLfloat getRBFZ()                       {return RBFZ;}
        void setRBFX(GLfloat input)             {RBFX = input;}
        void setRBFY(GLfloat input)             {RBFY = input;}
        void setRBFZ(GLfloat input)             {RBFZ = input;}
        //right, top, front
        GLfloat getRTFX()                       {return RTFX;}
        GLfloat getRTFY()                       {return RTFY;}
        GLfloat getRTFZ()                       {return RTFZ;}
        void setRTFX(GLfloat input)             {RTFX = input;}
        void setRTFY(GLfloat input)             {RTFY = input;}
        void setRTFZ(GLfloat input)             {RTFZ = input;}
        //right, top, back
        GLfloat getRTBX()                       {return RTBX;}
        GLfloat getRTBY()                       {return RTBY;}
        GLfloat getRTBZ()                       {return RTBZ;}
        void setRTBX(GLfloat input)             {RTBX = input;}
        void setRTBY(GLfloat input)             {RTBY = input;}
        void setRTBZ(GLfloat input)             {RTBZ = input;}
        //right, back, back
        GLfloat getRBBX()                       {return RBBX;}
        GLfloat getRBBY()                       {return RBBY;}
        GLfloat getRBBZ()                       {return RBBZ;}
        void setRBBX(GLfloat input)             {RBBX = input;}
        void setRBBY(GLfloat input)             {RBBY = input;}
        void setRBBZ(GLfloat input)             {RBBZ = input;}


};//cuboids

void cuboids::setVertex(GLfloat midX, GLfloat midY, GLfloat midZ)
{
        //left X
        LBFX = midX - cuboidW/2;
        LTFX = midX - cuboidW/2;
        LTBX = midX - cuboidW/2;
        LBBX = midX - cuboidW/2;
        //right X
        RBFX = midX + cuboidW/2;
        RTFX = midX + cuboidW/2;
        RTBX = midX + cuboidW/2;
        RBBX = midX + cuboidW/2;
        //Top Y
        LTFY = midY + cuboidH/2;
        LTBY = midY + cuboidH/2;
        RTFY = midY + cuboidH/2;
        RTBY = midY + cuboidH/2;
        //Bottom Y
        LBFY = midY - cuboidH/2;
        LBBY = midY - cuboidH/2;
        RBFY = midY - cuboidH/2;
        RBBY = midY - cuboidH/2;
        //Front Z
        LBFZ = midZ + cuboidD/2;
        LTFZ = midZ + cuboidD/2;
        RBFZ = midZ + cuboidD/2;
        RTFZ = midZ + cuboidD/2;
        //Back Z
        LTBZ = midZ - cuboidD/2;
        LBBZ = midZ - cuboidD/2;
        RTBZ = midZ - cuboidD/2;
        RBBZ = midZ - cuboidD/2;


}


void cuboids::cuboidsSetAll()
{
        cuboidX = 0.0f;
        cuboidY = 0.0f;
        cuboidZ = 0.0f;
        cuboidW = 0.0f;        //width
        cuboidH = 0.0f;        //height
        cuboidD = 0.0f;        //depth
        setBrightness(255);

        //left X
        LBFX = cuboidX - cuboidW/2;
        LTFX = cuboidX - cuboidW/2;
        LTBX = cuboidX - cuboidW/2;
        LBBX = cuboidX - cuboidW/2;
        //right X
        RBFX = cuboidX + cuboidW/2;
        RTFX = cuboidX + cuboidW/2;
        RTBX = cuboidX + cuboidW/2;
        RBBX = cuboidX + cuboidW/2;
        //Top Y
        LTFY = cuboidY + cuboidH/2;
        LTBY = cuboidY + cuboidH/2;
        RTFY = cuboidY + cuboidH/2;
        RTBY = cuboidY + cuboidH/2;
        //Bottom Y
        LBFY = cuboidY - cuboidH/2;
        LBBY = cuboidY - cuboidH/2;
        RBFY = cuboidY - cuboidH/2;
        RBBY = cuboidY - cuboidH/2;
        //Front Z
        LBFZ = cuboidZ + cuboidD/2;
        LTFZ = cuboidZ + cuboidD/2;
        RBFZ = cuboidZ + cuboidD/2;
        RTFZ = cuboidZ + cuboidD/2;
        //Back Z
        LTBZ = cuboidZ - cuboidD/2;
        LBBZ = cuboidZ - cuboidD/2;
        RTBZ = cuboidZ - cuboidD/2;
        RBBZ = cuboidZ - cuboidD/2;


}//cuboids




/*
This "threeDText" class resembls the 3D text in th 3D environment.
*/

class threeDText : public primObj
{
private:
        GLfloat fontSize;
        string  caption;
        string  font;
        GLfloat OriX;
        GLfloat OriY;
        GLfloat OriZ;
        GLfloat DirX;
        GLfloat DirY;
        GLfloat DirZ;

        GLuint          threeDfontID;                 //storage for the 2D font
        HFONT           threeDoldFont;
        bool isOn;

public:
        GLYPHMETRICSFLOAT gmf[256];	        // Storage For Information About Font
        threeDText()                            {threeDTextsetAll();}
        void threeDTextsetAll();

        void setIsOn(bool state)                {isOn=state;}
        bool getIsOn()                          {return isOn;}
        void setFontID(GLuint ID)               {threeDfontID = ID;}
        void setOldFont(HFONT input)            {threeDoldFont = input;}
        void setFont(string f)                  {font = f;}
        void setFontSize(GLfloat size)              {fontSize = size;}
        void setCap(string cap)                 {caption = cap;}

        void setOrigin(GLfloat myOX, GLfloat myOY, GLfloat myOZ)
        {
                OriX = myOX;
                OriY = myOY;
                OriZ = myOZ;
        }
        void setDirection(GLfloat myDX, GLfloat myDY, GLfloat myDZ)
        {
                DirX = myDX;
                DirY = myDY;
                DirZ = myDZ;
        }

        GLuint getFontID()                      {return threeDfontID;}
        HFONT getOldFont()                      {return threeDoldFont;}
        string getFont()                        {return font;}
        GLfloat getFontSize()                   {return fontSize;}
        string getCaption()                     {return caption;}

        GLfloat getOriX()                       {return OriX;}
        GLfloat getOriY()                       {return OriY;}
        GLfloat getOriZ()                       {return OriZ;}
        GLfloat getDirX()                       {return DirX;}
        GLfloat getDirY()                       {return DirY;}
        GLfloat getDirZ()                       {return DirZ;}

};//threeDText

void threeDText::threeDTextsetAll()
{
        font = "";
        fontSize = 12.0f;
        caption = "";
        isOn = true;

        setBrightness(255);
        OriX= 0.0f;
        OriY= 0.0f;
        OriZ= 0.0f;
        DirX= 0.0f;
        DirY= 0.0f;
        DirZ= 0.0f;
        threeDfontID = NULL;
        threeDoldFont = NULL;
}//setAll
