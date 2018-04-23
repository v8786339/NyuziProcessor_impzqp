//
// Copyright 2011-2015 Jeff Bush
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "SceneView.h"

#define ROTATION_SPEED M_PI/16
#define MOVEMENT_SPEED 1.0

smdb_t *smdb;

// PROTOTYPES

void invalidate_cache(uint8_t *start_address, size_t size);

/**
 * @brief   Main entry point of the program
 */
int main() {
    if (get_current_thread_id() != 0) {
        worker_thread();
    }

    // Shared memory database
    smdb = (smdb_t *)SMDB_ADDR;
    invalidate_cache((uint8_t *)smdb, sizeof(smdb_t));

    SceneView *sv = new SceneView();
    sv->start();
    start_all_threads();

    // If this is set, the camera rotates around the object.
    bool rotation_mode = false;
    bool first_move = true;

    for (int frame = 0; ; frame++) {
        // Render next frame
        smdb->frame_rate = sv->render();
        __asm("dflush %0" : : "r" (&smdb->frame_rate));

        // Update next camera action
        //invalidate_cache((uint8_t *)smdb, sizeof(smdb_t));

        // Movement and rotation variables
        dir_t rotation_dir_vertical     = DIR_NONE;
        dir_t rotation_dir_horizontal   = DIR_NONE;

        __asm("dinvalidate %0" : : "r" (&smdb->camera_actions));
        __asm("membar" : : );
        uint32_t actions = smdb->camera_actions;

        if (actions & (1L << ACTION_BIT_ROTATE)) {
            rotation_mode = true;
            if (actions & (1L << ACTION_BIT_LEFT)) {
                rotation_dir_horizontal = DIR_LEFT;
            }
            else if (actions & (1L << ACTION_BIT_RIGHT)) {
                rotation_dir_horizontal = DIR_RIGHT;
            }
            if (actions & (1L << ACTION_BIT_UP)) {
                rotation_dir_vertical = DIR_UP;
            }
            else if (actions & (1L << ACTION_BIT_DOWN)) {
                rotation_dir_vertical = DIR_DOWN;
            }
            // Rotate according to rotation direction
            sv->rotate(rotation_dir_horizontal, rotation_dir_vertical, ROTATION_SPEED);
        }
        else {
            if (rotation_mode) {
                // Reset camera position
                sv->resetCamera();
                printf("Reset cam\n\r");
                rotation_mode = false;
            }
            if (actions & (1L << ACTION_BIT_LEFT)) {
                sv->turn(DIR_LEFT, ROTATION_SPEED);
                first_move = false;
            }
            else if (actions & (1L << ACTION_BIT_RIGHT)) {
                sv->turn(DIR_RIGHT, ROTATION_SPEED);
                first_move = false;
            }
            if (actions & (1L << ACTION_BIT_UP)) {
                if (first_move) {
                    sv->turn(DIR_LEFT, 0);
                    first_move = false;
                }
                sv->move(DIR_UP, MOVEMENT_SPEED);
            }
            else if (actions & (1L << ACTION_BIT_DOWN)) {
                if (first_move) {
                    sv->turn(DIR_LEFT, 0);
                    first_move = false;
                }
                sv->move(DIR_DOWN, MOVEMENT_SPEED);
            }
        }

        if (frame % 10 == 0) {
            printf("F: %d\n\r", (uint32_t)actions);
        }
    }
    return 0;
}

/**
 * @brief                   Invalidates the cache for a specified memory area
 * @param start_address     defines the start address
 * @param size              is the number of bytes that should be invalidated
 */
void invalidate_cache(uint8_t *start_address, size_t size) {
    for (uint32_t i = 0; i < size; i += 4) {
        __asm("dinvalidate %0" : : "r" (start_address + i));
    }
    __asm("membar" : : );
}

