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
	
	handler->task_object_models[SPILL]  = LoadModel("models/spill.glb");
	handler->task_object_models[GRAFFITI]  = LoadModel("models/toilet.glb");
	handler->task_object_models[TOILET] = LoadModel("models/toilet.glb");
	handler->task_object_models[TRASH]  = LoadModel("models/toilet.glb");

	for(uint8_t i = 0; i < 4; i++) {
		for(uint8_t j = 0; j < handler->task_object_models[i].materialCount; j++)
			handler->task_object_models[i].materials[j].shader = lh->shader;
	}

	for(uint8_t i = 0; i < 32; i++) handler->task_objects[i] = (TaskObject){0};
	LoadTaskObjects(handler, "task_objects.txt");
}

void HandlerClose(Handler *handler) {
	for(uint8_t i = 0; i < 4; i++) {
	}

	UnloadModel(handler->task_object_models[TOILET]);
	//free(handler->task_objects);
}

void HandlerUpdate(Handler *handler) {
	for(uint8_t i = 0; i < task_obj_count; i++) {
		TaskOjbectUpdate(&handler->task_objects[i]);
	}
}

void HandlerDraw(Handler *handler) {
	for(uint8_t i = 0; i < task_obj_count; i++) {
		//if(!(handler->task_objects[i].flags & TASK_OBJ_ACTIVE)) continue;
		DrawModelShaded(handler->task_objects[i].model, handler->task_objects[i].position);
		//DrawBoundingBox(handler->task_objects[i].bounds, RED);
	}
}

void LoadTaskObjects(Handler *handler, char *file_path) {
	uint8_t count = 0;

	handler->spill_count = 0, handler->graff_count = 0, handler->trash_count = 0, handler->toilet_count = 0;

	FILE *file = fopen(file_path, "r");

	if(file) {
		char line[64];
		while(fgets(line,sizeof(line), file)) {
			if(line[0] == COMMENT_MARKER) continue;
			else if (line[0] == NEW_OBJ_MARRKER) {
				char *name = strtok(line + 1, "\n");
				uint8_t type = 0;

				if(strcmp(name, "SPILL") 			== 0) {	type = SPILL, 		handler->spill_count++;  } 
				else if(strcmp(name, "GRAFFITI") 	== 0) { type = GRAFFITI,	handler->graff_count++;  }
				else if(strcmp(name, "TRASH")    	== 0) { type = TRASH,		handler->trash_count++;  }
				else if(strcmp(name, "TOILET") 	 	== 0) { type = TOILET,		handler->toilet_count++; }
				else continue;

				handler->task_objects[count++] = MakeTaskObject(type, Vector3Zero(), handler->task_object_models[type]);
			} if(line[0] == SETPOS_MARKER) {
				if(count == 0) continue;

				Vector3 pos = Vector3Zero();
				sscanf(line, ">{ %f, %f, %f }", &pos.x, &pos.y, &pos.z);
				handler->task_objects[count - 1].position = pos;	
			}
		}

		handler->player->tasks[0].count = handler->spill_count;
		handler->player->tasks[1].count = handler->graff_count;
		handler->player->tasks[2].count = handler->trash_count;
		handler->player->tasks[3].count = handler->toilet_count;

		task_obj_count = count;
		handler->task_obj_count = task_obj_count;

		fclose(file);
	} else puts("ERROR: COULD NOT OPEN TASK OBJECTS FILE!");
}

