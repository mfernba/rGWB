//
//  csmmaterial.inl
//  rGWB
//
//  Created by Manuel Fernandez on 14/10/17.
//  Copyright © 2017 Manuel Fernández. All rights reserved.
//

CONSTRUCTOR(struct csmmaterial_t *, csmmaterial_copy, (const struct csmmaterial_t *material));

void csmmaterial_free(struct csmmaterial_t **material);

CONSTRUCTOR(struct csmmaterial_t *, csmmaterial_read, (struct csmsave_t *csmsave));

void csmmaterial_write(const struct csmmaterial_t *material, struct csmsave_t *csmsave);
