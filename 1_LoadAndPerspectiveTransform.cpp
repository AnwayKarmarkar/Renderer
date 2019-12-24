/*
 * proj1.cpp
 *
 *  Created on: Jan 25, 2019
 *      Author: anway
 */
#include <GL/freeglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <math.h>
#include <iostream>
#define n 3
using namespace std;


/*
 *
 *
 *
 *
 *
 * Use W-S to change the Y value of the camera
 * Use A-D to change the X value of the camera
 * Use the Left and Right mouse buttons to change the Z value of the camera
 *
 *
 *
 *
 */

//Global Variables
int numOfPoly,numOfPoints,maxPolygon = 0;
float **pointArray,**polygonArray;

//File name to load the model
//Adjust the filename and filepath accordingly;
string filename = "hackedcube";
string file = "/home/anway/CG_Data/CG2/D files/dfiles/"+filename+".d.txt";

//Change to false to switch off BackFace Culling
bool backFaceToggle = true;

//C vector
float C[3]= {1,1,-3};

class LoadModel
{
	//Fill an empty matrix with values
	void InitArray(float Arr[],int colCount,int val)
	{
		for(int i =0;i<colCount;i++)
		{
			Arr[i]=val;
		}
	}

	//To read the first line of the .d file and get the number of Vertices(Points) and Polygons
	void GetPointPolyNumber(string line)
	{
		string word;
		string words[20];
		char x;
		bool check = false;
		int length = line.size();
		int i=0,count = 0;
		while(i<length)
		{
			x=line[i];

			if(x=='\t')
			{
				x=' ';
			}
			if(x != ' ')
			{
				word+=x;
				if(check == false)
				{
					check = true;
				}
			}
			else
			{
				if(check != false)
				{
					check = false;
					words[count]=word;
					word="";
					count++;
				}
			}
			if(i==length-1)
			{
				words[count]=word;
			}
			i++;
		}
		numOfPoints = atoi(words[1].c_str());
		numOfPoly = atoi(words[2].c_str());
	}

	//Get a word from a string
	string GetCharacter(string Arr)
	{
		char x;
		string result;
		bool check = false;
		for(int i=0;;i++)
		{
			x=Arr[i];
			if(check==true && x==' ')
			{
				return result;
			}
			if(x!=' ')
			{
				check = true;
				result+=x;
			}
		}
		return "0";
	}

	//Read the rest of the file and convert it from char to float
	void LineToWords(string Arr[],float floatArray[] ,string line)
	{
		string word;
		char x;
		bool check = false;
		int length = line.size();
		int i=0,count = 0;
		while(i<length)
		{
			x=line[i];
			if(x=='\t')
			{
				x=' ';
			}
			if(x != ' ')
			{
				word+=x;
				if(check == false)
				{
					check = true;
				}
			}
			else
			{
				if(check != false)
				{
					check = false;
					Arr[count]=word;
					word="";
					count++;
				}
			}
			if(i==length-1)
			{
				Arr[count]=word;
			}
			i++;
		}
		for(i=0;i<=count;i++)
		{
			floatArray[i] = atof(Arr[i].c_str());
		}
	}

