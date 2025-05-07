#ifndef TASKOBJECT_H_
#define TASKOBJECT_H_

#include <stdint.h>
#include "raylib.h"

#define TASK_OBJ_ACTIVE 	0x01
#define TASK_OBJ_DONE		0x02

enum TASKOBJECT_TYPES : uint8_t {
	SPILL		  = 0,
	GRAFFITI	  = 1,
	TRASH		  = 2,
	TOILET		  = 3
};

typedef struct {
	uint8_t type;
	uint8_t flags;

	Vector3 position;
	BoundingBox bounds;

	Model model;

	Texture clean_tex;
	Texture dirty_tex;
} TaskObject;

TaskObject MakeTaskObject(uint8_t type, Vector3 position, Model model);
void CloseTaskObject(TaskObject *obj);

void TaskOjbectUpdate(TaskObject *obj);
void TaskObjectDraw(TaskObject *obj);

void TaskObjectInteract(TaskObject *obj);

#endif // !TASKOBJECT_H_
