/**
 * Struct passed to the "kernel" from the exit_bs loader
 */
typedef struct {
    unsigned int    *framebuffer;
    unsigned int    width;
    unsigned int    height;
    unsigned int    pitch;
    int             argc;
    char            **argv;
} bootparam_t;
