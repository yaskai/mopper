#ifndef PLAYER_H_
#define PLAYER_H_

#include <stdint.h>
#include "audioplayer.h"
#include "lights.h"
#include "raylib.h"
#include "config.h"
#include "map.h"
#include "taskobject.h"

#define PLAYER_ALIVE		0x01
#define PLAYER_TASKVIEW		0x02
#define PLAYER_DAMAGE		0x04
#define PLAYER_HEAL			0x08
#define PLAYER_WIN			0x10
#define PLAYER_ATTACKED		0x20

#define PLAYER_BOUNDS \
(BoundingBox){(Vector3){ -1.0f, -1.0f, -1.0f }, (Vector3){ 1.0f, 1.0f, 1.0f }}

typedef struct {
	bool complete;
	uint8_t progress;
	uint8_t count;
	char *title;
} Task;

typedef struct {
	uint8_t flags;

	float hp;
	
	float pitch;
	float yaw;
	float roll;

	float speed;
	
	float radius;

	Vector3 position;
	Vector3 facing;

	Vector3 velocity;
	Vector3 accel;

	BoundingBox bounds;
	
	Config *conf;

	Camera *cam;
	Camera2D *cam2D;

	Camera *fixed_cam;

	LightHandler *light_handler;

	RenderTexture rend_tex;

	Map *map;

	AudioPlayer *ap;

	Font font;

	uint8_t task_count;
	Task *tasks;

	TaskObject *task_objects;

	char *dialogue;
	float dialogue_timer;

	uint8_t minute, hour;
	uint8_t task_points;

	uint8_t inhaler_count;

	float damage_timer, heal_timer;
} Player;

Player PlayerInit(Vector3 position, Camera *cam, Camera2D *cam2D, Map *map, Config *conf, LightHandler *light_handler);
void PlayerClose(Player *player);

void PlayerUpdate(Player *player);
void PlayerDraw(Player *player);

void PlayerInput(Player *player);

void PlayerDrawDebugInfo(Player *player, Font font);
void PlayerDrawBvhDebug(Player *player);

void PlayerDrawClipBoard(Player *player);

Vector3 PlayerTraceMove(Player *player, Vector3 start_point, Vector3 end_point);

void DrawTaskView(Player *player);
void DrawBreathMeter(Player *player);

void PlayerDie(Player *player);
void PlayerReset(Player *player);

void UpdateTasks(Player *player);
bool CheckWin(Player *player);

void CheckGround(Player *player);

#endif
