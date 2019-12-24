/*
 * proj2.cpp
 *
 *  Created on: Feb 19, 2019
 *      Author: anway
 */
#include <GL/freeglut.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <math.h>
#include <iostream>
#include <vector>

//Window Height and Width
#define windowHeight 1024
#define windowWidth 1024

using namespace std;

//Filenames for both the models

string filename = "better-ball";
string file = "/home/anway/CG_Data/CG2/D files/dfiles/"+filename+".d.txt";


string filename2 = "cube";
string file2 = "/home/anway/CG_Data/CG2/D files/dfiles/"+filename2+".d.txt";

//Camera Vector
float C[3]= {0,1,-3};

//Z-Buffer and Frame Buffer for each pixel
float zBuffer[windowWidth][windowHeight];
float frameBuffer[windowWidth][windowHeight][3];

//Function to Initialize each buffer
void InitBuffers()
{
	for(int i=0;i<windowWidth;i++)
	{
		for(int j=0;j<windowHeight;j++)
		{
			zBuffer[i][j] = 99999;
		}
	}
	for(int i=0;i<windowWidth;i++)
	{
		for(int j=0;j<windowHeight;j++)
		{
			for(int k=0;k<3;k++)
			{
				frameBuffer[i][j][k] = 0.0f;
			}
		}
	}
}

class Edge
{
public:
	//For Maximum Value of Y
	int yMax;

	//X value of the minimum Y value
	float xOfYMin;

	//Z value of the minimum Y value
	float zOfYMin;

	//slope inverse of the edge
	float slope;

	//Change of Z value corresponding to change in Y value
	float ZYSlope;
};

/*Std vectors for the edges of the two models.
They are 3 dimensional because for each polygon, the Edge table consists of two dimensions.
Master Table stores edge information for each polygon*/

vector<vector<vector<Edge> > > masterTable1;
vector<vector<vector<Edge> > > masterTable2;

//Active Edge Table
vector<Edge> AET;

class Model
{
public:
	int numOfPoly,numOfPoints,maxPolygon = 0;
	float **pointArray,**polygonArray;
	int **pointPixelArray;

	//To toggle backface culling on and off
	bool backFaceToggle = true;


};

Model model1;
Model model2;

/*Classes LoadModel, TransformToPerspective and VectorOperations
are used for Perspective Transformation

The only Addition is that of a pointPixelArray which stores
the pixel coordinates of each vertex in the polygon*/

class LoadModel
{
	Model currentModel;
	void InitArray(float Arr[],int colCount,int val)
	{
		for(int i =0;i<colCount;i++)
		{
			Arr[i]=val;
		}
	}
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
		currentModel.numOfPoints = atoi(words[1].c_str());
		currentModel.numOfPoly = atoi(words[2].c_str());
	}

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

	int FindNumOfPoints(float Arr[])
	{
		int result=0;
		for(int i=0;i<currentModel.numOfPoly;i++)
		{
			if(Arr[i]>result)
			{
				result=Arr[i];
			}
		}
		return result;
	}
