#ifndef MAP_H_
#define MAP_H_

#include <stdint.h>
#include "raylib.h"

typedef struct {
	uint16_t id;
	Vector3 normal;
	Vector3 vertices[3];
} Polygon;

typedef struct {
	float d;
	Vector3 normal;
} Plane;

#define BVH_MAX_NODES		2048
#define BVH_MAX_TRIS		8
#define BVH_MIN_EXTENT 		1.5f
#define BVH_CLOSE_RANGE		5.0f

#define SAH_BUCKET_COUNT    12
#define SAH_TRAVEL_COST     1.0f
#define SAH_INTERSECT_COST  1.0f

#define UP (Vector3){0, 1, 0};

typedef struct {
	uint32_t vrt[3];
	uint32_t tex[3];
	uint32_t nrm[3];
} Face;

typedef struct {
	uint16_t count; 
	BoundingBox bounds;
} SahBucket;

typedef struct BvhNode {
	uint16_t depth;					// Subdivision step
	uint32_t tri_count;				// Number of triangles contained in node
	uint32_t *tri_indices;			// Triangle indices contained in node
	BoundingBox bounds;				// Bounding box
	struct BvhNode *children[2];	// Pointers to child nodes
	uint32_t id;					// Index(for compression)
	uint32_t mem_pad;				// Padding of 4 bytes so the struct size is a neat 64 bytes
} BvhNode;

typedef struct {
	uint32_t poly_count;
	Polygon *polygons;

	BoundingBox bounds;
	Model model;

	BvhNode *root_node;
} Map;

void LoadMap(Map *map, char *file_path);
void LoadCollisionGeometry(Map *map, char *file_path);
void UnloadMap(Map *map);

Vector3 FaceNormal(Vector3 *vertices);
Polygon *MeshToTris(Mesh mesh, uint16_t offset);

Vector3 BoxGetSize(BoundingBox box);
float BoxGetSurfaceArea(BoundingBox box);
Vector3 BoxGetCenter(BoundingBox box);

Vector3 TriCentroid(Polygon tri);
Plane TriToPlane(Polygon tri);
float PlaneDistance(Vector3 point, Plane plane);

Vector3 EdgeNormal(Vector3 p0, Vector3 p1);
Polygon TriExpanded(Polygon *tri, float radius);

void DebugDrawMapTris(Map *map);

BvhNode *MakeBvhNode(Polygon *tris, uint32_t *tri_indices, uint32_t tri_count, uint32_t depth);
void BvhTraceNodes(BvhNode *node, Ray ray, BvhNode **hits, uint32_t *hit_count, uint32_t max_hits, float max_dist);
void BvhBoxSweep(BvhNode *node, BoundingBox box, BvhNode **hits, uint32_t *hit_count, uint32_t max_hits);

void DrawBvhDebug(BvhNode *node, Polygon *all_tris);
void DrawTriNormals(Map *map);

RayCollision GetRayTriCollision(Ray ray, Polygon *tri);

#endif

