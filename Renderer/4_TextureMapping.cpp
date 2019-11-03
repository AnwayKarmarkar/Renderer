/*
 * proj4.cpp
 *
 *  Created on: Mar 19, 2019
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

//For reusing the load model class for 2 or 3 dimensions
int dimension = 3;

/*
 * ShadingModel = 1 for Constant Shading
 * ShadingModel = 2 for Goraud Shading
 * ShadingModel = 3 for Phong Shading
 */
int ShadingModel = 3;

//Filenames for the model

string filename = "right-wall";
string file = "/home/anway/CG_Data/CG2/D files/dfiles/"+filename+".d.txt";

string bmpFilename = "grass";
string bmpFile = "/home/anway/CG_Data/CG2/D files/dfiles/"+bmpFilename+".bmp";

//Camera Vector
float C[3] = {0,0,-3};

//V, L vectors
float Viewer[3] = {0,0,0};
float Light[3] = {8,8,0};
float H[3];

//Incident light
float iLight[3] = {1.0 ,1.0, 1.0};

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

	//Vertex Normal of minimum Y value
	float normOfYMin[3];

	//slope inverse of the edge
	float slope;

	//Change of Z value corresponding to change in Y value
	float ZYSlope;

	//Change in Normal with the change in Y
	float normYSlope[3];
};

/*Std vectors for the edges of the two models.
They are 3 dimensional because for each polygon, the Edge table consists of two dimensions.
Master Table stores edge information for each polygon*/

vector<vector<vector<Edge> > > masterTable1;
vector<vector<vector<int> > > bmpRGB;
int tWidth,tHeight;

//Active Edge Table
vector<Edge> AET;

class Model
{
public:
	int numOfPoly,numOfPoints,maxPolygon = 0;
	vector<vector<float> > vertexArray;
	vector<vector<int> > vertexPixelArray;
	vector<vector<int> > polyArray;
	vector<vector<float> > normalArray;
	vector<vector<float> > vertexNormalArray;

	//Ambient, Diffused and Specular reflectivity
	float kA[3] = {0.3,0.3,0.3};
	float kD[3] = {0.6,0.6,0.6};
	float kS[3] = {0.9,0.9,0.9};

	//Shininess used in Specular term
	float Shininess = 10;

	//To toggle backface culling on and off
	bool backFaceToggle = true;
};

Model model1;

//Used in changing the camera position using the keyboard
Model backupModel;

/*Classes LoadModel, TransformToPerspective and VectorOperations
are used for Perspective Transformation

The only Addition is that of a pointPixelArray which stores
the pixel coordinates of each vertex in the polygon*/
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




class LoadModel
{
	Model *cM;

	vector<string> GetWords(string line)
	{
		vector<string> A;
		string word;
		char x;
		int size = line.size();
		int i,j;
		bool check = false;

		//cout<<"\n"<<line;

		for(i = 0; i < size; i++)
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
					A.push_back(word);
					word="";
				}
			}
			if(line[i+1] == '\0' && word != "")
			{
				A.push_back(word);
			}
		}

		return A;
	}

