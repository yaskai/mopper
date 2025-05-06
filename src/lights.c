#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "lights.h"

Color LIGHT_COLOR_DEFAULT;

int enabled_loc;
int positions_loc;
int colors_loc;
int ranges_loc;
int count_loc;
int time_loc;

float ent_light_timer = 0.0f;

Shader light_shader;

void MakeLight(int type, float range, Vector3 position, Color color, LightHandler *handler) {
	Light *light = &handler->lights[handler->light_count];
	light->type = type;
	light->range = range;
	light->position = position;
	light->color = color;
	light->enabled = 1;

	handler->light_count++;
}

void InitLights(LightHandler *handler) {
	handler->light_count = 0;
	LIGHT_COLOR_DEFAULT = ColorBrightness(BEIGE, -0.25f);

	handler->shader = LoadShader(
		TextFormat("shaders/light_v.glsl"),
		TextFormat("shaders/light_f.glsl")
		//TextFormat("shaders/debug_f.glsl")
	);

	light_shader = handler->shader;
	
	enabled_loc = GetShaderLocation(handler->shader, "light_enabled");
	positions_loc = GetShaderLocation(handler->shader, "light_positions");
	colors_loc = GetShaderLocation(handler->shader, "light_colors");
	ranges_loc = GetShaderLocation(handler->shader, "light_ranges");
	count_loc = GetShaderLocation(handler->shader,  "light_count");
	time_loc = GetShaderLocation(handler->shader, "time");

	// Static lights
	MakeLight(LIGHT_DEFAULT, 30.0f, (Vector3){   0.0f, 8.0f,  0.0f }, LIGHT_COLOR_DEFAULT, handler);
	MakeLight(LIGHT_DEFAULT, 30.0f, (Vector3){  12.0f, 8.0f,  0.0f }, LIGHT_COLOR_DEFAULT, handler);
	MakeLight(LIGHT_DEFAULT, 30.0f, (Vector3){ -18.0f, 8.0f, 16.0f }, LIGHT_COLOR_DEFAULT, handler);

	//MakeLight(LIGHT_PLAYER, 10.0f, Vector3Zero(), LIGHT_COLOR_DEFAULT, handler);

	int enabled[MAX_LIGHTS];
	Vector3 positions[MAX_LIGHTS];
	Vector3 colors[MAX_LIGHTS];
	float ranges[MAX_LIGHTS];

	for(int i = 0; i < handler->light_count; i++) {
		enabled[i] = 1;	
		positions[i] = handler->lights[i].position;

		colors[i] = (Vector3) {
			handler->lights[i].color.r / 255.0f,
			handler->lights[i].color.g / 255.0f,
			handler->lights[i].color.b / 255.0f
		};
		
		ranges[i] = handler->lights[i].range;
	}

	int count = handler->light_count;
	
	SetShaderValueV(handler->shader, enabled_loc, enabled, SHADER_UNIFORM_INT, count);
	SetShaderValueV(handler->shader, positions_loc, positions, SHADER_UNIFORM_VEC3, count);
	SetShaderValueV(handler->shader, colors_loc, colors, SHADER_UNIFORM_VEC3, count);
	SetShaderValueV(handler->shader, ranges_loc, ranges, SHADER_UNIFORM_FLOAT, count);
	SetShaderValue(handler->shader, count_loc, &count, SHADER_UNIFORM_INT);

	Vector4 diffuse = (Vector4){ 0.55f, 0.15f, 0.15f, 1.0f };
	SetShaderValue(handler->shader, GetShaderLocation(handler->shader, "col_diffuse"), &diffuse, SHADER_UNIFORM_VEC4);

	printf("light count: %d\n", handler->light_count);
	for(int i = 0; i < handler->light_count; i++) {
		printf("light[%d] enabled: %d\n", i, handler->lights[i].enabled);
	}
}

void UpdateLights(LightHandler *handler) {
	float time = GetTime();
	SetShaderValue(handler->shader, time_loc, &time, SHADER_UNIFORM_FLOAT);

	int enabled[handler->light_count];
	Vector3 positions[handler->light_count];
	float ranges[handler->light_count];

	for(int i = 0; i < handler->light_count; i++) {
		if(handler->lights[i].type == LIGHT_PLAYER) {
			handler->lights[i].enabled = handler->player_light_on;
			handler->lights[i].range = handler->player_light_range;
			handler->lights[i].position = handler->player_pos;
		}

		// Set on/off
		enabled[i] = handler->lights[i].enabled;
		SetShaderValueV(handler->shader, enabled_loc, enabled, SHADER_UNIFORM_INT, handler->light_count);

		// Set positions
		positions[i] = handler->lights[i].position;
		SetShaderValueV(handler->shader, positions_loc, positions, SHADER_UNIFORM_VEC3, handler->light_count);

		// Set ranges
		ranges[i] = handler->lights[i].range;		
		SetShaderValueV(handler->shader, ranges_loc, positions, SHADER_UNIFORM_VEC3, handler->light_count);
	}
}

void DrawModelShaded(Model model, Vector3 position) {
	Matrix mat = model.transform;
	int mat_model_loc = GetShaderLocation(light_shader, "mat_model");
	SetShaderValueMatrix(light_shader, mat_model_loc, mat);

	DrawModel(model, position, 1.0f, WHITE);	
}
 
