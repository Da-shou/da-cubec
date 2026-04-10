#include <pointer.h>
#include <math.h>

block_type_t get_pointed_block(chunk_t* chunk, camera_t* camera,
                               float max_distance, vec3* pointed_block,
                               vec3* neighbour_block) {
        vec3 pos, dir;
        glm_vec3_copy(camera->position, pos);
        glm_vec3_copy(camera->front, dir);

        axis_t last_pointed_to = X;

        /* Storing the camera position */
        int x, y, z;
        x = (int)floorf(pos[0]);
        y = (int)floorf(pos[1]);
        z = (int)floorf(pos[2]);

        /* Seeing which directions needs to step positively or negatively
         * based on the direction vector. */
        int step_x, step_y, step_z;
        step_x = (dir[0] > 0) ? 1 : -1;
        step_y = (dir[1] > 0) ? 1 : -1;
        step_z = (dir[2] > 0) ? 1 : -1;

        /* This tells us how far we need to travel along the raycast until
         * we reach the next x, y or z plane. The delta variables tells us
         * exactly how much we need to move along the ray path to cover ONE
         * unit of distance.*/
        float delta_x, delta_y, delta_z;
        delta_x = fabsf(1.0f / dir[0]);
        delta_y = fabsf(1.0f / dir[1]);
        delta_z = fabsf(1.0f / dir[2]);

        /* Then, we calculate the total distance the ray has to travel
         * starting from the camera to hit the very next vertical or
         * horizontal grid line.*/
        float side_x, side_y, side_z;
        side_x = (step_x > 0) ? (x + 1.0f - pos[0]) * delta_x
                              : (pos[0] - x) * delta_x;
        side_y = (step_y > 0) ? (y + 1.0f - pos[1]) * delta_y
                              : (pos[1] - y) * delta_y;
        side_z = (step_z > 0) ? (z + 1.0f - pos[2]) * delta_z
                              : (pos[2] - z) * delta_z;
        float distance = 0.0f;
        while (distance < max_distance) {
                bool boundary_check = x >= 0 && x < CHUNK_SIZE && y >= 0 &&
                                      y < CHUNK_SIZE && z >= 0 &&
                                      z < CHUNK_SIZE;
                if (boundary_check) {
                        block_type_t block = chunk->blocks[x][y][z];
                        if (block != BLOCK_AIR) {
                                process_block(
                                    (vec3) {x, y, z}, last_pointed_to,
                                    pointed_block, neighbour_block, step_x,
                                    step_y, step_z);
				return block;
                        }
                }

                if (side_x < side_y) {
                        if (side_x < side_z) {
                                side_x += delta_x;
                                x += step_x;
                                distance = side_x;
				last_pointed_to = X;
                        } else {
                                side_z += delta_z;
                                z += step_z;
                                distance = side_z;
				last_pointed_to = Z;
                        }
                } else {
                        if (side_y < side_z) {
                                side_y += delta_y;
                                y += step_y;
                                distance = side_y;
				last_pointed_to = Y;
                        } else {
                                side_z += delta_z;
                                z += step_z;
                                distance = side_z;
				last_pointed_to = Z;
                        }
                }
        }

        return BLOCK_AIR;
}

void process_block(vec3 block_position, axis_t last, vec3* pointed_block,
                   vec3* neighbour_block, int step_x, int step_y,
                   int step_z) {
        glm_vec3_copy(block_position, *pointed_block);
        glm_vec3_copy(*pointed_block, *neighbour_block);
        switch (last) {
        case X: (*neighbour_block)[0] -= step_x; break;
        case Y: (*neighbour_block)[1] -= step_y; break;
        case Z: (*neighbour_block)[2] -= step_z; break;
        }
}
