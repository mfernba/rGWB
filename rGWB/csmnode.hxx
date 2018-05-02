// Definición de nodo para poder hacer casting...

#ifndef csmnode_tli
#define csmnode_tli

struct csmnode_derivada_t;
typedef void (*csmnode_FPtr_destruye)(struct csmnode_derivada_t **clase_derivada);
#define CSMNODE_COMPRUEBA_FUNC_DESTRUYE(funcion, tipo) ((void(*)(struct tipo **))funcion == funcion)

struct csmnode_t
{
    unsigned long id;

    struct csmnode_derivada_t *clase_derivada;
    char *tipo_clase_derivada;
    csmnode_FPtr_destruye func_destruye_clase_derivada;

    struct csmnode_t *prev, *next;
};

#define CSMNODE(variable) ((struct csmnode_t *)(variable))

#endif /* csmnode_tli */
