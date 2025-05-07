#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include "raylib.h"
#include "raymath.h"
#include "map.h"

void LoadMap(Map *map, char *file_path) {
	map->model = LoadModel(file_path);
	
	// Count polygons
	uint32_t poly_count = 0;
	for(uint32_t i = 0; i < map->model.meshCount; i++) {
		poly_count += map->model.meshes[i].triangleCount;
	} map->poly_count = poly_count;

	// Allocate memory for polgyon array 
	map->polygons = (Polygon*)malloc(sizeof(Polygon) * map->poly_count);
	
	// Copy polygons from model into array
	uint16_t offset = 0;
	for(uint16_t i = 0; i < map->model.meshCount; i++) {
		uint16_t mesh_poly_count = map->model.meshes[i].triangleCount;
		Polygon *mesh_tris = MeshToTris(map->model.meshes[i], offset);

		memcpy(&map->polygons[offset], mesh_tris, sizeof(Polygon) * mesh_poly_count);

		offset += mesh_poly_count;
		free(mesh_tris);
	}

	uint32_t tri_indices[map->poly_count];
    for(uint32_t i = 0; i < map->poly_count; i++) tri_indices[i] = map->polygons[i].id; 

	// Make BVH 
	map->root_node = MakeBvhNode(map->polygons, tri_indices, map->poly_count, 0);
}

void LoadCollisionGeometry(Map *map, char *file_path) {
	FILE *file = fopen(file_path,"r");

	if(file != 0) {
		uint32_t v_count = 0, n_count = 0, f_count = 0;

		char line[64];

		while(fgets(line, sizeof(line), file)) {
			if(line[0] == 'v' && line[1] == ' ') v_count++;
			else if(line[0] == 'v' && line[1] == 'n') n_count++;
			else if(line[0] == 'f' && line[1] == ' ') f_count++;
		}

		Vector3 vertices[v_count];
		Vector3 normals[n_count];
		Face faces[f_count];

		fseek(file, 0, SEEK_SET);
		uint32_t v_index = 0, n_index = 0, f_index = 0;

		while(fgets(line, sizeof(line), file)) {
			if(line[0] == 'v' && line[1] == ' ') {
				sscanf(line, "v %f %f %f", &vertices[v_index].x, &vertices[v_index].y, &vertices[v_index].z);
				v_index++;				
			} else if(line[0] == 'v' && line[1] == 'n') {
				sscanf(line, "vn %f %f %f", &normals[n_index].x, &normals[n_index].y, &normals[n_index].z);
				n_index++;				
			} else if(line[0] == 'f' && line[1] == ' ') {
				Face f = (Face){0};
				
				sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
							&f.vrt[0], &f.tex[0], &f.nrm[0],
							&f.vrt[1], &f.tex[1], &f.nrm[1],
							&f.vrt[2], &f.tex[2], &f.nrm[2]);

				for(uint8_t i = 0; i < 3; i++) { f.vrt[i]--, f.tex[i]--, f.nrm[i]--; }
				
				faces[f_index] = f;
				f_index++;
			}
		}

		fclose(file);

		map->poly_count = f_count;
		map->polygons = (Polygon*)malloc(sizeof(Polygon) * map->poly_count);

		for(uint32_t i = 0; i < f_count; i++) {
			Face f = faces[i];

			Vector3 verts[3];
			for(uint8_t j = 0; j < 3; j++) { verts[j] = vertices[f.vrt[j]]; }

			Polygon tri = (Polygon){0};
			memcpy(tri.vertices, verts, sizeof(Vector3) * 3);
			tri.normal = normals[f.nrm[0]];

			tri.id = i;

			map->polygons[i] = tri;
		}

		uint32_t tri_indices[map->poly_count];
		for(uint32_t i = 0; i < map->poly_count; i++) tri_indices[i] = map->polygons[i].id;
		
		map->root_node = MakeBvhNode(map->polygons, tri_indices, map->poly_count, 0);
	} else {
		printf("ERROR: COLLISION GEOMETRY COULD NOT BE LOADED, FILE: %s\n", file_path);
	}
}

void UnloadMap(Map *map) {
	UnloadModel(map->model);
	free(map->polygons);
}

Vector3 FaceNormal(Vector3 *vertices) {
	Vector3 u = Vector3Subtract(vertices[1], vertices[0]);  // Edge 1
	Vector3 v = Vector3Subtract(vertices[2], vertices[0]);	// Edge 2
	return Vector3Normalize(Vector3CrossProduct(u, v));		// Normalized cross product 
}

