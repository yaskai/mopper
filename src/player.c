#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audioplayer.h"
#include "map.h"
#include "raylib.h"
#include "raymath.h"
#include "taskobject.h"
#include "player.h"
#include "handler.h"

Handler *pHandler;
void PlayerSetHandler(Player *player, Handler *handler) { pHandler = handler; }

float dt;
Vector3 up = (Vector3){0, 1, 0};

float max_vel = 2.0f;

RenderTexture cb_tex;
Model cb_model;
int cb_paper_mat_id = 4;
Texture paper_tex;

float taskview_on_timer = 0.0f;

char *task_titles[4];

int wall_hit_id = -1;
BvhNode *wall_hit_node;
Vector3 closest_coll_point = (Vector3){0, 0, 0}; 

Ray move_ray;

Player PlayerInit(Vector3 position, Camera *cam, Camera2D *cam2D, Map *map, Config *conf, LightHandler *light_handler) {
	RenderTexture rend_tex;
	rend_tex = LoadRenderTexture(conf->ww, conf->wh);

	Player player = (Player){
		.flags = (PLAYER_ALIVE),
		.speed = 4.5f,
		.radius = 0.5f,
		.position = position,
		.facing = Vector3Zero(),
		.conf  = conf,
		.cam = cam,
		.cam2D = cam2D,
		.rend_tex = rend_tex,
		.map = map,
		.light_handler = light_handler,

		.yaw = 0.0f,
		.pitch = 0.0f,
		.roll = 0.0f
	}; 

	cb_tex = LoadRenderTexture(1024, 1024);
	//cb_model = LoadModel("models/cb.glb");
	cb_model = LoadModel("models/clipboard.glb");

	paper_tex = cb_model.materials[cb_paper_mat_id].maps[MATERIAL_MAP_DIFFUSE].texture;

	//for(uint8_t i = 0; i < cb_model.materialCount; i++) cb_model.materials[i].shader = player.light_handler->shader;

	player.task_count = 4;
	player.tasks = (Task*)malloc(sizeof(Task) * player.task_count);
	
	task_titles[0] = "Mop spills";
	task_titles[1] = "Remove graffiti";
	task_titles[2] = "Take out trash";
	task_titles[3] = "Clean toilets";

	for(uint8_t i = 0; i < player.task_count; i++) {
		player.tasks[i].complete = false;
		player.tasks[i].progress = 0;
		player.tasks[i].title = task_titles[i];
	}

	return player;
}

void PlayerClose(Player *player) {
	UnloadRenderTexture(player->rend_tex);
	UnloadRenderTexture(cb_tex);
	UnloadTexture(paper_tex);	

	free(player->tasks);
}

void PlayerUpdate(Player *player) {
	dt = GetFrameTime() * 100.0f;

	player->bounds = (BoundingBox){ .min = Vector3Add(PLAYER_BOUNDS.min, player->position),
								    .max = Vector3Add(PLAYER_BOUNDS.max, player->position) };

	PlayerInput(player);
}

void PlayerDraw(Player *player) {
	if(player->flags & PLAYER_TASKVIEW) {
		PlayerDrawClipBoard(player);
	}
	
	BeginMode2D(*player->cam2D);
		DrawTextureRec(player->rend_tex.texture, (Rectangle){0, 0, player->conf->ww, -player->conf->wh}, (Vector2){0, 0}, WHITE);
	EndMode2D();
}

