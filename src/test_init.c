#include <stdio.h>
#include "flecs.h"

typedef struct {
    float x, y;
} Position, Velocity;

static void MoveSystem(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 0);
    Velocity *v = ecs_field(it, Position, 1);

    for (int i = 0; i < it->count; i++) {
        p[i].x += v[i].x * it->delta_time;
        p[i].y += v[i].y * it->delta_time;
        printf("Entity %llu: Pos (%.1f, %.1f) updated with Vel (%.1f, %.1f)\n",
               (unsigned long long)it->entities[i], p[i].x, p[i].y, v[i].x, v[i].y);
    }
}

int main() {
    ecs_world_t *world = ecs_init();
    printf("World initialized\n");

    printf("Flecs version: %s\n", FLECS_VERSION);

    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    printf("Components registered: Position ID %llu, Velocity ID %llu\n",
           (unsigned long long)ecs_id(Position), (unsigned long long)ecs_id(Velocity));

    ecs_entity_t sys = ecs_entity(world, { .name = "MoveSystem" });
    printf("System entity created: %llu\n", (unsigned long long)sys);
    ecs_add_id(world, sys, ecs_dependson(EcsOnUpdate));
    printf("Phase added with ecs_dependson(EcsOnUpdate)\n");
    ecs_system_init(world, &(ecs_system_desc_t){
        .entity = sys,
        .query.terms = {
            { .id = ecs_id(Position) },
            { .id = ecs_id(Velocity) }
        },
        .callback = MoveSystem
    });
    printf("MoveSystem registered\n");

    ecs_entity_t e = ecs_new(world);
    ecs_set(world, e, Position, {10.0f, 20.0f});
    ecs_set(world, e, Velocity, {1.5f, 2.5f});
    printf("Entity %llu created with Position (10.0, 20.0) and Velocity (1.5, 2.5)\n",
           (unsigned long long)e);

    for (int i = 0; i < 6; i++) {
        printf("Frame %d:\n", i + 1);
        ecs_progress(world, 1.0f);
    }

    ecs_fini(world);
    printf("World finalized\n");

    return 0;
}