/******************************************************************************/
/**
 
 Example of opengl 3.1 style usage implmented using opengl 2.1 + Extensions for 
 mac.
 
 Created: 2009-05-16
 Modified: 2009-05-22
 
 Original author: filip.wanstrom _at_ gmail.com
 
 Please feel free to use for any purpose. Preferably, add to this so we get a
 best practices example.
 
 **/

/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GLUT/glut.h>

#pragma mark Macros

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

#pragma mark Globals

// Globals
int             gLastFrameTime;
float   gRotation;
GLuint  gVBO;
GLuint  gEBO;
GLuint  gVAO;
GLuint  gFBO;
GLuint  gFBOColorTexture;
GLuint  gFBOColorTexture1;
GLuint  gFBODepthTexture;
GLuint  gDummyTex;
GLuint  gShader;
GLuint  gPostShader;
GLuint  gQuadVBO;
GLuint  gQuadVAO;
GLuint  gWindowWidth;
GLuint  gWindowHeight;
GLuint  gBufferWidth = 1024;
GLuint  gBufferHeight = 1024;
GLuint  gWindowHasBeenResized = 0;
double  gProjectionMatrix[16];
float   gProjectionMatrixf[16];
float   gOrthoProjectionMatrixf[16];
double  gModelViewMatrix[16];
float  gModelViewMatrixf[16];

#pragma mark Shaders source

/* Shader source */
/******************************************************************************/

// Basic shader

const GLchar* basicVertexShaderSource =
GLSL_COMPAT_STRING
TO_STRING(
attribute vec4  inPosition;
uniform mat4    inProjectionMatrix;
uniform mat4    inModelViewMatrix;
void main()
{
    gl_Position = inProjectionMatrix * inModelViewMatrix* inPosition;
}
);

const GLchar* basicFragmentShaderSource =
GLSL_COMPAT_STRING
TO_STRING(
varying out vec4 outColor1;
varying out vec4 outColor0;

void main()
{
    outColor0 = vec4(0.0, 1.0, 0.0, 1.0);
    outColor1 = vec4(0.0, 0.0, 1.0, 1.0);
}
);

// Post processing shader
const GLchar* postVertexShaderSource =
GLSL_COMPAT_STRING
TO_STRING(
attribute vec4 inPosition;
attribute vec4 inTexCoords;
varying vec4 texCoords;
uniform mat4    inProjectionMatrix;
uniform mat4    inModelViewMatrix;

void main()
{
    texCoords = inTexCoords;
    gl_Position = inProjectionMatrix * inPosition;
}
);

const GLchar* postFragmentShaderSource =
GLSL_COMPAT_STRING
TO_STRING(
uniform sampler2D tex;
uniform sampler2D texDepth;
varying vec4 texCoords;
varying out vec4 outColor;
void main()
{
    vec4 texcoordsColor = vec4(texCoords.xyz, 1);
    
    
    outColor = mix(vec4(texture2D(tex, texCoords.xy).rgb,
                         1.0),texcoordsColor,0.5) ;
    
}
);


/* Geometry data */
/******************************************************************************/

#pragma mark Geometry data

#define _A 0.525731112119133606f
#define _B 0.850650808352039932f
#define NUM_INDICES 20
#define NUM_VERTICES 12

