#ifndef MATERIAL_H
#define MATERIAL_H

/**
 * @brief Structure storing the important info about a material. For now,
 * this is simply a texture. */
typedef struct {
        unsigned int texture;
} material_t;

/**
 * @brief Creates an OpenGL texture using an image and assigns its id to
 * the material attribute.
 * @param material Pointer to the material which will be modified.
 * @param filename Path to the texture file.*/
void material_create(material_t* material, const char* filename);

/**
 * @brief Makes the necessary calls so that the material is used.
 * @param material Pointer to the material which will be used.
 * @param unit Number of the unit of the texture.*/
void material_use(material_t* material, int unit);

/**
 * @brief Destroys the allocation OpenGL material.
 * @param material Pointer to the material to be destroyed. */
void material_destroy(material_t* material);

#endif
