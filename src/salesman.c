#include "raylib.h"
#include "salesman.h"

Salesman SalesmanInit(Map *map, Player *player, LightHandler *lh, Vector3 position, Model model) {
	return (Salesman) {
		.flags = 0,
		.map = map,
		.player = player,
		.lh = lh,
		.position = position,
		.model = model
	};
}

void SalesmanClose(Salesman *salesman) {
	UnloadModelAnimation(*salesman->animations);
	UnloadModel(salesman->model);
}

void SalesmanUpdate(Salesman *salesman) {

}

void SalesmanDraw(Salesman *sm) {
	Matrix mat = sm->model.transform;
	int mat_model_loc = GetShaderLocation(sm->lh->shader, "mat_model");
	SetShaderValueMatrix(sm->lh->shader, mat_model_loc, mat);

	DrawModel(sm->model, sm->position, 1.0f, WHITE);
}

