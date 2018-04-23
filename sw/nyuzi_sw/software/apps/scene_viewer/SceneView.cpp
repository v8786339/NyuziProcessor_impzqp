/*
 * SceneView.cpp - This file contains the implementation of the SceneView class.
 *
 *      Course: SoC Design Laboratoy, 2017W
 *      Author: Jonas Ferdigg (e1226597), Markus Kessler (e1225380)
 *      Mail:   [matrNr]@student.tuwien.ac.at
 */

#include "SceneView.h"

// Offset for distance to object from the camera
#define CAMERA_DISTANCE_OFFSET 6

float getFloatIndex(const void *ptr, unsigned int index) {
    return *((float *)(ptr) + index);
}

/**
 * @brief       The default constructor which initializes member variables
 */
SceneView::SceneView() {
    projectionMatrix            = Matrix::getProjectionMatrix(FB_WIDTH, FB_HEIGHT);
    uniforms.fLightDirection    = Vec3(-1, -0.5, 1).normalized();
    uniforms.fDirectional       = 0.5f;
    uniforms.fAmbient           = 1.0;
    kAttrsPerVertex             = 8;
    kTestTextureSize            = 128;
    kCheckerSize                = 32;
    eRotateDirection            = DIR_NONE;
    lookAtArgs.vLocation        = Vec3(0,0,0);
    lookAtArgs.vLookAt          = Vec3(0,0,0);
    lookAtArgs.vUp              = Vec3(0,1,0);
    // fTheta and fPsi should NOT be initalized with 0.0,
    // to prevent flickering while rotating the scene.
    fTheta  = 0.1;
    fPsi    = 0.1;
}

/**
 * @brief       Initializes the scene viewer by loading the resource data, mapping the frame buffers, 
 *              creating render target and binding it to the context.
 */
void SceneView::start() {
    // Set up resource data
    resource_file = readResourceFile();
    resource_header = (FileHeader*) resource_file;
    texture_header = (TextureEntry*)(resource_file + sizeof(FileHeader));
    mesh_header = (MeshEntry*)(resource_file + sizeof(FileHeader) + resource_header->numTextures
                                    * sizeof(TextureEntry));
    textures = new Texture*[resource_header->numTextures];

    // Create texture objects
    for (unsigned int textureIndex = 0; textureIndex < resource_header->numTextures; textureIndex++) {
        textures[textureIndex] = new Texture();
        textures[textureIndex]->enableBilinearFiltering(true);
        int offset = texture_header[textureIndex].offset;
        for (unsigned int mipLevel = 0; mipLevel < texture_header[textureIndex].mipLevels; mipLevel++) {
            int width = texture_header[textureIndex].width >> mipLevel;
            int height = texture_header[textureIndex].height >> mipLevel;
            Surface *surface = new Surface(width, height, resource_file + offset);
            textures[textureIndex]->setMipSurface(mipLevel, surface);
            offset += width * height * 4;
        }
    }

    // Create Render Buffers
    vertexBuffers = new RenderBuffer[resource_header->numMeshes];
    indexBuffers = new RenderBuffer[resource_header->numMeshes];

    for (unsigned int meshIndex = 0; meshIndex < resource_header->numMeshes; meshIndex++) {
        const MeshEntry &entry = mesh_header[meshIndex];
        vertexBuffers[meshIndex].setData(resource_file + entry.offset,
                                            entry.numVertices, sizeof(float) * kAttrsPerVertex);

        // Iterate over alle vertices in mesh to find lowest and highest x,y,z
        for (unsigned int i = 0; i < entry.numVertices; i++) {
            float x = getFloatIndex(vertexBuffers[meshIndex].getData(), 8*i + 0);
            float y = getFloatIndex(vertexBuffers[meshIndex].getData(), 8*i + 1);
            float z = getFloatIndex(vertexBuffers[meshIndex].getData(), 8*i + 2);
            if (x < fXmin) {
                fXmin = x;
            }
            if (x > fXmax) {
                fXmax = x;
            }
            if (y < fYmin) {
                fYmin = y;
            }
            if (y > fYmax) {
                fYmax = y;
            }
            if (z < fZmin) {
                fZmin = z;
            }
            if (z > fZmax) {
                fZmax = z;
            }
        }
        indexBuffers[meshIndex].setData(resource_file + entry.offset + entry.numVertices
                            * kAttrsPerVertex * sizeof(float), entry.numIndices, sizeof(int));
    }

    // Set the camera position and viewing direction
    resetCamera();

    // Set up tripple framebuffer
    pFrameBuffer1 = init_vga(VGA_MODE_640x480);
    pFrameBuffer2 = (uint32_t *) (FB_BASE2);
    pFrameBuffer3 = (uint32_t *) (FB_BASE3);    
    
    // Creeate the render target and bind it to the first framebuffer
    context      = new RenderContext(0x1000000); // TODO describe this address
    renderTarget = new RenderTarget();
    depthBuffer  = new Surface(FB_WIDTH, FB_HEIGHT);
    colorBuffer1 = new Surface(FB_WIDTH, FB_HEIGHT, pFrameBuffer1);
    colorBuffer2 = new Surface(FB_WIDTH, FB_HEIGHT, pFrameBuffer2);
    colorBuffer3 = new Surface(FB_WIDTH, FB_HEIGHT, pFrameBuffer3);

    renderTarget->setColorBuffer(colorBuffer1);
    renderTarget->setDepthBuffer(depthBuffer);
    context->bindTarget(renderTarget);
    context->enableDepthBuffer(true);
    context->bindShader(new TextureShader());
    context->setClearColor(0.52, 0.80, 0.98);
}

