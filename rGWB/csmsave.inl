// Write / read to file or other sources...

#include "csmfwddecl.hxx"

CONSTRUCTOR(struct csmsave_t *, csmsave_new_file_writer, (const char *file_path));

CONSTRUCTOR(struct csmsave_t *, csmsave_new_file_reader, (const char *file_path));

void csmsave_free(struct csmsave_t **csmsave);


// Write...

void csmsave_write_block_separator(struct csmsave_t *csmsave);

void csmsave_write_uchar(struct csmsave_t *csmsave, unsigned char value);

void csmsave_write_ulong(struct csmsave_t *csmsave, unsigned long value);

void csmsave_write_double(struct csmsave_t *csmsave, double value);

void csmsave_write_char(struct csmsave_t *csmsave, const char *value);


// Read...

void csmsave_read_block_separator(struct csmsave_t *csmsave);

unsigned char csmsave_read_uchar(struct csmsave_t *csmsave);

unsigned long csmsave_read_ulong(struct csmsave_t *csmsave);

double csmsave_read_double(struct csmsave_t *csmsave);

CONSTRUCTOR(char *, csmsave_read_char, (struct csmsave_t *csmsave));