static unsigned short icosahedronVertices[NUM_INDICES][3]=
{
{0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
{8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
{7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
{6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
};

static float icosahedronIndices[NUM_VERTICES][3]=
{
{-_A,0.0,_B},{_A,0.0,_B},{-_A,0.0,-_B},{_A,0.0,-_B},
{0.0,_B,_A},{0.0,_B,-_A},{0.0,-_B,_A},{0.0,-_B,-_A},
{_B,_A,0.0},{-_B,_A,0.0},{_B,-_A,0.0},{-_B,-_A,0.0}
};

#define QUAD_W 200.0f
#define QUAD_H 200.0f
static float quadVertices[4][3] =
{
{0, 0, 0.0f},
{0, QUAD_H, 0.0f},
{QUAD_W, 0, 0.0f},
{QUAD_W, QUAD_H, 0.0f}
};
static float quadTexcoords[4][3] =
{
{0, 0, 0.0f},
{0, 1, 0.0f},
{1, 0, 0.0f},
{1, 1, 0.0f}
};

#pragma mark Math code

/******************************************************************************/

void matrixIdentity(double A[16])
{
    size_t bytes = 16*sizeof(double);

    printf("bytes: %d\n", bytes );
    bzero(A, bytes);
    A[0] = A[5] = A[10] = A[15] = 1.0;
    
}
void matrixTranslate(double A[16], double tx, double ty, double tz)
{
    bzero(A, 16*sizeof(double));
    A[0] = A[5] = A[10] = A[15] = 1.0;
    A[12] = tx;
    A[13] = ty;
    A[14] = tz;
    
}
void matrixRotateZ(double A[16], double angle)
{
    double angled = DEG2RAD(angle);
    bzero(A, 16*sizeof(double));
    A[10] = A[15] = 1.0;
    
    A[0] = cos(angled);
    A[1] = sin(angled);
    A[4] = -sin(angled);
    A[5] = cos(angled);
    
}


void matrixMultiply(double A[16], double B[16], double C[16])
{
    int i, j, k;
	for ( i=0;i<4;i++ ) {
		for ( j=0;j<4;j++ ) {
			C[MAT4_INDEX(i,j)] =  A[MAT4_INDEX(i,0)] * B[MAT4_INDEX(0,j)];
			for (k=1;k<4;k++) {
				C[MAT4_INDEX(i,j)] += A[MAT4_INDEX(i,k)] * B[MAT4_INDEX(k,j)];
			}
            
		}
	}
}



void matrixPrint(double mat[16])
{
    int i, j;
    for (j=0;j<4;j++) {
        for (i=0;i<4;i++) {
            printf("%4.2f ", mat[MAT4_INDEX(i,j)]);        
        }
        printf("\n");
    }
    
}

void matrixfPrint(float mat[16])
{
    int i, j;
    for (j=0;j<4;j++) {
        for (i=0;i<4;i++) {
            printf("%4.2f ", mat[MAT4_INDEX(i,j)]);        
        }
        printf("\n");
    }
    
}


#pragma mark Utility code

/******************************************************************************/
GLuint utilCompileShader(GLenum shaderType, const GLchar* source)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        printf("shader compile error: %s ", source );
        return 0;
    }
    return shader;
}

/******************************************************************************/

void utilCheckFramebufferStatus()
{
    GLenum status;
    status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    printf("checking FBO status\n");
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            printf("Unsupported framebuffer format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            printf("Framebuffer incomplete attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, missing attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            printf("Framebuffer incomplete, attached images must have same dimensions\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            printf("Framebuffer incomplete, attached images must have same format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            printf("Framebuffer incomplete, missing draw buffer\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            printf("Framebuffer incomplete, missing read buffer\n");
            break;
        default:
            printf("Unknown error %d\n", status);
    }
}

/******************************************************************************/
void utilCreateFrustum( GLdouble        left,
                       GLdouble     right,
                       GLdouble     bottom,
                       GLdouble     top,
                       GLdouble     nearVal,
                       GLdouble     farVal,
                       GLdouble             *mat)
{
    double A = (right+left)/(right-left);
    double B = (top+bottom)/(top-bottom);
    double C = (farVal+nearVal)/(farVal-nearVal);
    double D = (2.0*farVal*nearVal)/(farVal-nearVal);
    int i;
    for (i=0;i<16;i++) { mat[i] = 0.0; }
    
    mat[0] = (2.0*nearVal)/(right-left);
    mat[5] = (2.0*nearVal)/(top-bottom);
    mat[8] = A;
    mat[9] = B;
    mat[10] = C;
    mat[11] = -1.0;
    mat[14] = D;
}

/******************************************************************************/

void utilReshape(GLuint width, GLuint height)
{
    int i;
    if (width ==0)
        width = 1;
    
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    GLfloat h = (GLfloat) height / (GLfloat) width;
    glViewport(0, 0, (GLint) width, (GLint) height);
    
    utilCreateFrustum(-1.0f*nearPlane,
                      nearPlane,
                      -h*nearPlane,
                      h*nearPlane,
                      nearPlane,
                      farPlane,
                      gProjectionMatrix);
    
    for(i=0;i<16;i++) { gProjectionMatrixf[i] = gProjectionMatrix[i];}

}

/******************************************************************************/
void utilCreateOrtho(float w, float h, GLdouble mat[16])
{
    GLdouble        left = 0;
    GLdouble        right = w;
    GLdouble        bottom = 0;
    GLdouble        top = h;
    GLdouble        near = -1.0;
    GLdouble        far = 1.0;
    
    GLdouble tx = -(right+left)/(right-left);
    GLdouble ty = -(top+bottom)/(top-bottom);
    GLdouble tz = -(far+near)/(far-near);
    
    int i=0;
    for (i=0;i<16;i++) { mat[i] = 0.0;}
    
    
    mat[0] = (2.0/(right-left));
    mat[5] = (2.0/(top-bottom));
    mat[10] = -(2.0/(far-near));
    mat[12] = tx;
    mat[13] = ty;
    mat[14] = tz;
    mat[15] = 1.0;
    
}

/******************************************************************************/

void utilReshapeOrtho(GLuint width, GLuint height)
{
    if (width ==0)
        width = 1;
    
    double mat[16];
    int i;
    glViewport(0, 0, (GLint) width, (GLint) height);
    utilCreateOrtho(width, height, mat);
    for (i=0;i<16;i++) {gOrthoProjectionMatrixf[i] = mat[i]; }
}


#pragma mark Buffer and shader setup

/******************************************************************************/

void createDummyTex()
{
    unsigned char data[256*256*4];
    int x, y;
    for(y=0; y<256; y++)
    {
        for(x=0; x<256; x++)
        {
            int c = 127 + ((((y*8)/256)+((x*8)/256))%2) * 128;
            
            data[4*(y*256+x) + 0] = c;
            data[4*(y*256+x) + 1] = c;
            data[4*(y*256+x) + 2] = c;
            data[4*(y*256+x) + 3] = 255;
        }
    }
    
    glGenTextures(1, &gDummyTex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gDummyTex);
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  256, 256, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
    
}

/******************************************************************************/

void createShaders()
{
    GLint status;
    
    // The basic shader for the geometry
    gShader = glCreateProgram();
    GLuint vertexShader= utilCompileShader(GL_VERTEX_SHADER,
                                           basicVertexShaderSource);
    GLuint fragmentShader = utilCompileShader(GL_FRAGMENT_SHADER,
                                              basicFragmentShaderSource);
    
    glAttachShader(gShader, vertexShader);
    glAttachShader(gShader, fragmentShader);
    
    glBindFragDataLocationEXT(gShader, 0, "outColor0"); 
    glBindFragDataLocationEXT(gShader, 1, "outColor1"); 
    
    glLinkProgram(gShader);
    
    
   
    
    glGetProgramiv(gShader, GL_LINK_STATUS, &status);
    if (!status) {
        printf("program link error\n");
    }
    
    // The post processing shader
    gPostShader = glCreateProgram();
    vertexShader= utilCompileShader(GL_VERTEX_SHADER, postVertexShaderSource);
    fragmentShader = utilCompileShader(GL_FRAGMENT_SHADER,
                                       postFragmentShaderSource);
    
    glAttachShader(gPostShader, vertexShader);
    glAttachShader(gPostShader, fragmentShader);
    glLinkProgram(gPostShader);
    
    
    glGetProgramiv(gPostShader, GL_LINK_STATUS, &status);
    if (!status) {
        printf("program link error\n");
    }
}

/******************************************************************************/

void createFBO()
{
    GLuint w = gBufferWidth;
    GLuint h = gBufferHeight;
    glGenFramebuffersEXT(1, &gFBO);
    glGenTextures(1, &gFBOColorTexture);
    glGenTextures(1, &gFBOColorTexture1);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gFBO);
    
    // Depth
    glBindTexture(GL_TEXTURE_2D, gFBODepthTexture);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
                 GL_DEPTH_COMPONENT, GL_INT, NULL);
    
    glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT,
                               GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, gFBODepthTexture, 0);
    
    // Color 0
    glBindTexture(GL_TEXTURE_2D, gFBOColorTexture);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, w, h, 0, GL_RGBA,
                 GL_FLOAT, NULL);
    
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, gFBOColorTexture, 0);
    
    // Color 1
    glBindTexture(GL_TEXTURE_2D, gFBOColorTexture1);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, w, h, 0, GL_RGBA,
                 GL_FLOAT, NULL);
    
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, gFBOColorTexture1, 0);
    
    utilCheckFramebufferStatus();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