/**
 * @brief       Updates the member "resource_file" with the address value from the shared memory
 * @return      The new address of the resource file
 */
char *SceneView::readResourceFile() {
    smdb_t *smdb = (smdb_t *)SMDB_ADDR;
    resource_file = (char *)(smdb->resource_fil_st_addr);
    printf("Res: 0x%x\n\r", smdb->resource_fil_size);
    return resource_file;
}

/**
 * @brief       Start rendering the next frame
 * @return      The required time to render the frame
 */
uint64_t SceneView::render() {
    modelViewMatrix = Matrix::lookAt(lookAtArgs.vLocation, lookAtArgs.vLookAt, lookAtArgs.vUp);
    uniforms.fMVPMatrix = projectionMatrix * modelViewMatrix;
    uniforms.fNormalMatrix = modelViewMatrix.upper3x3();
    
    if (eCurrentRenderFB == FB_1) {
        switch_fb(FB_1);
        eCurrentRenderFB = FB_2;
        // Write to second framebuffer
        renderTarget->setColorBuffer(colorBuffer2);
    }
    else if (eCurrentRenderFB == FB_2) {
        switch_fb(FB_2);
        eCurrentRenderFB = FB_3;
        // Write to third framebuffer
        renderTarget->setColorBuffer(colorBuffer3);

    }
    else if (eCurrentRenderFB == FB_3) {
        switch_fb(FB_3);
        eCurrentRenderFB = FB_1;
        // Write to first framebuffer
        renderTarget->setColorBuffer(colorBuffer1);
    }
    context->clearColorBuffer();
    for (unsigned int meshIndex = 0; meshIndex < resource_header->numMeshes; meshIndex++) {
        const MeshEntry &entry = mesh_header[meshIndex];
        if (entry.textureId != 0xffffffff) {
            assert(entry.textureId < resource_header->numTextures);
            context->bindTexture(0, textures[entry.textureId]);
            uniforms.fHasTexture = true;
        }
        else {
            uniforms.fHasTexture = false;
        }

        context->bindUniforms(&uniforms, sizeof(uniforms));
        context->bindVertexAttrs(&vertexBuffers[meshIndex]);
        context->drawElements(&indexBuffers[meshIndex]);
    }

    clock_t startTime = clock();
    context->finish();
    return (uint64_t)(clock() - startTime);
}

