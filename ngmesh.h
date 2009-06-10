#ifndef _NG_MESH_H_
#define _NG_MESH_H_ 

#include "ngcore.h"

NG_EXTERN_C_BEGIN


struct _ngMesh {
	ngFloat*	vertices;
	ngUInt		numVertices;
	ngUInt*		indices;
	ngUInt		numIndices;
	
	ngUInt vbo;
	ngUInt ebo;
	ngUInt vao;
};
typedef struct _ngMesh ngMesh;


ngMesh*		ngMeshAlloc();
int			ngMeshInitFromArrays(ngMesh*	mesh, 
								 ngFloat*	vertices, 
								 ngInt		numVertices, 
								 ngInt*		indices, 
								 ngInt		numIndices);

void		ngMeshDraw(ngMesh* mesh);


NG_EXTERN_C_END

#endif //  _NG_MESH_H_