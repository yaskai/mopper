#ifndef LIGHTS_H_
#define LIGHTS_H_

#include <stdint.h>
#include "raylib.h"

#define MAX_LIGHTS 12

typedef enum : uint8_t {
	LIGHT_DEFAULT 	= 0,
	LIGHT_PLAYER 	= 1,	
} LIGHT_TYPE;

typedef struct {
	int enabled;
	int type;
	int id;
	float range;
	Vector3 position;
	Color color;
} Light;

typedef struct {
	int light_count;
	Light lights[MAX_LIGHTS];
	Shader shader;
	
	int player_light_on;
	int player_light_range;
	Vector3 player_pos;
} LightHandler;

void MakeLight(int type, float range, Vector3 position, Color color, LightHandler *handler);
void InitLights(LightHandler *handler);
void UpdateLights(LightHandler *handler);

void DrawModelShaded(Model model, Vector3 position);

#endif
