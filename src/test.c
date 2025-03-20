#include "flecs.h"
#include <stdio.h>

typedef struct {
    float x;
    float y;
} Position;

typedef struct {
    float x;
    float y;
} Velocity;

void move_sys(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 0);
    Velocity *v = ecs_field(it, Velocity, 1);

    char *type_str = ecs_table_str(it->world, it->table);
    printf("Move entities with [%s]\n", type_str);
    ecs_os_free(type_str);

    for (int i = 0; i < it->count; i++) {
        p[i].x += v[i].x * it->delta_time;
        p[i].y += v[i].y * it->delta_time;
        printf("Entity %llu: Pos (%.1f, %.1f) updated with Vel (%.1f, %.1f)\n",
               (unsigned long long)it->entities[i], p[i].x, p[i].y, v[i].x, v[i].y);
    }
}

int main(int argc, char *argv[]) {
    printf("Init test\n");
    ecs_world_t *world = ecs_init();

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);

    ecs_entity_t e = ecs_new(world);
    ecs_set(world, e, Position, {10, 20});
    ecs_set(world, e, Velocity, {1, 1});

    ECS_SYSTEM(world, move_sys, EcsOnUpdate, Position, Velocity);

    ecs_progress(world, 1.0f); // Use non-zero delta_time
    const Position *p1 = ecs_get(world, e, Position);
    printf("After 1 step, Position is {%.1f, %.1f}\n", p1->x, p1->y);

    ecs_progress(world, 1.0f);
    const Position *p2 = ecs_get(world, e, Position);
    printf("After 2 steps, Position is {%.1f, %.1f}\n", p2->x, p2->y);

    ecs_progress(world, 1.0f);
    const Position *p3 = ecs_get(world, e, Position);
    printf("After 3 steps, Position is {%.1f, %.1f}\n", p3->x, p3->y);

    ecs_fini(world);
    return 0;
}