void PlayerInput(Player *player) {
	float mouse_sensitivity = player->conf->mouse_sensitivity;
	Camera *cam = player->cam;

	Vector2 mouse_delta = GetMouseDelta();
    player->yaw += mouse_delta.x * mouse_sensitivity; 	// Horizontal rotation
    player->pitch -= mouse_delta.y * mouse_sensitivity; // Vertical rotation

    const float max_pitch = 89.0f * DEG2RAD; 
    player->pitch = Clamp(player->pitch, -max_pitch, max_pitch);
	
	Vector3 forward = Vector3Normalize((Vector3) {
		.x = cosf(player->yaw) * cosf(player->pitch),
		.y = sinf(player->pitch),
		.z = sinf(player->yaw) * cosf(player->pitch)
	});

    cam->target = Vector3Add(cam->position, forward);

	Vector3 move_dir = Vector3Zero();
	Vector3 horizontal_forward = Vector3Normalize((Vector3){forward.x, 0.0f, forward.z});
	
	Vector3 right = Vector3Normalize(Vector3CrossProduct(horizontal_forward, up));

    if(IsKeyDown(KEY_W)) move_dir = Vector3Add(move_dir, horizontal_forward);
    if(IsKeyDown(KEY_S)) move_dir = Vector3Subtract(move_dir, horizontal_forward);
    if(IsKeyDown(KEY_A)) move_dir = Vector3Subtract(move_dir, right);
    if(IsKeyDown(KEY_D)) move_dir = Vector3Add(move_dir, right);
	
    Vector3 h_vel = (Vector3){player->velocity.x, 0.0f, player->velocity.z};
    player->accel = Vector3Scale(move_dir, (player->speed) * dt);
	h_vel = Vector3Add(h_vel, player->accel);

    player->facing = forward; 

	if(Vector3Length(move_dir) > 0.0f) move_dir = Vector3Normalize(move_dir);
	else h_vel = Vector3Scale(h_vel, GetFrameTime() * 0.75f);

	if(Vector3Length(h_vel) > max_vel) h_vel = Vector3Scale(Vector3Normalize(h_vel), max_vel);

	player->velocity.x = (GetFrameTime() * h_vel.x);
	player->velocity.z = (GetFrameTime() * h_vel.z);
	
	Vector3 next_pos = Vector3Add(player->position, player->velocity);
	//player->position = PlayerTraceMove(player, player->position, next_pos);
	player->position = PlayerFindMove(player, player->position, next_pos);

	cam->position = player->position;
    cam->target = Vector3Add(cam->position, forward);

	if(IsKeyPressed(KEY_E)) {
		if((player->flags & PLAYER_TASKVIEW) == 0) {
			player->flags |= PLAYER_TASKVIEW;
		} else {
			BeginTextureMode(player->rend_tex);
			ClearBackground((Color){0, 0, 0, 0});
			EndTextureMode();
			player->flags &= ~PLAYER_TASKVIEW;
		}
	}

	// Interact with task objects
	if(IsMouseButtonPressed(0)) {
		Ray ray = (Ray){.position = player->position, .direction = Vector3Normalize(Vector3Subtract(cam->target, cam->position))};

		for(uint8_t i = 0; i < pHandler->task_obj_count; i++) {
			TaskObject *obj = &pHandler->task_objects[i];
			if(!(obj->flags & TASK_OBJ_ACTIVE)) continue;
			
			RayCollision coll = GetRayCollisionBox(ray, obj->bounds);
			if(coll.hit && coll.distance <= 1.0f) {
				PlaySound(player->ap->sfx[SFX_CLEAN].sound);

				obj->flags &= ~TASK_OBJ_ACTIVE;
				player->tasks[obj->type].progress++;
				if(player->tasks[obj->type].progress == player->tasks[obj->type].count) player->tasks[obj->type].complete = true;
				break;
			}
		}
	}
}

void PlayerDrawDebugInfo(Player *player, Font font) {
	DrawTextEx(font,
			TextFormat("position {%f, %f, %f}",
			player->position.x, player->position.y, player->position.z),
			(Vector2){32, 32}, 30, 1.0f, RAYWHITE);

	DrawTextEx(font,
			TextFormat("TASKVIEW_ON: %d", (player->flags & PLAYER_TASKVIEW)),
			(Vector2){32, 64}, 30, 1.0f, RAYWHITE);
}

void PlayerDrawBvhDebug(Player *player) {
	Vector3 dir = Vector3Normalize(Vector3Subtract(Vector3Add(player->position, player->velocity), player->position));

	Ray ray = (Ray){player->position, dir};
	
	uint32_t max_hits = 3;
	uint32_t hit_count = 0;
	BvhNode *hits[max_hits];
	float max_dist = 30.0f;

	BvhTraceNodes(player->map->root_node, ray, hits, &hit_count, max_hits, max_dist);
	for(uint32_t i = 0; i < hit_count; i++) {
		BvhNode *node = hits[i];

		DrawBoundingBox(node->bounds, GREEN);

		for(uint32_t j = 0; j < node->tri_count; j++) {
			Polygon *tri = &player->map->polygons[node->tri_indices[j]];

			RayCollision coll = GetRayCollisionTriangle(ray, tri->vertices[0], tri->vertices[1], tri->vertices[2]);
			if(coll.hit) DrawTriangle3D(tri->vertices[0], tri->vertices[1], tri->vertices[2], ColorAlpha(SKYBLUE, 0.8f));
			else DrawTriangle3D(tri->vertices[0], tri->vertices[1], tri->vertices[2], ColorAlpha(ORANGE, 0.2f));
		}
	}

	DrawSphere(closest_coll_point, 0.5f, RED);
}

void PlayerDrawClipBoard(Player *player) {
	BeginTextureMode(cb_tex);
	ClearBackground((Color){0, 0, 0, 0});
	DrawTaskView(player);	
	EndTextureMode();

	cb_model.materials[4].maps[MATERIAL_MAP_DIFFUSE].texture = cb_tex.texture;

	BeginTextureMode(player->rend_tex);
	ClearBackground((Color){0, 0, 0, 0});
	BeginMode3D(*player->fixed_cam);
	
	DrawModel(cb_model, Vector3Zero(), 1.0f, WHITE);
		
	EndMode3D();
	EndTextureMode();
}

