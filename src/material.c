#include "gl_glfw_include.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "material.h"

void material_create(material_t* material, const char* filename) {
    int width;
    int height;
    int nb_channels;
    unsigned char* image_data = stbi_load(filename, &width, &height, &nb_channels, 0);

    glGenTextures(1, &material->texture);
    glBindTexture(GL_TEXTURE_2D, material->texture);

    if (!image_data) {
        (void)fprintf(stderr, "%s\n", "Failed to load texture image from disk");
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     image_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    stbi_image_free(image_data);

    /* Setting the texture parameters so that the texture repeats and
     * no linear interpolation is used to smooth out the textures. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    /* Generating mipmap levels for LOD. No need for now. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
}

void material_use(const material_t* material, const int unit) {
    glActiveTexture(GL_TEXTURE0 + (GLenum)unit);
    glBindTexture(GL_TEXTURE_2D, material->texture);
}

void material_destroy(const material_t* material) {
    glDeleteTextures(1, &material->texture);
}