Polygon *MeshToTris(Mesh mesh, uint16_t offset) {
	Polygon *tris = (Polygon*)malloc(sizeof(Polygon) * mesh.triangleCount);
	
	if(tris == NULL) {
		puts("ERROR: COULD NOT COMPLETE MESH TO POLYGON CONVERSION");
		return NULL;
	}

	if(mesh.indices != NULL) {
		for(uint16_t i = 0; i < mesh.triangleCount; i++) {
			Polygon tri = (Polygon){0};

			uint16_t indices[3] = { mesh.indices[(i * 3) + 0], mesh.indices[(i * 3) + 1], mesh.indices[(i * 3) + 2] };	

			for(uint16_t j = 0; j < 3; j++) {
				uint16_t index = indices[j];
				tri.vertices[j].x = mesh.vertices[(index * 3) + 0];
				tri.vertices[j].y = mesh.vertices[(index * 3) + 1];
				tri.vertices[j].z = mesh.vertices[(index * 3) + 2];
			}
			
			tri.normal = FaceNormal(tri.vertices);
			tri.id = i + offset;
			tris[i] = tri;
		}
	} else if(mesh.indices == NULL) {
		for(uint16_t i = 0; i < mesh.triangleCount; i++) {
			Polygon tri = (Polygon){0};

			tri.vertices[0].x = mesh.vertices[(i * 9) + 0];
			tri.vertices[0].y = mesh.vertices[(i * 9) + 1];	
			tri.vertices[0].z = mesh.vertices[(i * 9) + 2];	

			tri.vertices[1].x = mesh.vertices[(i * 9) + 3];
			tri.vertices[1].y = mesh.vertices[(i * 9) + 4];	
			tri.vertices[1].z = mesh.vertices[(i * 9) + 5];	

			tri.vertices[2].x = mesh.vertices[(i * 9) + 6];
			tri.vertices[2].y = mesh.vertices[(i * 9) + 7];	
			tri.vertices[2].z = mesh.vertices[(i * 9) + 8];

			tri.normal = FaceNormal(tri.vertices);
			tri.id = i + offset;
			tris[i] = tri;
		}
	}
	
	return tris;
}

Vector3 BoxGetSize(BoundingBox box) {
	return (Vector3) { 
		box.max.x - box.min.x,
		box.max.y - box.min.y,
		box.max.z - box.min.z, 
	};	
}

float BoxGetSurfaceArea(BoundingBox box) {
    Vector3 size = BoxGetSize(box);
    return (size.x * size.y + size.x * size.z + size.y * size.z) * 2.0f;
}

Vector3 BoxGetCenter(BoundingBox box) {
	return (Vector3) {
		(box.min.x + box.max.x) * 0.5f,
		(box.min.y + box.max.y) * 0.5f,
		(box.min.z + box.max.z) * 0.5f,
	};
}

Vector3 TriCentroid(Polygon tri) {
	return (Vector3) {
		(tri.vertices[0].x + tri.vertices[1].x + tri.vertices[2].x) / 3.0f,
		(tri.vertices[0].y + tri.vertices[1].y + tri.vertices[2].y) / 3.0f,
		(tri.vertices[0].z + tri.vertices[1].z + tri.vertices[2].z) / 3.0f
	};
}

Plane TriToPlane(Polygon tri) {
	Plane plane;
	
	plane.normal = Vector3Normalize(Vector3CrossProduct(
							Vector3Subtract(tri.vertices[1], tri.vertices[0]),
							Vector3Subtract(tri.vertices[2], tri.vertices[0])) 
							);

	plane.d = -Vector3DotProduct(tri.vertices[0], plane.normal);

	return plane;
}

float PlaneDistance(Vector3 point, Plane plane) {
	return Vector3DotProduct(plane.normal, point) + plane.d;
}

Vector3 EdgeNormal(Vector3 p0, Vector3 p1) {
	Vector3 edge = Vector3Subtract(p1, p0);
	return Vector3Normalize((Vector3){ -edge.z, 0.0f, edge.x });
}

Polygon TriExpanded(Polygon *tri, float radius) {
	Polygon exp = *tri;

	for(uint8_t i = 0; i < 3; i++) {
		Vector3 v_next = tri->vertices[(i + 1) % 3];
		Vector3 v_prev = tri->vertices[(i + 2) % 3];
		Vector3 v_curr = tri->vertices[i];

		Vector3 norm_prev = EdgeNormal(v_prev, v_curr);
		Vector3 norm_next = EdgeNormal(v_curr, v_next);

		Vector3 avg = Vector3Normalize(Vector3Add(norm_prev, norm_next));

		exp.vertices[i] = Vector3Add(v_curr, Vector3Scale(avg, radius));
	}

	return exp;
}

void DebugDrawMapTris(Map *map) {
	for(uint16_t i = 0; i < map->poly_count; i++) {
		Polygon *tri = &map->polygons[i];
		DrawTriangle3D(tri->vertices[0], tri->vertices[1], tri->vertices[2], ColorBrightness(GREEN, -i * 0.01f));
	}
}