public:

	void SetCurrentModel(Model *currentModelIn)
	{
		cM = currentModelIn;
	}

	void TextToModel(char filepath[], Model* currentModel)
	{
		ifstream dFile;
		dFile.open(filepath);
		string line;
		string tempString;

		VectorOperations vecOps;
		vector<string> strVec;
		vector<float> flVec;
		vector<int> intVec;

		if(dFile.is_open())
		{
			cout<<"\nLOG : File Opened Successfully";

			while(getline(dFile,line))
			{
				strVec = GetWords(line);

				if(strVec.size() == 3)
				{
					(*cM).numOfPoints = atoi(strVec[1].c_str());

					(*cM).numOfPoly = atoi(strVec[2].c_str());
				}
				else
				{
					cout<<"\nERROR LOG : The number of Vertices and Polygons cannot be read";
				}
				break;
			}
			cout<<"\nPoints : "<<(*cM).numOfPoints<<"\tPolygons : "<<(*cM).numOfPoly;

			for(int i = 0; i < (*cM).numOfPoints; i++)
			{
				getline(dFile,line);

				flVec.clear();
				strVec.clear();

				strVec = GetWords(line);
				float x;

				for(int j = 0; j < strVec.size(); j++)
				{
					x = (atof( strVec[j].c_str()) );

					flVec.push_back(x);
				}

				if(dimension == 3)
				{
					flVec.push_back(1.0);
				}

				(*cM).vertexArray.push_back(flVec);
			}

			for(int i = 0; i < (*cM).numOfPoly; i++)
			{
				getline(dFile,line);
				intVec.clear();
				strVec.clear();

				strVec = GetWords(line);
				int x;

				for(int j = 0; j < strVec.size(); j++)
				{
					x = (atoi( strVec[j].c_str()) );

					intVec.push_back(x);
				}

				(*cM).polyArray.push_back(intVec);
			}
			(*cM).maxPolygon = 0;

			for(int i=0;i<(*cM).numOfPoly;i++)
			{
				if((*cM).polyArray[i][0] > (*cM).maxPolygon)
				{
					(*cM).maxPolygon = (*cM).polyArray[i][0];
				}
			}

			for(int i=0; i<(*cM).numOfPoints; i++)
			{
				vector<int> x;

				x.push_back(0);
				x.push_back(0);
				x.push_back(0);

				(*cM).vertexPixelArray.push_back(x);
			}

			dFile.close();
		}
		else
		{
			cout<<"\nERROR LOG : Cannot open file. Check filepath";
			exit(0);
		}
		cout<<"\nLOG : Arrays Created";
	}

	//Display the data of a given model
	void DisplayModel(Model model)
	{
		cout<<"\n\n\t Vertex Array\n";
		for(int i = 0; i < model.vertexArray.size(); i++)
		{
			cout<<"\n";
			for(int j = 0; j < dimension+1; j++)
			{
				cout<<"\t"<<model.vertexArray[i][j];
			}
		}
		cout<<"\n\n\t Polygon Array\n";
		for(int i = 0; i < model.polyArray.size(); i++)
		{
			cout<<"\n";
			for(int j = 0; j < model.polyArray[i].size(); j++)
			{
				cout<<"\t"<<model.polyArray[i][j];
			}
		}
		cout<<"\n\n\t Pixel Array\n";
		for(int i=0;i<model.numOfPoints;i++)
		{
			cout<<"\n";
			for(int j=0;j<dimension;j++)
			{
				cout<<"\t"<<model.vertexPixelArray[i][j];
			}
		}
		cout<<"\n\n\t Normal Array\n";
		for(int i = 0; i < model.polyArray.size(); i++)
		{
			cout<<"\n";
			for(int j = 0; j < 3; j++)
			{
				cout<<"\t"<<model.normalArray[i][j];
			}
		}
		cout<<"\n\n\t Vertex Normal Array\n";
		for(int i = 0; i < model.vertexNormalArray.size(); i++)
		{
			cout<<"\n";
			for(int j = 0; j < 3; j++)
			{
				cout<<"\t"<<model.vertexNormalArray[i][j];
			}
		}
	}

	//Convert the normalized coordinates to the
	void CoordToPixel(Model *model)
	{
		for(int i=0;i<(*model).numOfPoints; i++)
		{
			(*model).vertexPixelArray[i][0]=((*model).vertexArray[i][0]+1)*(windowWidth/2);
			(*model).vertexPixelArray[i][1]=((*model).vertexArray[i][1]+1)*(windowHeight/2);
			(*model).vertexPixelArray[i][2]=((*model).vertexArray[i][2])*(windowHeight);
		}
	}

	//Wipe data from all the standard vectors
	void DestroyModel(Model *model)
	{
		//(*model).normalArray.clear();
		(*model).polyArray.clear();
		(*model).vertexArray.clear();
		(*model).vertexPixelArray.clear();
	}

	//Calculate the surface normals of every polygon and then aggregate them for each vertex
	void CalcNormals(Model *model)
	{
		float tempPointArray[(*model).numOfPoints][3];
		//using tempPointArray for calculating the backfaces
		VectorOperations vecOps;
		for(int i=0;i<(*model).numOfPoints;i++)
		{
			for(int j=0;j<3;j++)
			{
				tempPointArray[i][j]=(*model).vertexArray[i][j];
			}
		}
		float normalArray[(*model).numOfPoly][3];
		float v1[3],v2[3],temp,div1,div2;
		int a,b,c;
		vector<float > x;
		for(int i=0;i<(*model).numOfPoly;i++)
		{
			//Check to make sure that polygon has atleast 3 sides
			if((*model).polyArray[i][0]>2)
			{
				//a,b,c are the first 3 vertices in any polygon
				a=(*model).polyArray[i][1];
				b=(*model).polyArray[i][2];
				c=(*model).polyArray[i][3];
				//V1 is the vector from a to b
				v1[0]=tempPointArray[b-1][0]-tempPointArray[c-1][0];
				v1[1]=tempPointArray[b-1][1]-tempPointArray[c-1][1];
				v1[2]=tempPointArray[b-1][2]-tempPointArray[c-1][2];

				div1 = vecOps.Normalize(v1);

				v1[0]=v1[0]/div1;
				v1[1]=v1[1]/div1;
				v1[2]=v1[2]/div1;
				//V2 is the vector from b to c
				v2[0]=tempPointArray[a-1][0]-tempPointArray[b-1][0];
				v2[1]=tempPointArray[a-1][1]-tempPointArray[b-1][1];
				v2[2]=tempPointArray[a-1][2]-tempPointArray[b-1][2];

				div2 = vecOps.Normalize(v2);

				v2[0]=v2[0]/div2;
				v2[1]=v2[1]/div2;
				v2[2]=v2[2]/div2;

				//cross product of the two vectors v1 and v2
				vecOps.CrossProduct(v1,v2,normalArray[i]);

				//Normalized cross product
				temp=vecOps.Normalize(normalArray[i]);

				//Final array of the surface normal vectors
				normalArray[i][0]=normalArray[i][0]/temp;
				normalArray[i][1]=normalArray[i][1]/temp;
				normalArray[i][2]=normalArray[i][2]/temp;

				x.clear();
				x.push_back(normalArray[i][0]);
				x.push_back(normalArray[i][1]);
				x.push_back(normalArray[i][2]);

				(*model).normalArray.push_back(x);

			}
		}
		if((*model).normalArray.size() != (*model).numOfPoly)
		{
			cout<<"\nERROR LOG : Normal Array Empty";
			exit(0);
		}

		for(int i=0;i<(*model).vertexArray.size();i++)
		{
			int vertex = i+1;
			vector<int> vertInPoly;
			vector<vector<float> > nArr;
			float sum[3] = {0,0,0};
			float divs;


			for(int j=0;j<(*model).numOfPoly;j++)
			{
				for(int k=1;k<(*model).polyArray[j].size();k++)
				{
					if(vertex == (*model).polyArray[j][k])
					{
						vertInPoly.push_back(j);
					}
				}
			}

			//cout<<"\nVertex "<<vertex<<" in Polygons : ";
			for(int j=0;j<vertInPoly.size();j++)
			{
				//cout<<" "<<vertInPoly[j]+1;
				nArr.push_back( (*model).normalArray[ vertInPoly[j] ] );
			}

			for(int j=0;j<nArr.size();j++)
			{
				sum[0]+=nArr[j][0];
				sum[1]+=nArr[j][1];
				sum[2]+=nArr[j][2];
			}

			divs = vecOps.Normalize(sum);

			sum[0] = sum[0]/divs;
			sum[1] = sum[1]/divs;
			sum[2] = sum[2]/divs;

			x.clear();
			x.push_back(sum[0]);
			x.push_back(sum[1]);
			x.push_back(sum[2]);

			(*model).vertexNormalArray.push_back(x);
		}
		cout<<"\nNormals Calculated";
	}

	void MakeBackups(Model modelA, Model *modelB)
	{
		(*modelB).numOfPoly = modelA.numOfPoly;
		(*modelB).numOfPoints = modelA.numOfPoints;
		(*modelB).maxPolygon = modelA.maxPolygon;

		(*modelB).vertexArray = modelA.vertexArray;
		(*modelB).vertexPixelArray = modelA.vertexPixelArray;
		(*modelB).polyArray = modelA.polyArray;
		(*modelB).normalArray = modelA.normalArray;
		(*modelB).vertexNormalArray = modelA.vertexNormalArray;
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
	void PointArrayMatMul(float A[][4],vector<float> *B)
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
				C[i]+=A[i][j] * (B->at(j));
			}
		}
		for(int i=0;i<4;i++)
		{
			B->at(i)=C[i];
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
		float temp[3],temp1[3],div;

		//Vector N
		for(int i=0;i<3;i++)
		{
			temp[i]=Pref[i]-C[i];
		}
		div=vecOps.Normalize(temp);
		N[0]=temp[0]/div;
		N[1]=temp[1]/div;
		N[2]=temp[2]/div;

		div=vecOps.Normalize(C);
		Viewer[0] = C[0]/div;
		Viewer[1] = C[1]/div;
		Viewer[2] = C[2]/div;

		for(int i=0;i<3;i++)
		{
			temp1[i]=Light[i]-Pref[i];
		}
		div=vecOps.Normalize(temp1);
		Light[0]=temp1[0]/div;
		Light[1]=temp1[1]/div;
		Light[2]=temp1[2]/div;

		//cout<<"\n N : "<<N[0]<<N[1]<<N[2];

		float sum[3],nLight,nViewer,temp2;

		nLight = vecOps.Normalize(Light);
		nViewer = vecOps.Normalize(Viewer);

		for(int j=0;j<3;j++)
		{
			Light[j]=Light[j]/nLight;
			Viewer[j]=Viewer[j]/nViewer;
		}
		for(int j = 0; j<3;j++)
		{
			sum[j] = Light[j] + Viewer[j];
		}
		temp2 = vecOps.Normalize(sum);
		for(int j = 0; j<3;j++)
		{
			H[j] = sum[j] / temp2;
		}




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
	void MviewFunction(Model *model)
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
		for(int i=0;i<(*model).numOfPoints;i++)
		{
			PointArrayMatMul(Mview,&model->vertexArray[i]);
		}
	}

		//Function to create the Mpers Matrix and multiply the model coordinates with it
		void MpersFunction(Model *model)
		{
			//Mpers Matrix created
			InitMatrix(Mpers);
			Mpers[0][0]=d/h;	Mpers[1][1]=d/h;	Mpers[2][2]=f/(f-d);
			Mpers[2][3]=(-d*f)/(f-d); Mpers[3][2]=1;

			//Multiply the View Coordinates with Mpers
			for(int i=0;i<(*model).numOfPoints;i++)
			{
				PointArrayMatMul(Mpers,&model->vertexArray[i]);
			}

			//X,Y,Z values in pointArray are divided by

			for(int i=0;i<(*model).numOfPoints;i++)
			{
				for(int j=0;j<3;j++)
				{
					(*model).vertexArray[i][j]=(*model).vertexArray[i][j]/(*model).vertexArray[i][3];
				}
			}
		}
		//Backface Culling Function
		void RemoveBackFaces(Model *model)
		{
			float tempPointArray[(*model).numOfPoints][3];
			//using tempPointArray for calculating the backfaces
			VectorOperations vecOps;
			for(int i=0;i<(*model).numOfPoints;i++)
			{
				for(int j=0;j<3;j++)
				{
					tempPointArray[i][j]=(*model).vertexArray[i][j];
				}
			}

			float normalArray[(*model).numOfPoly][3];
			float v1[3],v2[3],temp;
			int a,b,c;
			vector<float > x;

			for(int i=0;i<(*model).numOfPoly;i++)
			{
				//Check to make sure that polygon has atleast 3 sides
				if((*model).polyArray[i][0]>2)
				{
					//a,b,c are the first 3 vertices in any polygon
					a=(*model).polyArray[i][1];
					b=(*model).polyArray[i][2];
					c=(*model).polyArray[i][3];

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
					if((*model).backFaceToggle==true)
					{
						if(normalArray[i][2]>0)
						{
							//number of sides of backfacing polygon are made 0.
							(*model).polyArray[i][0]=0;
						}
					}
				}
			}
		}