public:
	Model TextToModel(char filepath[])
	{
		bool checkIfFirstLine = true;
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
					cout<<"\nLOG : Points : "<<currentModel.numOfPoints<<" | Polygons : "<<currentModel.numOfPoly;
					break;
				}
			}
			if(currentModel.numOfPoints == 0 || currentModel.numOfPoly == 0)
			{
				cout<<"\nERROR LOG : No Points or Polygons found";
				exit(0);
			}

			string pointStringArray[currentModel.numOfPoints][3];
			float pointFloatArray[currentModel.numOfPoints][3];

			for(int i=0;i<currentModel.numOfPoints;i++)
			{
				InitArray(pointFloatArray[i],3,0);
			}
			for(int i=0; i<currentModel.numOfPoints; i++)
			{
				getline(dFile,line);
				LineToWords(pointStringArray[i], pointFloatArray[i], line);
			}

			string testStringArray[currentModel.numOfPoly];
			float testFloatArray[currentModel.numOfPoly];
			string x;

			for(int i=0;i<currentModel.numOfPoly;i++)
			{
				getline(dFile,line);
				testStringArray[i]=line;
				x=GetCharacter(testStringArray[i]);
				testFloatArray[i]=atoi(x.c_str());
			}



			currentModel.maxPolygon=FindNumOfPoints(testFloatArray)+1;
			cout<<"\n Max Polygon : "<<currentModel.maxPolygon;
			string polygonStringArray[currentModel.numOfPoly][currentModel.maxPolygon];
			float polygonFloatArray[currentModel.numOfPoly][currentModel.maxPolygon];
			for(int i=0;i<currentModel.numOfPoly;i++)
			{
				InitArray(polygonFloatArray[i],currentModel.maxPolygon,0);
			}
			for(int i=0;i<currentModel.numOfPoly;i++)
			{
				LineToWords(polygonStringArray[i], polygonFloatArray[i], testStringArray[i]);
			}



			currentModel.pointArray = new float *[currentModel.numOfPoints];
			for(int i=0;i<currentModel.numOfPoints; i++)
			{
				currentModel.pointArray[i]=new float[4];
				InitArray(currentModel.pointArray[i],4,1);
			}

			for(int i = 0; i<currentModel.numOfPoints; i++)
			{
				for(int j=0; j<3; j++)
				{
					currentModel.pointArray[i][j]=pointFloatArray[i][j];
				}
			}

			currentModel.pointPixelArray = new int *[currentModel.numOfPoints];
			for(int i=0;i<currentModel.numOfPoints; i++)
			{
				currentModel.pointPixelArray[i]=new int[3];
				currentModel.pointPixelArray[i][0]=0;
				currentModel.pointPixelArray[i][1]=0;
				currentModel.pointPixelArray[i][2]=0;
			}

			currentModel.polygonArray = new float *[currentModel.numOfPoly];
			for(int i=0;i<currentModel.numOfPoly; i++)
			{
				currentModel.polygonArray[i]=new float[currentModel.maxPolygon];
				InitArray(currentModel.polygonArray[i],currentModel.maxPolygon,0);
			}
			for(int i = 0; i<currentModel.numOfPoly; i++)
			{
				for(int j=0; j<currentModel.maxPolygon; j++)
				{
					currentModel.polygonArray[i][j]=polygonFloatArray[i][j];
				}
			}

			dFile.close();
		}
		else
		{
			cout<<"\nERROR LOG : Cannot open file. Check filepath";
		}
		cout<<"\nLOG : Arrays Created";
		return currentModel;
	}

	void DestroyModel(Model model)
	{
		for(int i=0;i<model.numOfPoints;i++)
		{
			delete[] model.pointArray[i];
		}
		delete[] model.pointArray;


		for(int i=0;i<model.numOfPoly;i++)
		{
			delete[] model.polygonArray[i];
		}
		delete[] model.polygonArray;

		for(int i=0;i<model.numOfPoints;i++)
		{
			delete[] model.pointPixelArray[i];
		}
		delete[] model.pointPixelArray;

		cout<<"\nLOG : Arrays Destroyed";
	}

	void DisplayModel(Model model)
	{
		cout<<"\nLOG : Point Array \n";
		for(int i = 0; i<model.numOfPoints; i++)
		{
			cout<<"\n";
			for(int j=0; j<4; j++)
			{
				cout<<"\t"<<model.pointArray[i][j];
			}
		}
		cout<<"\nLOG : Polygon Array \n";
		for(int i = 0; i<model.numOfPoly; i++)
		{
			cout<<"\n";
			for(int j=0; j<model.maxPolygon; j++)
			{
				cout<<"\t"<<model.polygonArray[i][j];
			}
		}
		cout<<"\nLOG : Pixel Array \n";
		for(int i=0;i<model.numOfPoints; i++)
		{
			cout<<"\n"<<model.pointPixelArray[i][0];
			cout<<"\t"<<model.pointPixelArray[i][1];
			cout<<"\t"<<model.pointPixelArray[i][2];
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
	    for (int i = 0; i < 3; i++)

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
	float d=1.5,f=100,h=1;

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
	void CreateUVN(Model model)
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
	void MviewFunction(Model model)
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
		for(int i=0;i<model.numOfPoints;i++)
		{
			PointArrayMatMul(Mview,model.pointArray[i]);
		}
	}

		//Function to create the Mpers Matrix and multiply the model coordinates with it
		void MpersFunction(Model model)
		{
			//Mpers Matrix created
			InitMatrix(Mpers);
			Mpers[0][0]=d/h;	Mpers[1][1]=d/h;	Mpers[2][2]=f/(f-d);
			Mpers[2][3]=(-d*f)/(f-d); Mpers[3][2]=1;

			//Multiply the View Coordinates with Mpers
			for(int i=0;i<model.numOfPoints;i++)
			{
				PointArrayMatMul(Mpers,model.pointArray[i]);
			}

			//X,Y,Z values in pointArray are divided by

			for(int i=0;i<model.numOfPoints;i++)
			{
				for(int j=0;j<3;j++)
				{
					model.pointArray[i][j]=model.pointArray[i][j]/model.pointArray[i][3];
				}
			}
		}
		//Backface Culling Function
		void RemoveBackFaces(Model model)
		{
			float tempPointArray[model.numOfPoints][3];
			//using tempPointArray for calculating the backfaces
			VectorOperations vecOps;
			for(int i=0;i<model.numOfPoints;i++)
			{
				for(int j=0;j<3;j++)
				{
					tempPointArray[i][j]=model.pointArray[i][j];
				}
			}

			float normalArray[model.numOfPoly][3];
			float v1[3],v2[3],temp;
			int a,b,c;

			for(int i=0;i<model.numOfPoly;i++)
			{
				//Check to make sure that polygon has atleast 3 sides
				if(model.polygonArray[i][0]>2)
				{
					//a,b,c are the first 3 vertices in any polygon
					a=model.polygonArray[i][1];
					b=model.polygonArray[i][2];
					c=model.polygonArray[i][3];

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
						if(model.backFaceToggle==true)
						{
							//number of sides of backfacing polygon are made 0.
							model.polygonArray[i][0]=0;
						}
					}
				}
			}
		}
