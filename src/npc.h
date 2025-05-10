#ifndef NPC_H_
#define NPC_H_

#include <stdint.h>
#include "lights.h"
#include "raylib.h"
#include "map.h"
#include "player.h"

typedef enum : uint8_t {
	IDLE,
	HIDE,
	ATTACK,
	COOLDOWN
} SM_STATE;

typedef struct {
	float angle;
	Vector3 position;
} SpawnPoint;

typedef struct {
	uint8_t flags;
	uint8_t state;

	Map *map;
	Player *player;
	LightHandler *lh;

	float angle;

	float action_timer;

	Vector3 position;
	BoundingBox bounds;

	Model *model;
	ModelAnimation *animations;

	uint8_t spawn_point_count;
	SpawnPoint spawn_points[32];
} Salesman;

Salesman SalesmanInit(Map *map, Player *player, LightHandler *lh, Vector3 position, Model *model);
void SalesmanClose(Salesman *salesman);

void SalesmanUpdate(Salesman *salesman);
void SalesmanDraw(Salesman *salesman);

typedef struct {
	uint8_t flags;

	float angle;
	Vector3 position;

	Model *model;
} Pharmacist;

Pharmacist PharmacistInit(Vector3 position, Model *model);
void PharmacistClose(Pharmacist *pharm);

void PharmacistUpdate(Pharmacist *pm);
void PharmacistDraw(Pharmacist *pm);

#endif // !NPC_H_