/******************************************************************************/

void createVertexBuffers()
{
    int sizeInBytes = 0;
    
    // Indexed icosaderon buffers
    
    glGenVertexArrays(1, &gVAO);
    glBindVertexArray(gVAO);
    
    sizeInBytes = sizeof(float)*3*NUM_VERTICES;
    glGenBuffers(1, &gVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, icosahedronIndices);
    
    GLuint positionLoc = glGetAttribLocation(gShader, "inPosition");
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLoc);
    
    sizeInBytes = sizeof(unsigned short)*3*NUM_INDICES;
    glGenBuffers(1, &gEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes,NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeInBytes, icosahedronVertices);
    
    glBindVertexArray(0);
    
    // Quad buffers (Using tri strips)
    
    glGenVertexArrays(1, &gQuadVAO);
    glBindVertexArray(gQuadVAO);
    
    sizeInBytes = sizeof(float)*3*4;
    glGenBuffers(1, &gQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes*2, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, quadVertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeInBytes, sizeInBytes, quadTexcoords);
    
    positionLoc = glGetAttribLocation(gPostShader, "inPosition");
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLoc);
    
    GLuint texcoordLoc = glGetAttribLocation(gPostShader, "inTexCoords");
    glVertexAttribPointer(texcoordLoc, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(48));
    glEnableVertexAttribArray(texcoordLoc);
    
    GLuint texLoc  = glGetAttribLocation(gPostShader, "tex");
    glUniform1i(texLoc, 0);
    
    GLuint texLoc2  = glGetAttribLocation(gPostShader, "texDepth");
    glUniform1i(texLoc2, 1);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gFBOColorTexture);
    
    
    glBindVertexArray(0);
    
}