public:
	//Caller function for all the Transformation functions
	void TransformToPerspectiveCallerFunction(Model model)
	{
		CreateUVN(model);
		MviewFunction(model);
		MpersFunction(model);
		RemoveBackFaces(model);
	}
};
class ScanConversion
{
	//Used to generate a random number for the colour to be filled in each polygon
	float RandomNumber()
	{
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		return r;
	}

	//Sort the Active Edge Table
	void InsertionSort()
	{
		int i,j;
		Edge temp;
		for(i=1;i<AET.size();i++)
		{
			temp.yMax = AET[i].yMax;
			temp.xOfYMin = AET[i].xOfYMin;
			temp.slope = AET[i].slope;
			temp.ZYSlope = AET[i].ZYSlope;
			temp.zOfYMin = AET[i].zOfYMin;
			j=i-1;

			while((temp.xOfYMin < AET[j].xOfYMin) && (j >= 0))
			{
				AET[j+1].yMax = AET[j].yMax;
				AET[j+1].xOfYMin = AET[j].xOfYMin;
				AET[j+1].slope = AET[j].slope;
				AET[j+1].ZYSlope = AET[j].ZYSlope;
				AET[j+1].zOfYMin = AET[j].zOfYMin;

				j = j-1;
			}
			AET[j+1].yMax = temp.yMax;
			AET[j+1].xOfYMin = temp.xOfYMin;
			AET[j+1].slope = temp.slope;
			AET[j+1].ZYSlope = temp.ZYSlope;
			AET[j+1].zOfYMin = temp.zOfYMin;

		}
	}

	//Update the X and Z values for rach scanline
	void UpdateXZ()
	{
		for(int i=0;i<AET.size();i++)
		{
			AET[i].xOfYMin = AET[i].xOfYMin + AET[i].slope;
			AET[i].zOfYMin = AET[i].zOfYMin + AET[i].ZYSlope;
		}
	}
public:

