#include <stdlib.h>
#include <mesh.h>

void mesh_free(mesh_t *m) {
	free(m->vertices);
	free(m->indices);
}
