#ifndef RW_H_
#define RW_H_

#include "lights.h"
#include "handler.h"
#include "taskobject.h"
#include "npc.h"

#define COMMENT_MARKER	'#'
#define NEW_MARKER		'_'
#define POS_MARKER		'>'

void SetNpcPointers(Salesman *sm, Pharmacist *pm);

void DataRead(LightHandler *lh, Handler *obj_h);
void DataWrite(LightHandler *lh, Handler *obj_h);

#endif // !RW_H_