	/*Fill the Master Table of one of the models with the Edge Data
	 * It returns the fully filled Master Table
	 * The model for which the edge data is to be filled is passed as a parameter
	 */
	vector<vector<vector<Edge> > > FillEdgeTable(Model model)
	{
		int start,end,numOfSides, returnPoint;
		vector<vector<vector<Edge> > > mT;

		//For each polygon
		for(int i=0;i<model.numOfPoly;i++)
		{
			//A temporary edge table is created for each polygon
			vector<vector<Edge> > tempET(windowHeight);

			//returnPoint is a reference to complete the polygon by saving the first vertex
			returnPoint = model.polygonArray[i][1];

			//Number of sides in each polygon
			numOfSides = model.polygonArray[i][0];

			//For each edge
			for(int j=1;j<numOfSides+1;j++)
			{
				//Temporary edge to hold the edge data to be filled
				Edge e;
				int x1,x2,y1,y2,z1,z2,scanline;
				float m,n;

				//The start point for each edge in the polygon
				start = model.polygonArray[i][j];

				//To find the end point of the edge
				if(j==numOfSides)
				{
					//For the last edge in the polygon
					end=returnPoint;
				}
				else
				{
					//For other edges in the polygon
					end = model.polygonArray[i][j+1];
				}
				if(start != 0)
				{
					/*x1,y1 and z1 hold the pixel values for the X,Y and Z coordinates of the
					 * start point of the edge
					 *
					 * Similarly x2,y2 and z2 do the same for the end point
					 */

					x1 = model.pointPixelArray[start-1][0];	y1 = model.pointPixelArray[start-1][1];	z1 = model.pointPixelArray[start-1][2];
					x2 = model.pointPixelArray[end-1][0];	y2 = model.pointPixelArray[end-1][1];	z2 = model.pointPixelArray[end-1][2];

					//Horizontal Edges are ignored
					if(y2==y1)
					{
						continue;
					}

					//For vertical lines, the slope inverse is 0
					if(x2==x1)
					{
						e.slope = 0;
					}

					//Slope inverse is calculated
					else
					{
						m = ((float)(y2-y1))/((float)(x2-x1));
						e.slope = 1/m;
					}

					/*Determining the Y-Max and Y-Min values
					 * Supporting values line x of YMin and z of YMin are also determined
					 */
					if(y1>y2)
					{
						scanline = y2;
						e.yMax = y1;
						e.xOfYMin = x2;
						e.zOfYMin = z2;
					}
					else
					{
						scanline = y1;
						e.yMax = y2;
						e.xOfYMin = x1;
						e.zOfYMin = z1;
					}

					//"Slope" inverse of the Z value for the edge is calculated
					n = (float)((float)(z1-z2)/(float)(y1-y2));
					e.ZYSlope = n;

					//The edge is pushed into the Temporary Edge table
					tempET[scanline].push_back(e);
				}
			}

			//The edge table for a polygon is pushed into the Master Table
			mT.push_back(tempET);
		}
		//Master Table is returned
		return mT;
	}

