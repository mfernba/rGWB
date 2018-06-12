// Definición de nodo para poder hacer casting...

#ifndef csmnode_tli
#define csmnode_tli

enum csmnode_object_t
{
    CSMNODE_OBJECT_csmloop_t,
    CSMNODE_OBJECT_csmhedge_t
};

struct csmnode_t;
typedef void (*csmnode_FPtr_free)(struct csmnode_t **object);
#define CSMNODE_CHECK_FUNC_FREE(funcion, tipo) ((void(*)(struct tipo **))funcion == funcion)

struct csmnode_t
{
    unsigned long id;

    enum csmnode_object_t real_type;
    csmnode_FPtr_free func_free;

    struct csmnode_t *prev, *next;
};

#define CSMNODE(variable) ((struct csmnode_t *)(variable))

#endif /* csmnode_tli */
