#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "handler.h"
#include "lights.h"
#include "taskobject.h"
#include "raylib.h"
#include "raymath.h"

uint8_t task_obj_count;

void HandlerInit(Handler *handler, Player *player, LightHandler *lh) {
	handler->player = player;

	task_obj_count = 0;
	
	handler->task_object_models[SPILL]  	= LoadModel("models/spill.glb");
	handler->task_object_models[GRAFFITI]   = LoadModel("models/graffiti.glb");
	handler->task_object_models[TOILET]		= LoadModel("models/toilet.glb");
	handler->task_object_models[TRASH]  	= LoadModel("models/toilet.glb");

	for(uint8_t i = 0; i < 4; i++) {
		for(uint8_t j = 0; j < handler->task_object_models[i].materialCount; j++)
			handler->task_object_models[i].materials[j].shader = lh->shader;
	}

	for(uint8_t i = 0; i < 32; i++) handler->task_objects[i] = (TaskObject){0};
}

void HandlerClose(Handler *handler) {
	for(uint8_t i = 0; i < 4; i++) {
		UnloadModel(handler->task_object_models[i]);
	}
}

void HandlerUpdate(Handler *handler) {
	for(uint8_t i = 0; i < task_obj_count; i++) {
		TaskOjbectUpdate(&handler->task_objects[i]);
	}
}

void HandlerDraw(Handler *handler) {
	for(uint8_t i = 0; i < handler->task_obj_count; i++) {
		TaskObject *obj = &handler->task_objects[i];
		if((obj->flags & TASK_OBJ_ACTIVE) == 0) continue;

		DrawModelShadedEx(obj->model, obj->position, obj->angle);
		DrawBoundingBox(obj->bounds, WHITE);
		DrawSphere(obj->position, 1.0f, ColorAlpha(WHITE, 0.5f));
	}
}

void ResetObjects(Handler *handler) {
	for(uint8_t i = 0; i < handler->task_obj_count; i++) {
		TaskObject *obj = &handler->task_objects[i];
		
		obj->flags = (TASK_OBJ_ACTIVE);
	}
}