	/*The edge table is passed to this function to fill each polygon with a random colour
	 *
	 */
	void ScanlineFill(vector<vector<vector<Edge> > > mT)
	{
		/*For the number of polygons stored in the master table
		 * This will be less than the actual number of polygons in the model
		 * Because of backface culling which will nearly halve the polygons that must be filled
		 */
		for(int k=0;k<mT.size();k++)
		{
			int i, j, z1, z2, x1, ymax1, x2, ymax2, fillFlag = 0, coordCount;
			float zp,ZXSlope = 0;

			//Random R,G,B values for the polygon colour
			float r=RandomNumber();
			float g=RandomNumber();
			float b=RandomNumber();

			/*For Every scanline
			 * Starting from bottom to the Window Height
			 */
			for(i=0;i<windowHeight;i++)
			{
				//All the Edges starting at the "ith" scanline are moved to the Active Edge Table
				for(j=0;j<mT[k][i].size();j++)
				{
					AET.push_back(mT[k][i][j]);
				}

				//If the "ith" scanline has moved above the YMax of an edge, it is removed from the AET
				for(j=0;j<AET.size();j++)
				{
					if(AET[j].yMax<=i)
					{
						AET.erase(AET.begin()+j);
					}
				}

				/*Remaining edges in the Active Edge Table are
				 * Sorted according to their X-of-YMin values
				 *
				 * So that the filling is done left to right
				 */
				InsertionSort();

				//Initializations
				j = 0;
				fillFlag = 0;
				coordCount = 0;
				x1 = 0;
				x2 = 0;
				z1 = 0;
				z2 = 0;
				ymax1 = 0;
				ymax2 = 0;
				zp=0;

				//For every edge currently in the Active Edge Table
				while(j < AET.size())
				{
					//Odd Even rule for scanline filling
					if(coordCount%2 == 0)
					{
						//X1 and Z1 values for the scanline
						x1 = (int)(AET[j].xOfYMin);
						z1 = (AET[j].zOfYMin);
						ymax1 = AET[j].yMax;
						if (x1==x2)
						{
						/* three cases can arrive-
							1. lines are towards top of the intersection
							2. lines are towards bottom
							3. one line is towards top and other is towards bottom
						*/
							if (((x1==ymax1)&&(x2!=ymax2))||((x1!=ymax1)&&(x2==ymax2)))
							{
								x2 = x1;
								ymax2 = ymax1;
							}

							else
							{
								coordCount++;
							}
						}

						else
						{
								coordCount++;
						}
					}
					/*
					 * To make sure that there are 2 X and Z values before interpolation
					 */
					else
					{
						x2 = (int)(AET[j].xOfYMin);
						z2 = (AET[j].zOfYMin);
						ymax2 = AET[j].yMax;

						fillFlag = 0;

						// checking for intersection
						if (x1==x2)
						{
						/*three cases can arrive-
							1. lines are towards top of the intersection
							2. lines are towards bottom
							3. one line is towards top and other is towards bottom
						*/
							if (((x1==ymax1)&&(x2!=ymax2))||((x1!=ymax1)&&(x2==ymax2)))
							{
								x1 = x2;
								ymax1 = ymax2;
							}
							else
							{
								coordCount++;
								fillFlag = 1;
							}
						}
						else
						{
								coordCount++;
								fillFlag = 1;
						}
					}
					if(fillFlag)
					{

						//Interpolation incrementally done
						if(x2==x1)
						{
							ZXSlope = 0;
						}
						else
						{
							ZXSlope = ((float)(z1-z2)/(float)(x1-x2));
						}
						//Zp value is initialized to the Z value of left edge
						zp=z1;
						glBegin(GL_POINTS);

						//For every pixel between the two edges
						for(int a=x1;a<=x2;a++)
						{
							//Zp value is calculated by adding the interpolation value
							zp=zp+ZXSlope;

							//The Z value of a pixel shows that it overlaps with the previous value
							if(zp<zBuffer[a][i])
							{
								//Z buffer value is changed
								zBuffer[a][i]=zp;

								//Frame buffer value is changed to that of the pixel colour
								frameBuffer[a][i][0]=r;
								frameBuffer[a][i][1]=g;
								frameBuffer[a][i][2]=b;

								//Pixel is only drawn if it is not hidden by some other pixel value
								glColor3fv(frameBuffer[a][i]);
								glVertex2i(a,i);
							}

						}
						glEnd();
						glFlush();
					}
					j++;
				}
				//The XofYmin and ZofYmin values are incremented
				UpdateXZ();
			}

			//Active Edge Table is cleared
			AET.clear();
		}
		glutSwapBuffers();
	}
};

//Convert the normalized coordinates to the
void CoordToPixel(Model model)
{
	for(int i=0;i<model.numOfPoints; i++)
	{
		model.pointPixelArray[i][0]=(model.pointArray[i][0]+1)*(windowWidth/2);
		model.pointPixelArray[i][1]=(model.pointArray[i][1]+1)*(windowHeight/2);
		model.pointPixelArray[i][2]=(model.pointArray[i][2])*(windowHeight);
	}
}

