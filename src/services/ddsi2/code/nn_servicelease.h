#ifndef NN_SERVICELEASE_H
#define NN_SERVICELEASE_H

#include "u_participant.h"

struct nn_servicelease;

struct nn_servicelease *nn_servicelease_new (u_participant participant);
int nn_servicelease_start_renewing (struct nn_servicelease *sl);
void nn_servicelease_free (struct nn_servicelease *sl);

#endif /* NN_SERVICELEASE_H */