	//To find the largest polygon in the model
	int FindNumOfPoints(float Arr[])
	{
		int result=0;
		for(int i=0;i<numOfPoly;i++)
		{
			if(Arr[i]>result)
			{
				result=Arr[i];
			}
		}
		return result;
	}


public:
	void Load()
	{
		//Open and read the file
		char filepath[file.length()+1];
		strcpy(filepath,file.c_str());
		bool checkIfFirstLine = true;
			//Open the .d file
		ifstream dFile;
		dFile.open(filepath);
		string line;
		if(dFile.is_open())
		{
			cout<<"\nLOG : File Opened Successfully";
			cout<<"\nLOG : Reading contents";
			while(getline(dFile,line))
			{
				if(checkIfFirstLine == true)
				{
					GetPointPolyNumber(line);
					checkIfFirstLine = false;
					cout<<"\nLOG : Points : "<<numOfPoints<<" | Polygons : "<<numOfPoly;
					break;
				}
			}
			if(numOfPoints == 0 || numOfPoly == 0)
			{
				cout<<"\nERROR LOG : No Points or Polygons found";
			}

			//Reading the points from file
			string pointStringArray[numOfPoints][3];
			float pointFloatArray[numOfPoints][3];
			for(int i=0;i<numOfPoints;i++)
			{
				InitArray(pointFloatArray[i],3,0);
			}
			for(int i=0; i<numOfPoints; i++)
			{
				getline(dFile,line);
				LineToWords(pointStringArray[i], pointFloatArray[i], line);
			}

			//Reading the polygons from the file
			string testStringArray[numOfPoly];
			float testFloatArray[numOfPoly];
			string x;

			for(int i=0;i<numOfPoly;i++)
			{
				getline(dFile,line);
				testStringArray[i]=line;
				x=GetCharacter(testStringArray[i]);
				testFloatArray[i]=atoi(x.c_str());
			}



			maxPolygon=FindNumOfPoints(testFloatArray)+1;
			cout<<"\n Max Polygon : "<<maxPolygon;
			string polygonStringArray[numOfPoly][maxPolygon];
			float polygonFloatArray[numOfPoly][maxPolygon];
			for(int i=0;i<numOfPoly;i++)
			{
				InitArray(polygonFloatArray[i],maxPolygon,0);
			}
			for(int i=0;i<numOfPoly;i++)
			{
				LineToWords(polygonStringArray[i], polygonFloatArray[i], testStringArray[i]);
			}

			pointArray = new float *[numOfPoints];
			for(int i=0;i<numOfPoints; i++)
			{
				pointArray[i]=new float[4];
				InitArray(pointArray[i],4,1);
			}
			polygonArray = new float *[numOfPoly];
			for(int i=0;i<numOfPoly; i++)
			{
				polygonArray[i]=new float[maxPolygon];
				InitArray(polygonArray[i],maxPolygon,0);
			}

				//Putting the point data into a global array
			for(int i = 0; i<numOfPoints; i++)
			{
				for(int j=0; j<3; j++)
				{
					pointArray[i][j]=pointFloatArray[i][j];
				}
			}

			//Putting the polygon data into a global array
			for(int i = 0; i<numOfPoly; i++)
			{
				for(int j=0; j<maxPolygon; j++)
				{
					polygonArray[i][j]=polygonFloatArray[i][j];
				}
			}



			dFile.close();
		}
		else
		{
			cout<<"\nERROR LOG : Cannot open the file";
			exit(0);
		}
		cout<<"\nLOG : Arrays Created";
	}
};
class ArrayFunctions
{
public:
	//Destroy the model arrays
	void DestroyArrays()
	{
		for(int i=0;i<numOfPoints;i++)
		{
			delete[] pointArray[i];
		}
		delete[] pointArray;


		for(int i=0;i<numOfPoly;i++)
		{
			delete[] polygonArray[i];
		}
		delete[] polygonArray;
		cout<<"\nLOG : Arrays Destroyed";
	}

	//Display the model Arrays
	void DisplayArrays()
	{
		cout<<"\nLOG : Point Array \n";
		for(int i = 0; i<numOfPoints; i++)
		{
			cout<<"\n";
			for(int j=0; j<4; j++)
			{
				cout<<"\t"<<pointArray[i][j];
			}
		}
		cout<<"\nLOG : Polygon Array \n";
		for(int i = 0; i<numOfPoly; i++)
		{
			cout<<"\n";
			for(int j=0; j<maxPolygon; j++)
			{
				cout<<"\t"<<polygonArray[i][j];
			}
		}
	}
};
class VectorOperations
{
	//Vector operations
public:
	float DotProduct(float vect_A[], float vect_B[])
	{

	    float product = 0;

	    // Loop for calculate dot product
	    for (int i = 0; i < n; i++)

	        product = product + vect_A[i] * vect_B[i];
	    return product;
	}

	void CrossProduct(float vect_A[], float vect_B[], float cross_P[])
	{

	    cross_P[0] = vect_A[1] * vect_B[2] - vect_A[2] * vect_B[1];
	    cross_P[1] = vect_A[2] * vect_B[0] - vect_A[0] * vect_B[2];
	    cross_P[2] = vect_A[0] * vect_B[1] - vect_A[1] * vect_B[0];
	}



	float Normalize(float vect_A[] )
	{
		float sq1 = vect_A[0]*vect_A[0];
		float sq2 = vect_A[1]*vect_A[1];
		float sq3 = vect_A[2]*vect_A[2];
		return(sqrt(sq1+sq2+sq3));
	}
};
class TransformToPerspective
{
	VectorOperations vecOps;
	//Vectors Pref and V'
	float Pref[3]={0,0,0};
	float _V[3]={0,1,0};

	//Vectors U,N,V
	float U[3], N[3], V[3];

	//near and far plane and h
	float d=2,f=100,h=1;

	//Mview and Mpers Matrices
	float Mview[4][4],Mpers[4][4];

	//Function to multiply point array with a 4*4 matrix
	void PointArrayMatMul(float A[][4],float B[])
	{
		float C[4];
		for(int i=0;i<4;i++)
		{
			C[i]=0;
		}

		for(int i=0;i<4;i++)
		{
			for(int j=0;j<4;j++)
			{
				C[i]+=A[i][j]*B[j];
			}
		}
		for(int i=0;i<4;i++)
		{
			B[i]=C[i];
		}
	}