public:
	//Caller function for all the Transformation functions
	void TransformToPerspectiveCallerFunction(Model *model)
	{
		CreateUVN();
		MviewFunction(model);
		MpersFunction(model);
		RemoveBackFaces(model);
	}
};





class Illumination
{
	VectorOperations vecOps;
public:

	//Returns colour for constant illumination model
	vector<float> ConstantShading(Model model, int polyNumber)
	{
		vector<float> a;

		float amb,diff,spec,result,temp1;
		float nl,nh;
		float temp3=1;
		float normal[3],H[3],sum[3];

		normal[0] = model.normalArray[polyNumber][0];
		normal[1] = model.normalArray[polyNumber][1];
		normal[2] = model.normalArray[polyNumber][2];

		nl = vecOps.DotProduct(normal,Light);

		if(nl<0)
		{
			nl=0;
		}

		for(int j = 0; j<3;j++)
		{
			sum[j] = Light[j] + Viewer[j];
		}
		temp1 = vecOps.Normalize(sum);
		for(int j = 0; j<3;j++)
		{
			H[j] = sum[j] / temp1;
		}
		nh = vecOps.DotProduct(normal,H);

		if(nh<0)
		{
			nh=0;
		}

		temp3 = pow(nh,model.Shininess);
		nh=temp3;

		for(int i=0; i<3;i++)
		{
			amb = iLight[i] * model.kA[i];

			diff = ( iLight[i] * model.kD[i] * nl );

			spec = ( iLight[i] * model.kS[i] * nh );

			result = amb + diff + spec;

			a.push_back(result);
		}

		return a;
	}
	//Returns phong illumination for each pixel
	vector<float> GetPhongColour(Model model,float normal[])
	{
		vector<float> a;

		float amb,diff,spec,result,temp1;
		float nl,nh;
		float temp3=1;

		nl = vecOps.DotProduct(normal,Light);

		if(nl<0)
		{
			nl=0;
		}

		nh = vecOps.DotProduct(normal,H);


		temp3 = pow(nh,model.Shininess);
		nh=temp3;

		//cout<<"\n";
		for(int i=0; i<3;i++)
		{
			amb = iLight[i] * model.kA[i];

			diff = ( iLight[i] * model.kD[i] * nl );

			spec = ( iLight[i] * model.kS[i] * nh );
			//cout<<"\nSpec = "<<spec;
			if(spec<0)
			{
				spec=0;
			}
			if(diff<0)
			{
				diff=0;
			}
			result = amb + diff + spec;

			a.push_back(result);
		}

		return a;
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

			if(ShadingModel == 3)
			{
				temp.normYSlope[0] = AET[i].normYSlope[0];
				temp.normYSlope[1] = AET[i].normYSlope[1];
				temp.normYSlope[2] = AET[i].normYSlope[2];

				temp.normOfYMin[0] = AET[i].normOfYMin[0];
				temp.normOfYMin[1] = AET[i].normOfYMin[1];
				temp.normOfYMin[2] = AET[i].normOfYMin[2];
			}
			j=i-1;

			while((temp.xOfYMin < AET[j].xOfYMin) && (j >= 0))
			{
				AET[j+1].yMax = AET[j].yMax;
				AET[j+1].xOfYMin = AET[j].xOfYMin;
				AET[j+1].slope = AET[j].slope;
				AET[j+1].ZYSlope = AET[j].ZYSlope;
				AET[j+1].zOfYMin = AET[j].zOfYMin;

				if(ShadingModel == 3)
				{
					AET[j+1].normYSlope[0] = AET[j].normYSlope[0];
					AET[j+1].normYSlope[1] = AET[j].normYSlope[1];
					AET[j+1].normYSlope[2] = AET[j].normYSlope[2];

					AET[j+1].normOfYMin[0] = AET[j].normOfYMin[0];
					AET[j+1].normOfYMin[1] = AET[j].normOfYMin[1];
					AET[j+1].normOfYMin[2] = AET[j].normOfYMin[2];
				}


				j = j-1;
			}
			AET[j+1].yMax = temp.yMax;
			AET[j+1].xOfYMin = temp.xOfYMin;
			AET[j+1].slope = temp.slope;
			AET[j+1].ZYSlope = temp.ZYSlope;
			AET[j+1].zOfYMin = temp.zOfYMin;


			if(ShadingModel == 3)
			{
				AET[j+1].normYSlope[0] = temp.normYSlope[0];
				AET[j+1].normYSlope[1] = temp.normYSlope[1];
				AET[j+1].normYSlope[2] = temp.normYSlope[2];

				AET[j+1].normOfYMin[0] = temp.normOfYMin[0];
				AET[j+1].normOfYMin[1] = temp.normOfYMin[1];
				AET[j+1].normOfYMin[2] = temp.normOfYMin[2];
			}
		}
	}

	//Update the X,Z,colour and normal values for each scanline
	void UpdateXZ()
	{
		for(int i=0;i<AET.size();i++)
		{
			AET[i].xOfYMin = AET[i].xOfYMin + AET[i].slope;
			AET[i].zOfYMin = AET[i].zOfYMin + AET[i].ZYSlope;

			if(ShadingModel == 3)
			{
				AET[i].normOfYMin[0] = AET[i].normOfYMin[0] + AET[i].normYSlope[0];
				AET[i].normOfYMin[1] = AET[i].normOfYMin[1] + AET[i].normYSlope[1];
				AET[i].normOfYMin[2] = AET[i].normOfYMin[2] + AET[i].normYSlope[2];
			}
		}
	}
