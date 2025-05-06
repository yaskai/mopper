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

void TaskObjectDraw(TaskObject *obj) {
	if((obj->flags & TASK_OBJ_ACTIVE) == 0) return;
	
	DrawModelShaded(obj->model, obj->position);
	switch(obj->type) {
		case SPILL: 
			DrawModelShaded(obj->model, obj->position);
			break;
		case GRAFFITI:
			//DrawCube(obj->position, 1.0f, 1.0f, 1.0f, BROWN);
			DrawModelShaded(obj->model, obj->position);
			break;
		case TRASH:
			//DrawCube(obj->position, 1.0f, 1.0f, 1.0f, ORANGE);
			DrawModelShaded(obj->model, obj->position);
			break;
		case TOILET:
			//DrawCube(obj->position, 1.0f, 1.0f, 1.0f, YELLOW);
			DrawModelShaded(obj->model, obj->position);
			break;
	}
}
