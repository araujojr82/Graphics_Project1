#include "cMesh.h"

cMesh::cMesh()
{
	this->numberOfVertices = 0;
	this->numberOfTriangles = 0;
	//
	this->maxExtent = 0.0f;
	this->scaleForUnitBBox = 1.0f;
	return;
}


cMesh::~cMesh()
{

	return;
}

void cMesh::CalculateExtents(void)
{
	// Assume 1st vertx is both max and min
	this->minXYZ.x = this->pVertices[0].x;
	this->minXYZ.y = this->pVertices[0].y;
	this->minXYZ.z = this->pVertices[0].z;
	this->maxXYZ.x = this->pVertices[0].x;
	this->maxXYZ.y = this->pVertices[0].y;
	this->maxXYZ.z = this->pVertices[0].z;


	for (int index = 0; index != this->numberOfVertices; index++)
	{
		if (this->pVertices[index].x < this->minXYZ.x)
		{
			this->minXYZ.x = this->pVertices[index].x;
		}
		if (this->pVertices[index].x > this->maxXYZ.x)
		{
			this->maxXYZ.x = this->pVertices[index].x;
		}
		
		// Y
		if (this->pVertices[index].y < this->minXYZ.y)
		{
			this->minXYZ.y = this->pVertices[index].y;
		}
		if (this->pVertices[index].y > this->maxXYZ.y)
		{
			this->maxXYZ.y = this->pVertices[index].y;
		}

		// Z
		if (this->pVertices[index].z < this->minXYZ.z)
		{
			this->minXYZ.z = this->pVertices[index].z;
		}
		if (this->pVertices[index].z > this->maxXYZ.z)
		{
			this->maxXYZ.z = this->pVertices[index].z;
		}

	}// for (int index...

	this->maxExtentXYZ.x = this->maxXYZ.x - this->minXYZ.x;
	this->maxExtentXYZ.y = this->maxXYZ.y - this->minXYZ.y;
	this->maxExtentXYZ.z = this->maxXYZ.z - this->minXYZ.z;

	this->maxExtent = this->maxExtentXYZ.x;
	if (this->maxExtent < this->maxExtentXYZ.y) 
	{	// Y is bigger
		this->maxExtent = this->maxExtentXYZ.y;
	}
	if (this->maxExtent < this->maxExtentXYZ.z) 
	{	// Z is bigger
		this->maxExtent = this->maxExtentXYZ.z;
	}
	this->scaleForUnitBBox = 1.0f / this->maxExtent;

	return;
}

// Takes an indexed model and makes just a vertex array model
void cMesh::FlattenIndexedModel(void)
{
	// 1. Make a copy of the original vertices
	// 2. Using the triangle array, replace the original data

	int origNumVertices = this->numberOfVertices;

//	cVertex_xyz_rgb vertOrig[MAX_VERTEX_ARRAY_SIZE]; // Stack

	cVertex_xyz_rgb* pVertOrig = new cVertex_xyz_rgb[this->numberOfVertices]; // Heap

	for ( int index = 0; index < origNumVertices; index++ )
	{
		pVertOrig[index] = this->pVertices[index];
	}
	// We now have a copy of the original vertices

	// Adjust the original vertex array
	delete [] this->pVertices;		// GOTCHA!

//	int numberOfVertsTOTALGUESS = this->numberOfVertices * 4;
//	this->pVertices = new cVertex_xyz_rgb[numberOfVertsTOTALGUESS];

	// Actually, the number of vertices would be 
	//	the number of triangles x 3...
	int numberOfVertsNeeded = this->numberOfTriangles * 3;
	numberOfVertsNeeded += 30;	// Add a few, just in case...
	this->pVertices = new cVertex_xyz_rgb[numberOfVertsNeeded];

	int triIndex = 0;
	int vertIndex = 0;
	for ( ; triIndex < this->numberOfTriangles; 
		  triIndex++, vertIndex += 3 )
	{
		// 3 4 132 80 
		int triVert0_index = this->pTriangles[triIndex].vertex_ID_0;
		int triVert1_index = this->pTriangles[triIndex].vertex_ID_1;
		int triVert2_index = this->pTriangles[triIndex].vertex_ID_2;

		cVertex_xyz_rgb V0 = pVertOrig[triVert0_index];
		cVertex_xyz_rgb V1 = pVertOrig[triVert1_index];
		cVertex_xyz_rgb V2 = pVertOrig[triVert2_index];

		this->pVertices[vertIndex + 0] = V0;
		this->pVertices[vertIndex + 1] = V1;
		this->pVertices[vertIndex + 2] = V2;
	}//for ( ; 
	
	// Update the number of vertices
	// HACK: -3 YIKES!!! 
	this->numberOfVertices = vertIndex - 3;	

	return;
}