	//fill a matrix with 0
	void InitMatrix(float A[][4])
	{
		int i,j;
		for(i=0;i<4;i++)
		{
			for(j=0;j<4;j++)
			{
				A[i][j]=0;
			}
		}
	}

	//Display Matrix
	void DispMatrix(float A[][4])
	{
		int i,j;
		for(i=0;i<4;i++)
		{
			cout<<"\n";
			for(j=0;j<4;j++)
			{
				cout<<"  "<<A[i][j];
			}
		}
	}

	//Calculate U,V and N vectors from C,V' and Pref vectors
	void CreateUVN()
	{
		float temp[3],div;

		//Vector N
		for(int i=0;i<3;i++)
		{
			temp[i]=Pref[i]-C[i];
		}
		div=vecOps.Normalize(temp);
		N[0]=temp[0]/div;
		N[1]=temp[1]/div;
		N[2]=temp[2]/div;
		//cout<<"\n N : "<<N[0]<<N[1]<<N[2];


		//Vector U
		vecOps.CrossProduct(N,_V,temp);
		div=vecOps.Normalize(temp);
		if(div==0)
		{
			div=1;
		}
		U[0]=temp[0]/div;
		U[1]=temp[1]/div;
		U[2]=temp[2]/div;
		//cout<<"\n U : "<<U[0]<<U[1]<<U[2];

		//Vector V
		vecOps.CrossProduct(U,N,V);
		//cout<<"\n V : "<<V[0]<<V[1]<<V[2];
	}
	//Function to create the Mview Matrix and multiply the model coordinates with it
	void MviewFunction()
	{
		float A[4][4],B[4][4];
		InitMatrix(A);
		InitMatrix(B);
		InitMatrix(Mview);
		//Rotation Matrix
		A[0][0]=U[0];	A[0][1]=U[1];	A[0][2]=U[2];
		A[1][0]=V[0];	A[1][1]=V[1];	A[1][2]=V[2];
		A[2][0]=N[0];	A[2][1]=N[1];	A[2][2]=N[2];
		A[3][3]=1;
		//Translation Matrix
		B[0][0]=1;	B[1][1]=1;	B[2][2]=1;	B[3][3]=1;
		B[0][3]=-C[0];	B[1][3]=-C[1];	B[2][3]=-C[2];

			//Multiplying Rotation and Translation matrices to create the Mview matrix
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<4;j++)
			{
				for(int k=0;k<4;k++)
				{
					Mview[i][j]+=	A[i][k] * B[k][j];
				}
			}
		}
			//Multiplying the model coordinates by Mview
		for(int i=0;i<numOfPoints;i++)
		{
			PointArrayMatMul(Mview,pointArray[i]);
		}
	}

		//Function to create the Mpers Matrix and multiply the model coordinates with it
		void MpersFunction()
		{
			//Mpers Matrix created
			InitMatrix(Mpers);
			Mpers[0][0]=d/h;	Mpers[1][1]=d/h;	Mpers[2][2]=f/(f-d);
			Mpers[2][3]=(-d*f)/(f-d); Mpers[3][2]=1;

			//Multiply the View Coordinates with Mpers
			for(int i=0;i<numOfPoints;i++)
			{
				PointArrayMatMul(Mpers,pointArray[i]);
			}

			//X,Y,Z values in pointArray are divided by

			for(int i=0;i<numOfPoints;i++)
			{
				for(int j=0;j<3;j++)
				{
					pointArray[i][j]=pointArray[i][j]/pointArray[i][3];
				}
			}
		}
		//Backface Culling Function
		void RemoveBackFaces()
		{
			float tempPointArray[numOfPoints][3];
			//using tempPointArray for calculating the backfaces
			VectorOperations vecOps;
			for(int i=0;i<numOfPoints;i++)
			{
				for(int j=0;j<3;j++)
				{
					tempPointArray[i][j]=pointArray[i][j];
				}
			}

			float normalArray[numOfPoly][3];
			float v1[3],v2[3],temp;
			int a,b,c;

			for(int i=0;i<numOfPoly;i++)
			{
				//Check to make sure that polygon has atleast 3 sides
				if(polygonArray[i][0]>2)
				{
					//a,b,c are the first 3 vertices in any polygon
					a=polygonArray[i][1];
					b=polygonArray[i][2];
					c=polygonArray[i][3];

					//V1 is the vector from a to b
					v1[0]=tempPointArray[b-1][0]-tempPointArray[a-1][0];
					v1[1]=tempPointArray[b-1][1]-tempPointArray[a-1][1];
					v1[2]=tempPointArray[b-1][2]-tempPointArray[a-1][2];

					//V2 is the vector from b to c
					v2[0]=tempPointArray[c-1][0]-tempPointArray[b-1][0];
					v2[1]=tempPointArray[c-1][1]-tempPointArray[b-1][1];
					v2[2]=tempPointArray[c-1][2]-tempPointArray[b-1][2];

					//cross product of the two vectors v1 and v2
					vecOps.CrossProduct(v1,v2,normalArray[i]);

					//Normalized cross product
					temp=vecOps.Normalize(normalArray[i]);

					//Final array of the surface normal vectors
					normalArray[i][0]=normalArray[i][0]/temp;
					normalArray[i][1]=normalArray[i][1]/temp;
					normalArray[i][2]=normalArray[i][2]/temp;

					//If Zs of the normal vector > 0,then the polygon is marked as backfacing.
					if(normalArray[i][2]>0)
					{
						if(backFaceToggle==true)
						{
							//number of sides of backfacing polygon are made 0.
							polygonArray[i][0]=0;
						}
					}
				}
			}
		}