#pragma mark Drawing code

/******************************************************************************/

void drawUsingVAO()
{
    glUseProgram(gShader);
    int i;
    double test[16];
    
    GLuint projMatrixLoc = glGetUniformLocation(gShader, "inProjectionMatrix");
    glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, gProjectionMatrixf);
    
    GLuint mvMatrixLoc = glGetUniformLocation(gShader, "inModelViewMatrix");
    glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, gModelViewMatrixf);
    
 
    
    glBindVertexArray(gVAO);
    glDrawElements(GL_TRIANGLES, NUM_INDICES*3, GL_UNSIGNED_SHORT,
                   BUFFER_OFFSET(0));
    glBindVertexArray(0);
}

/******************************************************************************/
void drawQuadUsingVAO()
{
    if (gWindowHasBeenResized) {
        // update size of quad to fill whole screen
        float w = gWindowWidth;
        float h = gWindowHeight;
        float quadVertices[4][3] =
        {
            {0, 0, 0.0f},
            {0, h, 0.0f},
            {w, 0, 0.0f},
            {w, h, 0.0f}
        };
        int sizeInBytes = sizeof(float)*3*4;
        
        glBindBuffer(GL_ARRAY_BUFFER, gQuadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, quadVertices);
        gWindowHasBeenResized = 0;
    }
    
    glUseProgram(gPostShader);
    
    GLuint projMatrixLoc = glGetUniformLocation(gPostShader, "inProjectionMatrix");
    glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, gOrthoProjectionMatrixf);
    
    glBindVertexArray(gQuadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gFBOColorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gFBODepthTexture);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4*3);
    glBindVertexArray(0);
}

#pragma mark Callbacks

/******************************************************************************/
static void init()
{
   utilReshape(gBufferWidth, gBufferHeight); 
}

/******************************************************************************/

static void update()
{
    double T[16];
    double R[16];
    int i;
    static int frame = 1;
    
    if (frame==1) { 
        init(); 
        frame++;
    }
    // Update
    if (gLastFrameTime == 0)
    {
        gLastFrameTime = glutGet(GLUT_ELAPSED_TIME);
    }
    
    int now = glutGet(GLUT_ELAPSED_TIME);
    int elapsedMilliseconds = now - gLastFrameTime;
    float elapsedTime = elapsedMilliseconds / 1000.0f;
    gLastFrameTime = now;
    gRotation+=elapsedTime*30.0f;    
    
    // transform the geometry
    matrixTranslate(T , 0, 0, -10);
    matrixRotateZ(R, gRotation);
    matrixMultiply(T, R, gModelViewMatrix);
    
    // copy doubles to float
    for(i=0;i<16;i++) { gModelViewMatrixf[i] = gModelViewMatrix[i];}
    
}

/******************************************************************************/

void cbDisplay(void)
{
    GLenum output[2] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT}; 
    update();
    // Draw geometry to FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gFBO);
    glDrawBuffers(2, output);
    glViewport(0, 0, gBufferWidth, gBufferHeight);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawUsingVAO();
    
    // Draw fbo to back buffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    utilReshapeOrtho(gWindowWidth, gWindowHeight);
    drawQuadUsingVAO();
    
    glutSwapBuffers();
}

/******************************************************************************/

void cbReshape(int width, int height)
{
    gWindowWidth = width;
    gWindowHeight = height;
    gWindowHasBeenResized = 1;
    
}

/******************************************************************************/

void cbIdle(void)
{
    glutPostRedisplay();
}

#pragma mark Entry point

/******************************************************************************/
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    
    glutCreateWindow("GL3 Tutorial 2");
    
    createDummyTex();
    createFBO();
    createShaders();
    createVertexBuffers();
    
    glutDisplayFunc(cbDisplay);
    glutReshapeFunc(cbReshape);
    glutIdleFunc(cbIdle);
    
    glutMainLoop();
    return EXIT_SUCCESS;
}
/******************************************************************************/