BvhNode *MakeBvhNode(Polygon *tris, uint32_t *tri_indices, uint32_t tri_count, uint32_t depth) {
    if(tri_count == 0) return NULL;

    BvhNode *node = malloc(sizeof(BvhNode));
    node->depth = depth;

    node->bounds = (BoundingBox){0};
    if(tri_count > 0) {
        Polygon *first_tri = &tris[tri_indices[0]];
        node->bounds.min = first_tri->vertices[0];
        node->bounds.max = first_tri->vertices[0];

        for(uint32_t i = 0; i < tri_count; i++) {
            Polygon *tri = &tris[tri_indices[i]];
            for(uint8_t j = 0; j < 3; j++) {
                node->bounds.min = Vector3Min(node->bounds.min, tri->vertices[j]);
                node->bounds.max = Vector3Max(node->bounds.max, tri->vertices[j]);
            }
        }
    }

    if(tri_count <= BVH_MAX_TRIS) {
        node->tri_indices = (uint32_t*)malloc(sizeof(uint32_t) * tri_count);
        memcpy(node->tri_indices, tri_indices, sizeof(uint32_t) * tri_count);
        node->tri_count = tri_count;
        node->children[0] = NULL;
        node->children[1] = NULL;
        return node;
    }

    short best_axis = -1;
    short best_bucket = -1;
    float best_cost = FLT_MAX;
    float best_split = 0.0f;
    Vector3 box_size = BoxGetSize(node->bounds);

    float size_vals[3] = { box_size.x, box_size.y, box_size.z };
    float min_vals[3]  = { node->bounds.min.x, node->bounds.min.y, node->bounds.min.z };
    float max_vals[3]  = { node->bounds.max.x, node->bounds.max.y, node->bounds.max.z };

    for(short axis = 0; axis < 3; axis++) {
        if(size_vals[axis] < EPSILON) continue;

        SahBucket buckets[SAH_BUCKET_COUNT];
        for(uint8_t i = 0; i < SAH_BUCKET_COUNT; i++) {
            buckets[i].bounds.min = (Vector3){ FLT_MAX, FLT_MAX, FLT_MAX };
            buckets[i].bounds.max = (Vector3){ -FLT_MAX, -FLT_MAX, -FLT_MAX };
            buckets[i].count = 0;
        }

        float axis_min = min_vals[axis];
        float axis_max = max_vals[axis];
        float bucket_size = (axis_max - axis_min) / SAH_BUCKET_COUNT;

        for(uint32_t i = 0; i < tri_count; i++) {
            Polygon *tri = &tris[tri_indices[i]];
            Vector3 centroid = TriCentroid(*tri);
            float centroid_vals[3] = { centroid.x, centroid.y, centroid.z };
            int bucket_id = (int)((centroid_vals[axis] - axis_min) / bucket_size);
            bucket_id = (int)Clamp(bucket_id, 0, SAH_BUCKET_COUNT - 1);
            for(uint8_t j = 0; j < 3; j++) {
                buckets[bucket_id].bounds.min = Vector3Min(buckets[bucket_id].bounds.min, tri->vertices[j]);
                buckets[bucket_id].bounds.max = Vector3Max(buckets[bucket_id].bounds.max, tri->vertices[j]);
            }
            buckets[bucket_id].count++;
        }

        for(uint8_t i = 1; i < SAH_BUCKET_COUNT; i++) {
            BoundingBox left_box = { .min = {FLT_MAX, FLT_MAX, FLT_MAX}, .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX} };
            BoundingBox right_box = { .min = {FLT_MAX, FLT_MAX, FLT_MAX}, .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX} };
            uint32_t left_count = 0, right_count = 0;

            for(uint8_t j = 0; j < i; j++) {
                left_box.min = Vector3Min(left_box.min, buckets[j].bounds.min);
                left_box.max = Vector3Max(left_box.max, buckets[j].bounds.max);
                left_count += buckets[j].count;
            }

            for(uint8_t j = i; j < SAH_BUCKET_COUNT; j++) {
                right_box.min = Vector3Min(right_box.min, buckets[j].bounds.min);
                right_box.max = Vector3Max(right_box.max, buckets[j].bounds.max);
                right_count += buckets[j].count;
            }

            float area_lft = BoxGetSurfaceArea(left_box);
            float area_rgt = BoxGetSurfaceArea(right_box);
            float area_total = BoxGetSurfaceArea(node->bounds);

            float cost = (
				SAH_TRAVEL_COST + 
				(area_lft / area_total) *
				left_count * SAH_INTERSECT_COST +
				(area_rgt / area_total) *
				right_count *
				SAH_INTERSECT_COST
			);

            if(cost < best_cost) {
                best_cost = cost;
                best_axis = axis;
                best_bucket = i;
                best_split = axis_min + i * bucket_size;
            }
        }
    }

    uint32_t left_count = 0;
    for(uint32_t i = 0; i < tri_count; i++) {
        Vector3 centroid = TriCentroid(tris[tri_indices[i]]);
        float centroid_vals[3] = { centroid.x, centroid.y, centroid.z };
        if(centroid_vals[best_axis] < best_split) {
            uint32_t temp = tri_indices[i];
            tri_indices[i] = tri_indices[left_count];
            tri_indices[left_count++] = temp;
        }
    }

    if(left_count == 0 || left_count == tri_count) left_count = tri_count / 2;

    node->children[0] = MakeBvhNode(tris, tri_indices, left_count, depth + 1);
    node->children[1] = MakeBvhNode(tris, tri_indices + left_count, tri_count - left_count, depth + 1);

    node->tri_count = 0;
    node->tri_indices = NULL;

    return node;
}