/**
 * @brief                   Rotate the camera
 * @param dir_horizontal    defines the horizontal rotation direction
 * @param dir_vertical      defines the vertical rotation direction
 * @param delta             is the rotation step size
 */
void SceneView::rotate(dir_t dir_horizontal, dir_t dir_vertical, float delta) {
    // Add delta fTheta
    if (dir_horizontal == DIR_RIGHT) {
        this->fTheta += delta;
    }
    else if (dir_horizontal == DIR_LEFT) {
        this->fTheta -= delta;
    }
    // Adjust the fTheta object attribute to be within 0.0 and 2*Pi
    if (this->fTheta < 0.0) {
        this->fTheta += 2*M_PI;
    }
    if (this->fTheta >= 2*M_PI) {
        this->fTheta -= 2*M_PI;
    }
    // Add delta fPsi
    if (dir_vertical == DIR_UP) {
        this->fPsi += delta;
    }
    else if (dir_vertical == DIR_DOWN) {
        this->fPsi -= delta;
    }
    // Adjust the fPsi object attribute to be within 0.0 and 2*Pi
    if (this->fPsi < 0.0) {
        this->fPsi += 2*M_PI;
    }
    else if (this->fPsi >= 2*M_PI) {
        this->fPsi -= 2*M_PI;
    }
    // Calculate the new position of the camera according to the angles
    lookAtArgs.vLocation = Vec3(
        cos(this->fPsi) * sin(this->fTheta) * CAMERA_DISTANCE_OFFSET,
        sin(this->fPsi) * CAMERA_DISTANCE_OFFSET,
        cos(this->fPsi) * cos(this->fTheta) * CAMERA_DISTANCE_OFFSET
    );
    // Look at the spot where the object is placed
    lookAtArgs.vLookAt = Vec3(0, 0, 0);
    if (this->fPsi < M_PI/2 || this->fPsi >= 3*M_PI/2) {
        lookAtArgs.vUp = Vec3(0, 1, 0);
    } 
    else {
        lookAtArgs.vUp = Vec3(0, -1, 0); 
    }
}

/**
 * @brief           Moves the camera by specified step distance
 * @param dir       defines the moving direction
 * @param step_size defines the moving step size
 */
void SceneView::move(dir_t dir, float step_size) {
    // Move step_size either forwards or backwards in the direction of vLookAt
    if (dir == DIR_UP) {
        lookAtArgs.vLocation = lookAtArgs.vLocation + lookAtArgs.vLookAt.normalized() * step_size;
    }
    else if (dir == DIR_DOWN) {
        lookAtArgs.vLocation = lookAtArgs.vLocation - lookAtArgs.vLookAt.normalized() * step_size;
    }
}

/**
 * @brief           Turns the camera by a specified angle
 * @param dir       defines the turning direction
 * @param angle     defines the angle step size
 */
void SceneView::turn(dir_t dir, float angle) {
    // Add delta fTheta
    if (dir == DIR_RIGHT) {
        this->fTheta -= angle;
    }
    else if (dir == DIR_LEFT) {
        this->fTheta += angle;
    }
    // Adjust the fTheta object attribute to be within 0.0 and 2*Pi
    if (this->fTheta < 0.0) {
        this->fTheta += 2*M_PI;
    }
    else if (this->fTheta >= 2*M_PI) {
        this->fTheta -= 2*M_PI;
    }
    // Set the camera viewing direction
    lookAtArgs.vLookAt = Vec3(
        sin(this->fTheta)*1000,
        CAMERA_DISTANCE_OFFSET,
        cos(this->fTheta)*1000
    );
}

/**
 * @brief       Reset the camera locatin and looking direction
 */
void SceneView::resetCamera() {
    fTheta  = 0.1;
    fPsi    = 0.1;

    // Set the camera position and viewing direction
    lookAtArgs.vLocation = Vec3(CAMERA_DISTANCE_OFFSET, CAMERA_DISTANCE_OFFSET, 0);
    lookAtArgs.vLookAt = Vec3(0,0,0);
    lookAtArgs.vUp = Vec3(0, 1, 0);
}