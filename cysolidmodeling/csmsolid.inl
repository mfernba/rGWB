// Sólido euleriano...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmsolid_t *, csmsolid_crea_vacio, (void));

CONSTRUCTOR(struct csmsolid_t *, csmsolid_duplicate, (const struct csmsolid_t *solid));

void csmsolid_destruye(struct csmsolid_t **solido);


unsigned long *csmsolid_id_new_element(struct csmsolid_t *solido);

void csmsolid_append_new_face(struct csmsolid_t *solido, struct csmface_t **face);

void csmsolid_append_new_edge(struct csmsolid_t *solido, struct csmedge_t **edge);

void csmsolid_append_new_vertex(struct csmsolid_t *solido, double x, double y, double z, struct csmvertex_t **vertex);


void csmsolid_remove_face(struct csmsolid_t *solido, struct csmface_t **face);

void csmsolid_remove_edge(struct csmsolid_t *solido, struct csmedge_t **edge);

void csmsolid_remove_vertex(struct csmsolid_t *solido, struct csmvertex_t **vertex);


void csmsolid_print_debug(struct csmsolid_t *solido, CYBOOL assert_si_no_es_integro);
