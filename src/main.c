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
#include "rw.h"
#include "npc.h"

enum GAME_STATE : uint8_t { START, MAIN, GAMEOVER_LOSE, GAMEOVER_WIN }; 
uint8_t game_state = START;

void UpdateMain(Player *player, LightHandler *lights, Handler *handler, Salesman *sm, Pharmacist *pm);
void RenderMain(Player *player, Map *map, LightHandler *lights, Config *conf, Handler *handler, Salesman *sm, Pharmacist *pm);
void ResetGame(Player *player, Handler *handler);
void RenderTitle(Config *conf, Map *map, LightHandler *lh);

Camera cam;
Camera2D cam2D;
Camera fixed_cam;

Font font0;
//Model level_model;

Vector2 v_res = {320, 240};
RenderTexture buf_rtex;
RenderTexture ui_buf;

void PlayerSetHandler(Player *player, Handler *handler);

int main () {
	SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_FULLSCREEN_MODE);
	SetTraceLogLevel(LOG_INFO);

	int ww = 1920, wh = 1080, fps = 60;
	Config conf = (Config){0};
	LoadConfig(&conf, "options.conf");
	
	if(conf.flags & CONF_LOADED) ww = conf.ww, wh = conf.wh, fps = conf.fps;
	else conf.ww = ww, conf.wh = wh, conf.fps = fps;

	InitWindow(ww, wh, "Mopper");
	SetTargetFPS(fps);
	SetWindowFocused();
	HideCursor();
	DisableCursor();
	InitAudioDevice();

	SearchAndSetResourceDir("resources");
	
	buf_rtex = LoadRenderTexture(v_res.x, v_res.y);
	ui_buf = LoadRenderTexture(1920, 1080);
	font0 = LoadFont("fonts/dokdo.ttf");

	char *level_name;
	level_name = "map/map_model.glb";
	//level_name = "mall_greybox.glb";
	//level_name = "testmap.glb";

	Map map = (Map){0};
	//LoadCollisionGeometry(&map, "testmap.obj");

	//level_model = LoadModel(level_name);
	LoadMap(&map, level_name);

	// Camera setup 
	cam.position = (Vector3){ 8.0f, 2.0f, 1.0f };
	cam.target   = (Vector3){ 0.0f, 2.0f, 0.0f };
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
	fixed_cam.fovy 	   		= 60.0f;
	fixed_cam.projection	= CAMERA_PERSPECTIVE; 
	
	// Lighting setup
	LightHandler light_handler = (LightHandler){0};
	InitLights(&light_handler);
	
	for(uint16_t i = 0; i < map.model.materialCount; i++) map.model.materials[i].shader = light_handler.shader;

	Player player = PlayerInit((Vector3){3.0f, 1.5f, 0.0f}, &cam, &cam2D, &map, &conf, &light_handler);
	player.font = font0;
	player.fixed_cam = &fixed_cam;
		
	Model npc_models[2] = { LoadModel("models/perfumeGuy.glb"), LoadModel("models/pharmacist.glb") };

	for(uint8_t i = 0; i < 2; i++) 
		for(uint8_t j = 0; j < npc_models[i].materialCount; j++)
		npc_models[i].materials[j].shader = light_handler.shader;

	AudioPlayer	ap = {0};
	AudioPlayerInit(&ap);
	player.ap = &ap;

	Salesman sm = SalesmanInit(&map, &player, &light_handler, player.position, &npc_models[0]);
	sm.position.y = 1.0f;

	Pharmacist pm = PharmacistInit(Vector3Zero(), &npc_models[1]);

	Handler handler = {0};
	HandlerInit(&handler, &player, &light_handler);
	PlayerSetHandler(&player, &handler);


	SetNpcPointers(&sm, &pm);
	DataRead(&light_handler, &handler);

	while(!WindowShouldClose()) {
		PlayTrack(&ap, TRACK_DEFAULT_MUSIC);
		
		// Update
		switch(game_state) {
			case START:
				if(IsKeyPressed(KEY_ENTER)) game_state = MAIN; // Start game
				break;

			case MAIN:
				UpdateMain(&player, &light_handler, &handler, &sm, &pm);
				break;

			case GAMEOVER_LOSE:
			case GAMEOVER_WIN:
				if(IsKeyPressed(KEY_ENTER)) ResetGame(&player, &handler);
				if(IsKeyPressed(KEY_ENTER)) ResetGame(&player, &handler);
				break;
		}	

		// Render 
		BeginDrawing();
			ClearBackground(BLACK);

			switch(game_state) {
				case START:
					RenderTitle(&conf, &map, &light_handler);
					break;

				case MAIN:
					RenderMain(&player, &map, &light_handler, &conf, &handler, &sm, &pm);
					break;

				case GAMEOVER_LOSE:
					BeginMode2D(cam2D);
					DrawTextEx(font0, "YOU DIED", (Vector2){750, 500}, 50, 2.0f, RAYWHITE);
					EndMode2D();
					break;

				case GAMEOVER_WIN:
					BeginMode2D(cam2D);
					DrawTextEx(font0, "SHIFT OVER", (Vector2){750, 500}, 50, 2.0f, RAYWHITE);
					EndMode2D();
					break;
			}

		EndDrawing();
	}
	
	PlayerClose(&player);
	UnloadMap(&map);
	UnloadFont(font0);
	UnloadRenderTexture(buf_rtex);
	UnloadRenderTexture(ui_buf);
	//UnloadModel(level_model);
	
	for(uint8_t i = 0; i < 2; i++) UnloadModel(npc_models[i]);

	HandlerClose(&handler);
	AudioPlayerClose(&ap);

	CloseWindow();
	return 0;
}