public:

	Illumination ill;

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
			returnPoint = model.polyArray[i][1];

			//Number of sides in each polygon
			numOfSides = model.polyArray[i][0];

			//For each edge
			for(int j=1;j<numOfSides+1;j++)
			{
				//Temporary edge to hold the edge data to be filled
				Edge e;
				int x1,x2,y1,y2,z1,z2,scanline;
				float m,n;

				float n1[3];
				float n2[3];

				//The start point for each edge in the polygon
				start = model.polyArray[i][j];

				//To find the end point of the edge
				if(j==numOfSides)
				{
					//For the last edge in the polygon
					end=returnPoint;
				}
				else
				{
					//For other edges in the polygon
					end = model.polyArray[i][j+1];
				}
				if(start != 0)
				{
					/*x1,y1 and z1 hold the pixel values for the X,Y and Z coordinates of the
					 * start point of the edge
					 *
					 * Similarly x2,y2 and z2 do the same for the end point
					 */

					x1 = model.vertexPixelArray[start-1][0];	y1 = model.vertexPixelArray[start-1][1];	z1 = model.vertexPixelArray[start-1][2];
					x2 = model.vertexPixelArray[end-1][0];	y2 = model.vertexPixelArray[end-1][1];	z2 = model.vertexPixelArray[end-1][2];

					if(ShadingModel == 3)
					{
						n1[0] = model.vertexNormalArray[start-1][0];
						n1[1] = model.vertexNormalArray[start-1][1];
						n1[2] = model.vertexNormalArray[start-1][2];

						n2[0] = model.vertexNormalArray[end-1][0];
						n2[1] = model.vertexNormalArray[end-1][1];
						n2[2] = model.vertexNormalArray[end-1][2];
					}

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

						//Colour and normal values for the Y min of each edge
						if(ShadingModel == 3)
						{
							e.normOfYMin[0] = n2[0];
							e.normOfYMin[1] = n2[1];
							e.normOfYMin[2] = n2[2];

						}
					}
					else
					{
						scanline = y1;
						e.yMax = y2;
						e.xOfYMin = x1;
						e.zOfYMin = z1;

						if(ShadingModel == 3)
						{
							e.normOfYMin[0] = n1[0];
							e.normOfYMin[1] = n1[1];
							e.normOfYMin[2] = n1[2];

						}
					}



					if(ShadingModel == 3)
					{
						e.normYSlope[0]= (float)((float)(n1[0]-n2[0])/(float)(y1-y2));
						e.normYSlope[1]= (float)((float)(n1[1]-n2[1])/(float)(y1-y2));
						e.normYSlope[2]= (float)((float)(n1[2]-n2[2])/(float)(y1-y2));
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
	void ScanlineFill(vector<vector<vector<Edge> > > mT, Model model)
	{
		/*For the number of polygons stored in the master table
		 * This will be less than the actual number of polygons in the model
		 * Because of backface culling which will nearly halve the polygons that must be filled
		 */
		for(int k=0;k<mT.size();k++)
		{
			int i, j, z1, z2, x1, ymax1, x2, ymax2, fillFlag = 0, coordCount;
			float zp,ZXSlope = 0;

			float NormXSlope[3] = {0,0,0};

			float r,g,b;
			vector<float> RGB;

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
				float i1[3] = {0,0,0};
				float i2[3] = {0,0,0};
				float n1[3] = {0,0,0};
				float n2[3] = {0,0,0};
				ymax1 = 0;
				ymax2 = 0;
				zp=0;
				float ip[3] = {0,0,0};
				float np[3] = {0,0,0};

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

						if(ShadingModel == 3)
						{
							n1[0] = (AET[j].normOfYMin[0]);
							n1[1] = (AET[j].normOfYMin[1]);
							n1[2] = (AET[j].normOfYMin[2]);

						}

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

						if(ShadingModel == 3)
						{
							n2[0] = (AET[j].normOfYMin[0]);
							n2[1] = (AET[j].normOfYMin[1]);
							n2[2] = (AET[j].normOfYMin[2]);

							/*cout<<"\n"<<n2[0];
							cout<<"\t"<<n2[1];
							cout<<"\t"<<n2[2];*/
						}

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

							//Colour and normal slopes along each scanline
							if(ShadingModel == 3)
							{
								NormXSlope[0] = ((float)(n1[0]-n2[0])/(float)(x1-x2));
								NormXSlope[1] = ((float)(n1[1]-n2[1])/(float)(x1-x2));
								NormXSlope[2] = ((float)(n1[2]-n2[2])/(float)(x1-x2));

							}
						}
						//Zp value is initialized to the Z value of left edge
						zp=z1;

						if(ShadingModel == 2)
						{
							ip[0] = i1[0];	ip[1] = i1[1];	ip[2] = i1[2];
						}
						if(ShadingModel == 3)
						{
							np[0] = n1[0];	np[1] = n1[1];	np[2] = n1[2];
						}

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

								//For Phong shading the normal value is calculated for the pixel.
								//After which the Colour is calculated.
								int col[3];
								if(ShadingModel == 3)
								{
									VectorOperations vecOps;
									float cArr[3],divN;
									float u,v;
									cArr[0]=np[0];	cArr[1]=np[1];	cArr[2]=np[2];
									divN = vecOps.Normalize(cArr);

									np[0]=cArr[0]/divN;
									np[1]=cArr[1]/divN;
									np[2]=cArr[2]/divN;

									//Texture mapping part
									//Cylinder Mapping

									float z=(np[2]+1)/2;
									float theta = atan(np[1]/np[0]);
									theta = (theta + M_PI/2)*2;

									//Calculate u,v
									u=theta/(2*M_PI);  //asin(np[0])/M_PI + 0.5;
									v=z;  //asin(np[1])/M_PI + 0.5;

									u=u * tWidth;
									v=v * tHeight;

									//texel values
									int ui = (int)u; int vi = (int)v;
									//cout<<"\n"<<ui<<"\t"<<vi;

									//Get texel RGB values
									if(ui>=0 && vi>=0 && ui<tWidth && vi<tHeight )
									{
										col[0]=(int)bmpRGB[vi][ui][0];
										col[1]=(int)bmpRGB[vi][ui][1];
										col[2]=(int)bmpRGB[vi][ui][2];
									}
									else
									{
										//cout<<"\n"<<ui<<"\t"<<vi;
										col[0]=0;col[1]=0;col[2]=0;
									}

									//Get the phong illumination
									vector<float> phongColour;
									phongColour = ill.GetPhongColour(model,np);

									np[0]=np[0] + NormXSlope[0];
									np[1]=np[1] + NormXSlope[1];
									np[2]=np[2] + NormXSlope[2];

									//Multiply intensity to the RGB values
									r=(float)col[0]*phongColour[0];
									g=(float)col[1]*phongColour[1];
									b=(float)col[2]*phongColour[2];

									//Normalize RGB
									r=(float)r/255;
									g=(float)g/255;
									b=(float)b/255;

									//After this it is the same process as the other shading models
								}

								//Frame buffer value is changed to that of the pixel colour
								frameBuffer[a][i][0]=r;
								frameBuffer[a][i][1]=g;
								frameBuffer[a][i][2]=b;

								//Pixel is only drawn if it is not hidden by some other pixel value
								glColor3fv(frameBuffer[a][i]);
								glBegin(GL_POINTS);
									glVertex2i(a,i);
								glEnd();
								glFlush();
							}
						}
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




class BMPReader
{
public:
	void ReadBMP(char * fileName,vector<vector<vector<int> > > *bmpRGB1)
	{
	    int i;
	    FILE* f = fopen(fileName, "rb");
	    if(f==NULL)
	    {
	    	cout<<"\nCannot Open Texture Image. Abort";
	    	exit(0);
	    }
	    unsigned char info[54];
	    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

	    ;

	    // extract image height and width from header
	    int width = *(int*)&info[18];
	    int height = *(int*)&info[22];

	    tWidth = width;
	    tHeight = height;

	    cout<<"\nWidth : "<<width<<"\tHeight : "<<height;

	    int size = 3 * width * height;
	    unsigned char data [size]; // allocate 3 bytes per pixel
	    fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	    fclose(f);
	    for(i = 0; i < size; i += 3)
	    {
	            unsigned char tmp = data[i];
	            data[i] = data[i+2];
	            data[i+2] = tmp;
	    }

	    //Put each individual texel data into a std vector to be accessed during scan conversion.
	    for(i=0;i<height;i++)
	    {
	    	vector<vector<int> > tempVec;
	    	for(int j=0;j<width;j++)
	    	{
	    		vector<int> temp;
	    		temp.push_back(data[3 * (i * width + j)]);
	    		temp.push_back(data[3 * (i * width + j) + 1]);
	    		temp.push_back(data[3 * (i * width + j) + 2]);

	    		tempVec.push_back(temp);
	    	}
	    	(*bmpRGB1).push_back(tempVec);
	    }
	    cout<<"\nBMP file read";
	}
};





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

	sc.ScanlineFill(masterTable1,model1);
}

