#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "raymath.h"
#include "rw.h"
#include "taskobject.h"
#include "handler.h"
#include "lights.h"
#include "npc.h"

Salesman *_sm;
Pharmacist *_pm;
void SetNpcPointers(Salesman *sm, Pharmacist *pm) { _sm = sm, _pm = pm; }

void DataRead(LightHandler *lh, Handler *obj_h) {
	char line[128];
	
	// Load lights
	FILE *pfl = fopen("map/lights.txt", "r");

	if(pfl) {
		puts("LOADING LIGHTS...");	

		lh->light_count = 0;

		while(fgets(line, sizeof(line), pfl)) {
			if(line[0] == COMMENT_MARKER) continue; // Skip commments
		
			switch(line[0]) {
				case NEW_MARKER: {
					MakeLight(0, 0.0f, Vector3Zero(), WHITE, lh);
				} break;

				case POS_MARKER: {
					Vector3 pos = Vector3Zero();
					sscanf(line, ">{%f, %f, %f}", &pos.x, &pos.y, &pos.z);
					lh->lights[lh->light_count - 1].position = pos; 
				} break;

				case 'c': {
					Vector3 color;
					sscanf(line, "color: {%f, %f, %f}", &color.x, &color.y, &color.z);
					lh->lights[lh->light_count - 1].color = (Color){color.x, color.y, color.z, 255};	
				} break;

				case 'r': {
					float range;
					sscanf(line, "range: %f", &range);
					lh->lights[lh->light_count - 1].range = range;
				} break;
			}
		}

		printf("LOADED %d LIGHTS FROM FILE\n", lh->light_count);

		fclose(pfl);
	} else puts("ERROR: COULD NOT OPEN LIGHTS FILE");

	// Load task objects 
	FILE *pfo = fopen("map/objects.txt", "r");

	if(pfo) {
		puts("LOADING OBJECTS...");

		obj_h->task_obj_count = 0;
		for(uint8_t i = 0; i < 4; i++) obj_h->instances[i] = 0; 

		while(fgets(line, sizeof(line), pfo)) {
			if(line[0] == COMMENT_MARKER) continue;	// Skip comments
			
			switch(line[0]) {
				case NEW_MARKER: {
					uint8_t type = 0;
					char *name = strtok(line + 1, "\n");

					if(strcmp(name, "SPILL") 		 == 0) type = SPILL;
					else if(strcmp(name, "GRAFFITI") == 0) type = GRAFFITI;
					else if(strcmp(name, "TRASH")	 == 0) type = TRASH;
					else if(strcmp(name, "TOILET")	 == 0) type = TOILET;

					obj_h->instances[type]++;
					obj_h->task_objects[obj_h->task_obj_count++] = MakeTaskObject(type, Vector3Zero(), obj_h->task_object_models[type]);
				} break;

				case POS_MARKER: {
					Vector3 pos = Vector3Zero();
					sscanf(line, ">{%f, %f, %f}", &pos.x, &pos.y, &pos.z);
					obj_h->task_objects[obj_h->task_obj_count - 1].position = pos;
					obj_h->task_objects[obj_h->task_obj_count - 1].start_position = pos;
				} break;

				case 'a': {
					float angle = 0.0f;
					sscanf(line, "angle: %f", &angle);
					obj_h->task_objects[obj_h->task_obj_count - 1].angle = angle;
					obj_h->task_objects[obj_h->task_obj_count - 1].start_angle = angle;
				} break;
			}
		}

		for(uint8_t i = 0; i < 4; i++) 
			obj_h->player->tasks[i].count = obj_h->instances[i];
		
		for(uint8_t i = 0; i < obj_h->player->task_count; i++) 
			obj_h->player->tasks[i].complete = (obj_h->player->tasks[i].count == 0);

		printf("LOADED %d OBJECTS FROM FILE\n", obj_h->task_obj_count);

		fclose(pfo);
	} else puts("ERROR: COULD NOT OPEN OBJECTS FILE");

	FILE *pfn = fopen("map/npc_list.txt", "r");
	
	if(pfn) {
		puts("LOADING ENTITIES...");

		uint8_t npc_id;
		_sm->spawn_point_count = 0;
		
		while(fgets(line, sizeof(line), pfn)) {
			if(line[0] == COMMENT_MARKER) continue;

			switch(line[0]) {
				case NEW_MARKER: { 
					char *name = strtok(line + 1, "\n");
					
					if(strcmp(name, "PERFUME_MAN") == 0) { 		
						npc_id = 0; 
						_sm->spawn_point_count++;
					} else if(strcmp(name, "PHARMACIST") == 0) 	npc_id = 1;
				} break;

				case POS_MARKER: {
					Vector3 pos = Vector3Zero();
					sscanf(line, ">{%f, %f, %f}", &pos.x, &pos.y, &pos.z);

					if(npc_id == 0) _sm->spawn_points[_sm->spawn_point_count - 1].position = pos; 
					else if(npc_id == 1) _pm->position = pos;
				} break;

				case 'a': {
					float angle = 0.0f;
					sscanf(line, "angle: %f", &angle);

					if(npc_id == 0) _sm->spawn_points[_sm->spawn_point_count - 1].angle = angle; 
					else if(npc_id == 1) _pm->angle = angle;
				} break;
			}
		}

		_sm->position = _sm->spawn_points[0].position;
		_sm->angle = _sm->spawn_points[0].angle;

		fclose(pfn);

	} else puts("ERROR: COULD NOT OPEN FILE, [npc_list.txt]");
} 