void UpdateMain(Player *player, LightHandler *lights, Handler *handler, Salesman *sm, Pharmacist *pm) {
	UpdateLights(lights);
	PlayerUpdate(player);
	HandlerUpdate(handler);
	SalesmanUpdate(sm);
	PharmacistUpdate(pm);

	if((player->flags & PLAYER_ALIVE) == 0) game_state = GAMEOVER_LOSE;
	if((player->flags & PLAYER_WIN)	  != 0) game_state = GAMEOVER_WIN;
}

void RenderMain(Player *player, Map *map, LightHandler *lights, Config *conf, Handler *handler, Salesman *sm, Pharmacist *pm) {
	BeginTextureMode(buf_rtex);
	ClearBackground((Color){0, 0, 0, 0});
	BeginMode3D(cam);

	DrawModelShadedEx(map->model, Vector3Zero(), 0.0f);
	HandlerDraw(handler);

	SalesmanDraw(sm);
	PharmacistDraw(pm);

	//DrawModel(map->model, Vector3Zero(), 1.0f, WHITE);
	//DebugDrawMapTris(map);
	
	//PlayerDraw(player);
	//DrawModelWires(level_model, Vector3Zero(), 1.0f, SKYBLUE);
	//DrawBvhDebug(map->root_node, map->polygons);
	//PlayerDrawBvhDebug(player);
	//DrawTriNormals(map);

	EndMode3D();
	EndTextureMode();

	BeginMode2D(cam2D);

	DrawTexturePro(buf_rtex.texture, (Rectangle){0, 0, v_res.x, -v_res.y}, (Rectangle){0, 0, conf->ww, conf->wh}, Vector2Zero(), 0.0f, WHITE);
	PlayerDraw(player);
	//PlayerDrawDebugInfo(player, font0);

	EndMode2D();
}

void ResetGame(Player *player, Handler *handler) {
	PlayerReset(player);
	ResetObjects(handler);
	game_state = MAIN;
}

void RenderTitle(Config *conf, Map *map, LightHandler *lh) {
	UpdateCamera(&cam, CAMERA_ORBITAL);
	UpdateLights(lh);

	BeginTextureMode(buf_rtex);
	ClearBackground((Color){0, 0, 0, 0});
	BeginMode3D(cam);
	DrawModelShadedEx(map->model, Vector3Zero(), 0.0f);
	EndMode3D();
	EndTextureMode();
	
	BeginTextureMode(ui_buf);
	ClearBackground((Color){0, 0, 0, 0});
	BeginMode2D(cam2D);
	DrawTextEx(font0, "The Last Mopper", (Vector2){450, 300}, 150, 2.0f, RAYWHITE);
	DrawTextEx(font0, "PRESS ENTER TO START", (Vector2){680, 800}, 50, 2.0f, RAYWHITE);
	EndMode2D();
	EndTextureMode();

	BeginMode2D(cam2D);
	//DrawTextEx(font0, "The Last Mopper", (Vector2){450, 300}, 150, 2.0f, RAYWHITE);
	//DrawTextEx(font0, "PRESS ENTER TO START", (Vector2){680, 800}, 50, 2.0f, RAYWHITE);
	DrawTexturePro(buf_rtex.texture, (Rectangle){0, 0, v_res.x, -v_res.y}, (Rectangle){0, 0, conf->ww, conf->wh}, Vector2Zero(), 0.0f, WHITE);
	DrawTexturePro(ui_buf.texture, (Rectangle){0, 0, 1920, -1080}, (Rectangle){0, 0, conf->ww, conf->wh}, Vector2Zero(), 0.0f, WHITE);
	EndMode2D();
}