//Adjust the camera with this function
void AdjustCamera()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	char filepath[file.length()+1];
	strcpy(filepath,file.c_str());
	InitBuffers();

	ScanConversion sc;
	TransformToPerspective tp;
	LoadModel lm;

	lm.DestroyModel(&model1);
	lm.MakeBackups(backupModel,&model1);

	tp.TransformToPerspectiveCallerFunction(&model1);

	lm.CoordToPixel(&model1);

	masterTable1.clear();

	masterTable1 = sc.FillEdgeTable(model1);

	sc.ScanlineFill(masterTable1,model1);

	glutPostRedisplay();
}

//Mouse Function
void OnMouseClick(int button, int state, int mx, int my)
{
    if(button==GLUT_LEFT_BUTTON)
    {
    	C[2]+=0.25;
    }
    else if(button==GLUT_RIGHT_BUTTON)
    {
    	C[2]-=0.25;
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

	char bmpFilepath[bmpFile.length()+1];
	strcpy(bmpFilepath,bmpFile.c_str());

	LoadModel loadModel;
	TransformToPerspective transform;
	ScanConversion scanConversion;
	BMPReader bmpR;


	loadModel.SetCurrentModel(&model1);
	loadModel.TextToModel(filepath, &model1);
	loadModel.CalcNormals(&model1);
	loadModel.MakeBackups(model1,&backupModel);

	transform.TransformToPerspectiveCallerFunction(&model1);

	loadModel.CoordToPixel(&model1);
	bmpR.ReadBMP(bmpFilepath,&bmpRGB);
	//loadModel.DisplayModel(model1);

	masterTable1 = scanConversion.FillEdgeTable(model1);

	//OpenGL Section
	glutInit( &argc, argv );

	// set windows size
	glutInitWindowSize( windowHeight, windowWidth );

	// set OpenGL display mode
	glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );

	// set window title
	glutCreateWindow( "Lab_4" );
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_CONTINUE_EXECUTION);
	// set rendering function
	glutDisplayFunc( render );

	//glutMouseFunc(OnMouseClick);
	//glutKeyboardFunc(OnKeyPress);

	glutMainLoop();

	return 0;
}