Vector3 PlayerTraceMove(Player *player, Vector3 start_point, Vector3 end_point) {
	Map *map = player->map;

	Vector3 dest = end_point;

	Vector3 direction = Vector3Normalize(Vector3Subtract(end_point, start_point));
	float full_dist = Vector3Distance(end_point, start_point) + player->radius;
	
	BoundingBox check_bounds = (BoundingBox) {
		Vector3Min(Vector3Add(player->bounds.min, start_point), Vector3Add(player->bounds.min, end_point)),	
		Vector3Max(Vector3Add(player->bounds.max, start_point), Vector3Add(player->bounds.max, end_point)) };

	uint32_t max_node_hits = 32;
	uint32_t node_hit_count = 0;
	BvhNode *hit_nodes[max_node_hits];
	BvhBoxSweep(map->root_node, check_bounds, hit_nodes, &node_hit_count, max_node_hits);

	int hit_id = -1;
	
	Ray ray = (Ray){ .position = start_point, .direction = Vector3Normalize(Vector3Subtract(end_point, start_point)) };
	RayCollision closest_coll;
	closest_coll.distance = FLT_MAX;

	for(uint32_t i = 0; i < node_hit_count; i++) {
		BvhNode *node = hit_nodes[i];
		if(node->tri_count == 0) continue;

		for(uint32_t j = 0; j < node->tri_count; j++) {
			Polygon *tri = &map->polygons[node->tri_indices[j]];
			if(fabsf(tri->normal.y) == 1.0f) continue;

			RayCollision coll = GetRayCollisionTriangle(ray, tri->vertices[0], tri->vertices[1], tri->vertices[2]);
			if(!coll.hit || coll.distance > player->radius) continue;

			Plane plane = TriToPlane(*tri);

			float dist = PlaneDistance(dest, plane);

			if(dist < player->radius) {
				float push_dist = player->radius - dist;
				dest = Vector3Add(dest, Vector3Scale(plane.normal, push_dist));
			}
		}
	}
	
	return (Vector3){dest.x, player->position.y, dest.z};
}

Vector3 PlayerFindMove(Player *player, Vector3 start_point, Vector3 wish_point) {
	Map *map = player->map;

	start_point.y -= 0.7f;
	wish_point.y -= 0.7f;

	Vector3 dest = wish_point;	// Initialize destination vector

	Vector3 direction = Vector3Normalize(Vector3Subtract(wish_point, start_point));	// Calculate direction
	float full_dist = Vector3Distance(wish_point, start_point);						// Calulate desired movement distance
	
	Ray ray = { .position = start_point, .direction = direction };
	
	// -- BVH SETUP --
	uint32_t max_node_hits = 64;						// Max number of nodes allowed to be tested
	uint32_t node_hit_count = 0;						// Number of nodes that intersected ray 
	float bvh_max_dist = full_dist + player->radius;	// Max distance for node collection(if exceeded, node won't be stored in hit_nodes)
	BvhNode *hit_nodes[max_node_hits];					// Nodes collected
	
	// -- BVH TRACING --
	// Collect BVH nodes, individual polygons tested later
	BvhTraceNodes(map->root_node, ray, hit_nodes, &node_hit_count, max_node_hits, 30.0f);
	
	RayCollision closest_coll;			// Closest collision
	closest_coll.distance = FLT_MAX;	// Distance is set to max to ensure hits
	int hit_id = -1;					// Index of polygon that was hit, -1 if no hit was found
	
	// -- COLLISION TESTS --
	for(uint32_t i = 0; i < node_hit_count; i++) {
		BvhNode *node = hit_nodes[i];

		for(uint32_t j = 0; j < node->tri_count; j++) {
			Polygon *tri = &map->polygons[node->tri_indices[j]];
			if(fabsf(tri->normal.y) == 1.0f) continue;	// Skip collision checks with floors and ceilings

			RayCollision coll = GetRayCollisionTriangle(ray, tri->vertices[0], tri->vertices[1], tri->vertices[2]);

			// Update closest collision
			if(coll.hit && coll.distance <= closest_coll.distance) {
				closest_coll = coll;
				hit_id = tri->id;

				closest_coll_point = coll.point;
				if(coll.distance <= EPSILON) break;	// Exit early on very close hits
			}
		}
	}

	// -- COLLISION RESPONSE --
	if(hit_id > -1 && closest_coll.distance <= full_dist + player->radius) {
		float new_dist = closest_coll.distance - player->radius;
		new_dist = fmaxf(new_dist, 0.0f);
		if(new_dist <= 0.1f) new_dist = 0.0f;

		dest = Vector3Add(start_point, Vector3Scale(direction, new_dist));
	}

	return (Vector3){dest.x, player->position.y, dest.z};
}

void DrawTaskView(Player *player) {
	ClearBackground(RAYWHITE);
	DrawTextureV(paper_tex, Vector2Zero(), WHITE);
	
	Vector2 origin = {150, 100};

	for(uint8_t i = 0; i < player->task_count; i++) {
		float y_val = origin.y + (80 * i);

		DrawTextEx(player->font,
			 TextFormat(
			"%s [%d/%d]",
			 player->tasks[i].title,
			 player->tasks[i].progress,
			 player->tasks[i].count),
			 (Vector2){origin.x, y_val},
			 80, 1, 
			 BLACK);

		if(player->tasks[i].complete) DrawLineEx((Vector2){origin.x - 20, y_val + 25}, (Vector2){origin.x + 750, y_val + 25}, 5, RED);
	}
}