public:
	//Caller function for all the Transformation functions
	void TransformToPerspectiveCallerFunction()
	{
		CreateUVN();
		MviewFunction();
		MpersFunction();
		RemoveBackFaces();
	}
};


void DrawShape()
{
	//Copy the Point array into a temporary float array for display
	float tempPointArray[numOfPoints][3];
	for(int i=0;i<numOfPoints;i++)
	{
		for(int j=0;j<3;j++)
		{
			tempPointArray[i][j]=pointArray[i][j];
		}
	}

	// clear buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	//Set the line color
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );


	int start,end,numOfSides, returnPoint;
	glBegin(GL_LINES);
	for(int i=0;i<numOfPoly;i++)
	{
		//cout<<"\nLOG : Polygon "<<i+1<<" rendering : ";

		//returnPoint is a reference to complete the polygon by saving the first vertex
		returnPoint = polygonArray[i][1];

		//Number of sides in each polygon
		numOfSides = polygonArray[i][0];
		for(int j=1;j<numOfSides+1;j++)
		{
			//The start point for each edge in the polygon
			start = polygonArray[i][j];

			//To find the end point of the edge
			if(j==numOfSides)
			{
				//For the last edge in the polygon
				end=returnPoint;
			}
			else
			{
				//For other edges in the polygon
				end = polygonArray[i][j+1];
			}
			if(start != 0)
			{
				//cout<<"\nLOG :\t Point "<<start<<" to "<<end;
				glVertex3fv(tempPointArray[start-1]);
				glVertex3fv(tempPointArray[end-1]);
			}
		}
		// flush

	}
	glEnd();
	glFlush();
	// swap buffers
	glutSwapBuffers();
}

void render( void )
{
	DrawShape();
}
//Function to reload and redisplay the model after changing C vector
void AdjustCamera()
{
	TransformToPerspective tp;
	LoadModel lm;
	ArrayFunctions af;
	af.DestroyArrays();
	lm.Load();
	tp.TransformToPerspectiveCallerFunction();
	glutPostRedisplay();
}

//Mouse Function
void OnMouseClick(int button, int state, int mx, int my)
{
    if(button==GLUT_LEFT_BUTTON)
    {
    	C[2]+=0.5;
    }
    else if(button==GLUT_RIGHT_BUTTON)
    {
    	C[2]-=0.5;
    }
    AdjustCamera();
}

//Keyboard Function
void OnKeyPress(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'w':
		C[1]+=0.5;
		break;
	case 's':
		C[1]-=0.5;
		break;
	case 'a':
		C[0]+=0.5;
		break;
	case 'd':
		C[0]-=0.5;
		break;
	default:
		cout<<"\nLOG: Unrecognized Key. Check Control scheme";
	}
	AdjustCamera();
}

int main(int argc, char** argv)
{

	LoadModel loadModel;
	ArrayFunctions arrayFunctions;
	TransformToPerspective transformToPerspective;

	//Load the model
	loadModel.Load();

	//Mview and Mpers matrices created and Multiplied to the coordinates
	//Backface culling also applied
	transformToPerspective.TransformToPerspectiveCallerFunction();

	//Display the Point and Polygon arrays

	//OpenGL Section
	glutInit( &argc, argv );

	// set windows size
	glutInitWindowSize( 1024, 1024 );

	// set OpenGL display mode
	glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );

	// set window title
	glutCreateWindow( "Lab_1" );
	//Action taken after exiting main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_CONTINUE_EXECUTION);
	// set rendering function
	glutDisplayFunc( render );
	glutMouseFunc(OnMouseClick);
	glutKeyboardFunc(OnKeyPress);
	glutMainLoop();

	//Function to destroy the point and polygon arrays
	arrayFunctions.DisplayArrays();
	arrayFunctions.DestroyArrays();

	return 0;
}

