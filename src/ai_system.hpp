#pragma once

#include <vector>


#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_system.hpp"
#include "world_init.hpp"

// Constants for Boss
const float ABILITY_COOLDOWN = 10000.f;
const float COW_SPAWNING_DURATION = 3000.f;
const float SPEEDING_DURATION = 1750.f;
const float RANGE_ATTACKING_DURATION = 3000.f;

class AISystem{
public:
	AISystem();
	void step(float elapsed_ms);

	void init (WorldSystem* world_system);

private: 
	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// Builder functions that create rooms.
	std::map<ROOM, std::function<Entity()>> room_builders;

	RenderSystem* renderer;
	float next_enemy_spawn = 200.f;
	float speed_up_prep = 1000.f;
	float next_range_attack = 200.f;

	WorldSystem* world_system;

	int chooseNextAbility(Boss& boss, Motion& boss_motion, Player& player, Motion& player_motion);

};
