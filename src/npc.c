#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "audioplayer.h"
#include "player.h"
#include "raylib.h"
#include "raymath.h"
#include "npc.h"
#include "lights.h"

Player *_player;
AudioPlayer *_ap;

char *refill_no[4];
bool first_refill = true;

float attack_dist;
Vector3 attack_dir;

Salesman SalesmanInit(Map *map, Player *player, LightHandler *lh, Vector3 position, Model *model) {
	_player = player;
	_ap = player->ap;

	return (Salesman) {
		.flags = 0,
		.map = map,
		.player = player,
		.lh = lh,
		.angle = 0.0f,
		.position = position,
		.model = model
	};
}

void SalesmanClose(Salesman *salesman) {
	UnloadModelAnimation(*salesman->animations);
	UnloadModel(*salesman->model);
}

void SalesmanUpdate(Salesman *sm) {
	sm->action_timer -= GetFrameTime();	
	Player *player = sm->player;

	switch(sm->state) {
		case IDLE: {
			Vector3 forward = Vector3Normalize(Vector3Subtract(player->position, sm->position));
			sm->angle = atan2f(forward.x, forward.z) * RAD2DEG + 85.0f;
			
			//Ray ray = { .position = sm->position, .direction = forward };
			Ray ray = {.position = player->position, .direction = player->facing };
			RayCollision coll = GetRayCollisionSphere(ray, sm->position, 5.0f);

			if(coll.hit && coll.distance <= 10.0f && fabs(coll.point.y - sm->position.y) <= 2.5f) {
				if(player->hour < 1) sm->action_timer = 0.0f;
				else {
					PlayEffect(_ap, SFX_PFG_ATTACK);
					sm->action_timer = 20.0f;
					sm->state = ATTACK;
					
					attack_dir = forward;
					attack_dist = Vector3Distance(player->position, sm->position);
				}
			}

			if(sm->action_timer <= 0.0f) {
				sm->state = HIDE;
				sm->action_timer = 10.0f;
			}
		} break;

		case HIDE: {
			if(sm->action_timer <= 0.0f) {
				sm->state = COOLDOWN;

				uint8_t next_point = 0;
				float best_weight = FLT_MAX;
				
				for(uint8_t i = 0; i < sm->spawn_point_count; i++) {
					Vector3 point = sm->spawn_points[i].position;
					float dist = Vector3Distance(point, sm->player->position);

					float aggro = Clamp(1.0f - ((float)sm->player->hour / 5.0f), 0.2f, 1.0f);
					float weight = Vector3Distance(sm->player->position, point) * aggro + GetRandomValue(0, 5);
					if(fabs(point.y - player->position.y) > 2.0f) weight += 2.0f;
					
					if(weight < best_weight) {
						best_weight = weight;
						next_point = i;
					}
				}

				sm->position = sm->spawn_points[next_point].position;
				sm->angle = sm->spawn_points[next_point].angle;
				sm->action_timer = 3.0f;

				EffectSetSpatialVolume(_ap, SFX_PFG_WALK, sm->position, _player->position);
				PlayEffect(_ap, SFX_PFG_WALK);
			}
		} break;

		case ATTACK: {
			sm->position = Vector3Add(sm->position, Vector3Scale(attack_dir, 10.0f * GetFrameTime()));
			if(CheckCollisionSpheres(sm->position, 1.0f, player->position, 0.5f)) {
				sm->player->flags |= PLAYER_DAMAGE;
				sm->player->flags &= ~PLAYER_HEAL;

				sm->action_timer = 20.0f;
				sm->state = HIDE;
			} else if(sm->action_timer <= 0.0f) {
				sm->action_timer = 3.0f;
				sm->state = COOLDOWN;
			}

		} break;

		case COOLDOWN: {
			Vector3 forward = Vector3Normalize(Vector3Subtract(player->position, sm->position));
			sm->angle = atan2f(forward.x, forward.z) * RAD2DEG + 85.0f;

			if(sm->action_timer <= 0.0f) {
				sm->action_timer = 10.0f;
				sm->state = IDLE;
			}
		} break;
	}
}

void SalesmanDraw(Salesman *sm) {
	if(sm->state == HIDE) return;
	DrawModelShadedEx(*sm->model, sm->position, sm->angle);
	
	//for(uint8_t i = 0; i < sm->spawn_point_count; i++) DrawSphere(sm->spawn_points[i].position, 1.0f, ColorAlpha(RED, 0.5f));
}

Pharmacist PharmacistInit(Vector3 position, Model *model) {
	refill_no[0] = "Come back when you finish more tasks...";
	refill_no[1] = "Price is three task tokens for your inhalers...";
	refill_no[2] = "...";
	refill_no[3] = "Come back if you need inhalers or if you're done";

	return (Pharmacist) {
		.flags = 0,
		.position = position,
		.model = model,
	};
}

void PharmacistUpdate(Pharmacist *pm) {
	if(IsMouseButtonPressed(0)) {
		Ray ray = { .position = _player->position, .direction = _player->facing };
		RayCollision coll = GetRayCollisionSphere(ray, pm->position, 4.0f);

		if(coll.hit && coll.distance <= 3.5f) {
			if(_player->task_points >= 3) {
				_player->dialogue = "Here you go!";
				_player->task_points -= 3;
				_player->inhaler_count += 3;

				if(CheckWin(_player)) _player->flags |= PLAYER_WIN;
			} else {
				uint8_t diag_id = GetRandomValue(0, 2);
				if(first_refill) diag_id = 3; 
				_player->dialogue = refill_no[diag_id];
			}

			first_refill = false;
			_player->dialogue_timer = 3.0f;
		}
	}
}

void PharmacistDraw(Pharmacist *pm) {
	DrawModelShadedEx(*pm->model, pm->position, pm->angle);
}