void BvhTraceNodes(BvhNode *node, Ray ray, BvhNode **hits, uint32_t *hit_count, uint32_t max_hits, float max_dist) {
	RayCollision coll = GetRayCollisionBox(ray, node->bounds);

	if(coll.hit && *hit_count < max_hits) {
		if(node->tri_count > 0) {
			// If node is leaf, add to hits

			if(coll.distance <= max_dist) hits[(*hit_count)++] = node;

		} else if(node->tri_count == 0) {
			// If node is a branch, test children	

			uint8_t next_a = 0;		// Index of first child to test 
			uint8_t next_b = 1;		// Index of second child to test 
		
			// Sort child nodes to test by proximity
			float dist_a = Vector3Distance(BoxGetCenter(node->children[0]->bounds), coll.point);
			float dist_b = Vector3Distance(BoxGetCenter(node->children[1]->bounds), coll.point);

			// Order by distance
			if(dist_a < dist_b) {
				next_a = 1;
				next_b = 0;
			}
			
			// Trace child nodes 
			BvhTraceNodes(node->children[next_a], ray, hits, hit_count, max_hits, max_dist);	
			BvhTraceNodes(node->children[next_b], ray, hits, hit_count, max_hits, max_dist);
		}
	}
}

void BvhBoxSweep(BvhNode *node, BoundingBox box, BvhNode **hits, uint32_t *hit_count, uint32_t max_hits) {
	if(CheckCollisionBoxes(node->bounds, box) && *hit_count < max_hits) {
		if(node->tri_count > 0) 
			hits[(*hit_count)++] = node;
		else 
			for(uint8_t i = 0; i < 2; i++) BvhBoxSweep(node->children[i], box, hits, hit_count, max_hits);
	}
}

void DrawBvhDebug(BvhNode *node, Polygon *all_tris) {
    if(!node) return;

    // Choose color based on depth (cycling through rainbow)
    Color colors[] = {RED, GREEN, BLUE, YELLOW, PURPLE, ORANGE, SKYBLUE};
    Color draw_color = colors[node->depth % (sizeof(colors)/sizeof(colors[0]))];
    draw_color.a = 128; // 50% transparency

    // Draw the bounding box
    DrawBoundingBox(node->bounds, draw_color);
	//DrawBvhNode(node);

    // Draw triangle centroids for leaf nodes
    if(node->tri_count > 0) {
        for(uint32_t i = 0; i < node->tri_count; i++) {
            Polygon tri = all_tris[node->tri_indices[i]];
            Vector3 centroid = TriCentroid(tri);
            //DrawSphere(centroid, 0.05f, draw_color);
        }
    }

    // Recursively draw children
    DrawBvhDebug(node->children[0], all_tris);
    DrawBvhDebug(node->children[1], all_tris);
}

void DrawTriNormals(Map *map) {
	for(uint32_t i = 0; i < map->poly_count; i++) {
		Polygon *tri = &map->polygons[i];

		Vector3 centroid = TriCentroid(*tri);
		DrawSphere(centroid, 0.025f, ColorAlpha(SKYBLUE, 0.5f));

		DrawLine3D(centroid, Vector3Add(centroid, Vector3Scale(tri->normal, 1.0f)), ColorAlpha(SKYBLUE, 0.5f));
	}
}

RayCollision GetRayTriCollision(Ray ray, Polygon *tri) {
	RayCollision coll = GetRayCollisionTriangle(ray, tri->vertices[0], tri->vertices[1], tri->vertices[2]);
	if(!coll.hit) coll = GetRayCollisionTriangle(ray, tri->vertices[0], tri->vertices[2], tri->vertices[1]);

	return coll;
}

