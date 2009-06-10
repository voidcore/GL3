#ifndef _NG_CORE_H_
#define _NG_CORE_H_

#ifdef __cplusplus
#  define NG_EXTERN_C_BEGIN extern "C" {
#  define NG_EXTERN_C_END   }
#else
#  define NG_EXTERN_C_BEGIN
#  define NG_EXTERN_C_END
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define NG_SNPRINTF _snprintf
#else
#include <stdbool.h>
#define NG_SNPRINTF snprintf
#endif
#include <stdlib.h>



// VBO helper macro
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#ifdef __APPLE__
#define glGenVertexArrays(num, id) glGenVertexArraysAPPLE(num, id)
#define glBindVertexArray(id) glBindVertexArrayAPPLE(id)
#define glGenFramebuffers(n, framebuffers) glGenFramebuffersEXT(n, framebuffers)
#define glBindFramebuffer(target, buffer) glBindFramebufferEXT(target, buffer)
#define glBindRenderbuffer(target, buffer) glBindRenderbufferEXT(target, buffer)
#define glFramebufferTexture2D(target, attachment, textureTarget,textureId, level)\
glFramebufferTexture2DEXT(target, attachment, textureTarget, textureId, level)

#endif

// Shader helper macro
#define TO_STRING(str) #str
#define GLSL_COMPAT_STRING "#version 120\n#extension GL_EXT_gpu_shader4 : enable\n"
// Matrix index macro
#define MAT4_INDEX(i,j) (j*4+i)
#define DEG2RAD(a) (a*3.141592653589793/180.0)


#ifdef __cplusplus
extern "C" {
#endif
	
	/* Types, enums and structs */
	typedef int				ngInt;
	typedef float			ngFloat;
	typedef double			ngDouble;
	typedef unsigned int	ngUInt;
	typedef unsigned short  ngUShort;
	typedef int				ngSize;
	typedef int				ngBool;
	
#ifdef __cplusplus
}
#endif


#endif // _NG_CORE_H_
