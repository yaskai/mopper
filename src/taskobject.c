#include "lights.h"
#include "raylib.h"
#include "taskobject.h"
#include "raymath.h"

TaskObject MakeTaskObject(uint8_t type, Vector3 position, Model model) {
	return (TaskObject) {
		.type = type, 
		.flags = (TASK_OBJ_ACTIVE),
		.position = position,
		.model = model
	};
}

void CloseTaskObject(TaskObject *obj) {
	UnloadModel(obj->model);
}

void TaskOjbectUpdate(TaskObject *obj) {
	obj->bounds = (BoundingBox) {
		.min = Vector3Subtract(obj->position, (Vector3){0.5f, 0.5f, 0.5f}),
		.max = Vector3Add(obj->position, (Vector3){0.5f, 0.5f, 0.5f})
	};
}

void TaskObjectInteract(TaskObject *obj) {
	switch(obj->type) {
		case SPILL: 
			break;

		case GRAFFITI:
			break;

		case TRASH:
			break;

		case TOILET:
			break;
	}
}
