#ifndef HANDLER_H_
#define HANDLER_H_

#include <stdint.h>
#include "lights.h"
#include "raylib.h"
#include "taskobject.h"
#include "player.h"

#define COMMENT_MARKER 	'#'
#define NEW_OBJ_MARRKER '_'
#define SETPOS_MARKER	'>'

typedef struct {
	uint8_t spill_count, graff_count, trash_count, toilet_count;
	uint8_t task_obj_count;

	Player *player;

	TaskObject task_objects[32];
	Model task_object_models[4];
} Handler;

void HandlerInit(Handler *handler, Player *player, LightHandler *lh);
void HandlerClose(Handler *handler);

void HandlerUpdate(Handler *handler);
void HandlerDraw(Handler *handler);

void LoadTaskObjects(Handler *handler, char *file_path);

#endif // !HANDLER_H_
