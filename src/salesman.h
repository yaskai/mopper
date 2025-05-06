#ifndef SALESMAN_H_
#define SALESMAN_H_

#include <stdint.h>
#include "lights.h"
#include "raylib.h"
#include "map.h"
#include "player.h"

typedef struct {
	uint8_t flags;

	Map *map;
	Player *player;
	LightHandler *lh;

	Vector3 position;
	BoundingBox bounds;

	Model model;
	ModelAnimation *animations;
} Salesman;

Salesman SalesmanInit(Map *map, Player *player, LightHandler *lh, Vector3 position, Model model);
void SalesmanClose(Salesman *salesman);

void SalesmanUpdate(Salesman *salesman);
void SalesmanDraw(Salesman *salesman);

#endif // !SALESMAN_H_
