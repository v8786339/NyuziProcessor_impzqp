/*
 * SceneView.h
 *
 *      Course: SoC Design Laboratoy, 2017W
 *      Author: Jonas Ferdigg (e1226597), Markus Kessler (e1225380)
 *      Mail:   [matrNr]@student.tuwien.ac.at
 */

#ifndef __SCENE_VIEW_H
#define __SCENE_VIEW_H

#include <nyuzi.h>
#include <RenderContext.h>
#include <schedule.h>
#include <stdio.h>
#include <time.h>
#include <vga.h>
#include <Surface.h>
#include "DepthShader.h"
#include "schedule.h"
#include "TextureShader.h"
#include "shared_mem_itf.h"
#include <float.h>

struct FileHeader {
    uint32_t fileSize;
    uint32_t numTextures;
    uint32_t numMeshes;
};

struct TextureEntry {
    uint32_t offset;
    uint32_t mipLevels;
    uint16_t width;
    uint16_t height;
};

struct MeshEntry {
    uint32_t offset;
    uint32_t textureId;
    uint32_t numVertices;
    uint32_t numIndices;
};

struct LookAtArguments {
    Vec3 vLocation;
    Vec3 vLookAt;
    Vec3 vUp;
};

typedef enum {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP_LEFT,
    DIR_UP_RIGHT,
    DIR_DOWN_LEFT,
    DIR_DOWN_RIGHT
} dir_t;

typedef enum {
    FB_NULL = 0,
    FB_1,
    FB_2,
    FB_3
} fb_t;

class SceneView {
public:
    SceneView();
    void start();
    uint64_t render();
    void stop();
    void rotate(dir_t dir_horizontal, dir_t dir_vertical, float delta);
    void move(dir_t dir, float step_size);
    void turn(dir_t dir, float angle);
    void resetCamera();
    
private:
    char *readResourceFile();   // TODO remove prefix for variables
    int kTestTextureSize;
    int kAttrsPerVertex;
    int kCheckerSize;
    void *pFrameBuffer1;
    void *pFrameBuffer2;
    void *pFrameBuffer3;
    char *resource_file;            // Address of the resource file
    FileHeader *resource_header;
    TextureEntry *texture_header;
    MeshEntry *mesh_header;
    Texture **textures;
    RenderBuffer *vertexBuffers;
    RenderBuffer *indexBuffers;
    RenderContext *context;
    RenderTarget *renderTarget;
    Matrix modelViewMatrix;
    Surface *depthBuffer;        
    Surface *colorBuffer1;
    Surface *colorBuffer2;
    Surface *colorBuffer3;
    Matrix projectionMatrix;
    TextureUniforms uniforms;
    float fTheta;
    float fPsi;
    dir_t eRotateDirection;
    LookAtArguments lookAtArgs;
    float fXmin = FLT_MAX;
    float fYmin = FLT_MAX;
    float fZmin = FLT_MAX;
    float fXmax = FLT_MIN;
    float fYmax = FLT_MIN;
    float fZmax = FLT_MIN;
    fb_t eCurrentRenderFB = FB_1;
};

#endif