//Function to draw the outlines of the polygons
void DrawPixels(Model model)
{
	int start,end,numOfSides, returnPoint;
	glBegin(GL_LINES);
	for(int i=0;i<model.numOfPoly;i++)
	{
		//cout<<"\nLOG : Polygon "<<i+1<<" rendering : ";

		//returnPoint is a reference to complete the polygon by saving the first vertex
		returnPoint = model.polygonArray[i][1];

		//Number of sides in each polygon
		numOfSides = model.polygonArray[i][0];
		for(int j=1;j<numOfSides+1;j++)
		{
			//The start point for each edge in the polygon
			start = model.polygonArray[i][j];

			//To find the end point of the edge
			if(j==numOfSides)
			{
				//For the last edge in the polygon
				end=returnPoint;
			}
			else
			{
				//For other edges in the polygon
				end = model.polygonArray[i][j+1];
			}
			if(start != 0)
			{

				//cout<<"\nLOG :\t Point "<<start<<" to "<<end;
				//glVertex2d(model.pointPixelArray[start-1][0],model.pointPixelArray[start-1][1]);
				//glVertex2d(model.pointPixelArray[end-1][0],model.pointPixelArray[end-1][1]);
			}
		}
		// flush

	}
	glEnd();
	glFlush();
	// swap buffers
	glutSwapBuffers();
}

//Display Function
void render( void )
{
	InitBuffers();

	ScanConversion sc;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glClearColor(0,0,0,1);

	// clear buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	gluOrtho2D(0,windowWidth,0,windowHeight);

	sc.ScanlineFill(masterTable1);
	//sc.ScanlineFill(masterTable2);

	glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
	DrawPixels(model1);
	//DrawPixels(model2);
}

//Adjust the camera with this function
void AdjustCamera()
{
	char filepath[file.length()+1];
	strcpy(filepath,file.c_str());

	char filepath2[file2.length()+1];
	strcpy(filepath2,file2.c_str());

	InitBuffers();

	ScanConversion sc;
	TransformToPerspective tp;
	LoadModel lm;

	lm.DestroyModel(model1);
	lm.DestroyModel(model2);

	model1 = lm.TextToModel(filepath);
	model2 = lm.TextToModel(filepath2);

	tp.TransformToPerspectiveCallerFunction(model1);
	tp.TransformToPerspectiveCallerFunction(model2);

	CoordToPixel(model1);
	CoordToPixel(model2);

	masterTable1.clear();
	masterTable2.clear();

	masterTable1 = sc.FillEdgeTable(model1);
	masterTable2 = sc.FillEdgeTable(model2);

	sc.ScanlineFill(masterTable1);
	sc.ScanlineFill(masterTable2);

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

	char filepath[file.length()+1];
	strcpy(filepath,file.c_str());

	char filepath2[file2.length()+1];
	strcpy(filepath2,file2.c_str());

	LoadModel loadModel;
	TransformToPerspective transform;
	ScanConversion scanConversion;


	model1 = loadModel.TextToModel(filepath);
	model2 = loadModel.TextToModel(filepath2);

	transform.TransformToPerspectiveCallerFunction(model1);
	transform.TransformToPerspectiveCallerFunction(model2);

	CoordToPixel(model1);
	CoordToPixel(model2);

	loadModel.DisplayModel(model1);
	loadModel.DisplayModel(model2);

	masterTable1 = scanConversion.FillEdgeTable(model1);
	masterTable2 = scanConversion.FillEdgeTable(model2);

	//OpenGL Section
	glutInit( &argc, argv );

	// set windows size
	glutInitWindowSize( windowHeight, windowWidth );

	// set OpenGL display mode
	glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );

	// set window title
	glutCreateWindow( "Lab_2" );
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_CONTINUE_EXECUTION);
	// set rendering function
	glutDisplayFunc( render );

	glutMouseFunc(OnMouseClick);
	glutKeyboardFunc(OnKeyPress);

	glutMainLoop();

	loadModel.DisplayModel(model1);

	loadModel.DestroyModel(model1);
	loadModel.DestroyModel(model2);

	return 0;
}

