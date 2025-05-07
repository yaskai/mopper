#include <stdint.h>
#include <stdio.h>
#include "lights.h"
#include "player.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "include/resource_dir.h"
#include "config.h"
#include "map.h"
#include "handler.h"
#include "audioplayer.h"

enum GAME_STATE : uint8_t {
	START,
	MAIN,
	GAMEOVER
}; uint8_t game_state = START;

void UpdateMain(Player *player, LightHandler *lights, Handler *handler);
void RenderMain(Player *player, Map *map, LightHandler *lights, Config *conf, Handler *handler);

Camera cam;
Camera2D cam2D;
Camera fixed_cam;

Font font0;
Model level_model;

Vector2 v_res = {320, 240};
RenderTexture buf_rtex;

Model clipboard_model;

void PlayerSetHandler(Player *player, Handler *handler);

int main () {
	SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_FULLSCREEN_MODE);

	SetTraceLogLevel(LOG_WARNING);

	Config conf = (Config){0};
	LoadConfig(&conf, "options.conf");

	int ww = conf.ww;
	int wh = conf.wh;
	int fps = conf.fps;

	InitWindow(ww, wh, "Mopper");
	SetTargetFPS(fps);
	SetWindowFocused();
	HideCursor();
	DisableCursor();
	InitAudioDevice();

	SearchAndSetResourceDir("resources");
	
	buf_rtex = LoadRenderTexture(v_res.x, v_res.y);
	font0 = LoadFont("fonts/dokdo.ttf");

 	clipboard_model = LoadModel("models/clipboard.glb");

	char *level_name;
	level_name = "mall_greybox.glb";
	//level_name = "testmap.glb";

	Map map = (Map){0};
	//LoadCollisionGeometry(&map, "testmap.obj");

	level_model = LoadModel(level_name);
	LoadMap(&map, level_name);

	// Camera setup 
	cam.position = (Vector3){ 25.0f, 15.0f, 25.0f };
	cam.target   = (Vector3){ 0.0f, -15.0f, 0.f };
	cam.up 	  	 = (Vector3){ 0, 1, 0 };
	cam.fovy = 120;
	cam.projection = CAMERA_PERSPECTIVE; 

	cam2D.target = Vector2Zero();
	cam2D.offset = Vector2Zero();
	cam2D.rotation = 0.0f;
	cam2D.zoom = 1.0f;

	fixed_cam.position	    = (Vector3){0, 0, -4};
	fixed_cam.target	    = Vector3Zero();
	fixed_cam.up		    = (Vector3){0, 1, 0};
	fixed_cam.fovy 	   		= 45.0f;
	fixed_cam.projection	= CAMERA_PERSPECTIVE; 
	
	// Lighting setup
	LightHandler light_handler = (LightHandler){0};
	InitLights(&light_handler);
	
	for(uint16_t i = 0; i < map.model.materialCount; i++) {
		map.model.materials[i].shader = light_handler.shader;
	}

	Player player = PlayerInit((Vector3){3.0f, 1.0f, 0.0f}, &cam, &cam2D, &map, &conf, &light_handler);
	player.font = font0;
	player.fixed_cam = &fixed_cam;

	Model sm_model = LoadModel("PerfumeGuy.glb");
	for(uint16_t i = 0; i < map.model.materialCount; i++) map.model.materials[i].shader = light_handler.shader;

	Handler handler = {0};
	HandlerInit(&handler, &player, &light_handler);
	PlayerSetHandler(&player, &handler);
	
	AudioPlayer	ap = {0};
	AudioPlayerInit(&ap);

	player.ap = &ap;

	while(!WindowShouldClose()) {
		// Update
		switch(game_state) {
			case START:
				if(IsKeyPressed(KEY_ENTER)) {
					// Start game
					game_state = MAIN;
				}
				break;
			case MAIN:
				UpdateMain(&player, &light_handler, &handler);
				break;
			case GAMEOVER:
				break;
		}	

		//UpdateMusicStream(ap.tracks[TRACK_DEFAULT_MUSIC].music);
		//UpdateMusicStream(ap.tracks[TRACK_PHARM_MUSIC].music);

		// Render 
		BeginDrawing();
			ClearBackground(BLACK);

			switch(game_state) {
				case START:
					BeginMode2D(cam2D);
					//DrawText("PRESS ENTER TO START", 650, 500, 50, RAYWHITE);
					DrawTextEx(font0, "PRESS ENTER TO START", (Vector2){650, 500}, 50, 2.0f, RAYWHITE);
					EndMode2D();
					break;
				case MAIN:
					RenderMain(&player, &map, &light_handler, &conf, &handler);
					break;
				case GAMEOVER:
					break;
			}

		EndDrawing();
	}
	
	PlayerClose(&player);
	UnloadMap(&map);
	UnloadFont(font0);
	UnloadRenderTexture(buf_rtex);
	UnloadModel(level_model);
	UnloadModel(clipboard_model);
	HandlerClose(&handler);
	AudioPlayerClose(&ap);

	CloseWindow();
	return 0;
}

void UpdateMain(Player *player, LightHandler *lights, Handler *handler) {
	lights->player_light_on = 1;
	lights->player_light_range = 1;
	lights->player_pos = Vector3Subtract(player->position, player->facing);
	UpdateLights(lights);

	PlayerUpdate(player);

	HandlerUpdate(handler);
}

void RenderMain(Player *player, Map *map, LightHandler *lights, Config *conf, Handler *handler) {
	BeginTextureMode(buf_rtex);
	ClearBackground((Color){0, 0, 0, 0});
	BeginMode3D(cam);
		Matrix level_matrix = map->model.transform;
		int mat_model_loc = GetShaderLocation(lights->shader, "mat_model");
		SetShaderValueMatrix(lights->shader, mat_model_loc, level_matrix);

		DrawModel(map->model, Vector3Zero(), 1.0f, WHITE);
		//DebugDrawMapTris(map);
		
		//PlayerDraw(player);
		//DrawModelWires(level_model, Vector3Zero(), 1.0f, SKYBLUE);
		//DrawBvhDebug(map->root_node, map->polygons);
		//PlayerDrawBvhDebug(player);
		//DrawTriNormals(map);

		HandlerDraw(handler);
	EndMode3D();
	EndTextureMode();

	BeginMode2D(cam2D);
		DrawTexturePro(buf_rtex.texture, (Rectangle){0, 0, v_res.x, -v_res.y}, (Rectangle){0, 0, conf->ww, conf->wh}, Vector2Zero(), 0.0f, WHITE);
		PlayerDraw(player);
		PlayerDrawDebugInfo(player, font0);
	EndMode2D();
}

