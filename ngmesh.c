#include "ngmesh.h"
#include <stdlib.h>

ngMesh* ngMeshAlloc()
{
	ngMesh* mesh = calloc(1, sizeof(*mesh));
	return mesh;
}

int ngMeshInitFromArrays(ngMesh* mesh, float* vertices, int numVertices, int* indices, int numIndices)
{
	
	return 1;
}

void ngMeshDraw(ngMesh* mesh)
{
	
}

