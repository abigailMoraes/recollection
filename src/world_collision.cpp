#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include "physics_system.hpp"
#include "particle_system.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
inline vec2 get_bounding_box(const Motion& motion)
{
	return abs(motion.scale) / 2.f;
}


mat3 translate(mat3 transform, vec2 offset)
{
	mat3 T = { { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ offset.x, offset.y, 1.f } };
	transform = transform * T;
	return transform;
}


mat3 scale(mat3 transform, vec2 scale)
{
	mat3 S = { { scale.x, 0.f, 0.f },{ 0.f, scale.y, 0.f },{ 0.f, 0.f, 1.f } };
	transform = transform * S;
	return transform;
}

// Resolve collision between an immovable structure and a movable object.
void resolve_static_collision(const Motion& structure, Motion& movable) {
	// Handles based on from which direction player is in contact.
	float x_resolve = 0.f;
	float y_resolve = 0.f;

	vec2&& dist = glm::abs(structure.position - movable.position);
	const vec2 sum_vec = get_bounding_box(structure) + get_bounding_box(movable) - dist;

	if (sum_vec == vec2{0.f, 0.f}) {
		movable.velocity = vec2{0.f, 0.f};
		return;
	}

	if (sum_vec.x > 0.f) {
		// If the object is to the right, it's always better to resolve by moving it to the right.
		if (movable.position.x > structure.position.x) {
			float&& static_edge = structure.position.x + abs(structure.scale.x) / 2;
			float&& movable_edge = movable.position.x - abs(movable.scale.x) / 2;
			x_resolve = static_edge - movable_edge;
		} else {
			float&& static_edge = structure.position.x - abs(structure.scale.x) / 2;
			float&& movable_edge = movable.position.x + abs(movable.scale.x) / 2;
			x_resolve = static_edge - movable_edge;
		}
	}
	if (sum_vec.y > 0.f) {
		if (movable.position.y > structure.position.y) {
			float &&static_edge = structure.position.y + abs(structure.scale.y) / 2;
			float &&movable_edge = movable.position.y - abs(movable.scale.y) / 2;
			y_resolve = static_edge - movable_edge;
		} else {
			float &&static_edge = structure.position.y - abs(structure.scale.y) / 2;
			float &&movable_edge = movable.position.y + abs(movable.scale.y) / 2;
			y_resolve = static_edge - movable_edge;
		}
	}

	if (sum_vec.x <= 0.f) {
		movable.position.y += y_resolve;
		movable.velocity.y = 0.f;
		return;
	}

	// If they don't overlap in y
	if (sum_vec.y <= 0.f) {
		movable.position.x += x_resolve;
		movable.velocity.x = 0.f;
		return;
	}

	// If they overlap in both x and y, choose the smallest adjustment required.
	if (abs(x_resolve) < abs(y_resolve)) {
		movable.position.x += x_resolve;
		movable.velocity.x = 0.f;
	} else {
		movable.position.y += y_resolve;
		movable.velocity.y = 0.f;
	}
}

inline float is_left(const vec2& p1, const vec2& p2, const vec2& p3) {
	return (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y);
}

inline bool is_point_between(float a, float b, float c) {
	return (a <= b && b < c) || (c <= b && b < a);
};

bool is_point_inside_triangle(const vec2& point, const vec2& v0, const vec2& v1, const vec2& v2) {
	int wn = 0;  // Winding number

	if (point.y >= std::min(v0.y, std::min(v1.y, v2.y)) &&
	    point.y <= std::max(v0.y, std::max(v1.y, v2.y))) {
		if (is_left(v0, v1, point) > 0 && is_left(v1, v2, point) > 0 && is_left(v2, v0, point) > 0) {
			wn = 1;
		} else if (is_left(v0, v1, point) < 0 && is_left(v1, v2, point) < 0 && is_left(v2, v0, point) < 0) {
			wn = -1;
		} else if (is_point_between(v0.y, point.y, v1.y) && v0.y != v1.y) {
			wn += (v0.x < v1.x) ? 1 : -1;
		} else if (is_point_between(v1.y, point.y, v2.y) && v1.y != v2.y) {
			wn += (v1.x < v2.x) ? 1 : -1;
		} else if (is_point_between(v2.y, point.y, v0.y) && v2.y != v0.y) {
			wn += (v2.x < v0.x) ? 1 : -1;
		}
	}

	return wn != 0;
}

// Check collision between two transformed meshes
// Makes an assumption that meshes are convex, and the vertices are already sorted.
bool verify_collision(const Mesh& mesh_i, const Mesh& mesh_j, const mat3& objectTransform_i, const mat3& objectTransform_j) {
	for (int vertex_i = 0; vertex_i < mesh_i.vertices.size(); vertex_i += 3) {
		const auto& v1 = objectTransform_i * mesh_i.vertices[mesh_i.vertex_indices[vertex_i]].position;
		const auto& v2 = objectTransform_i * mesh_i.vertices[mesh_i.vertex_indices[vertex_i+1]].position;
		const auto& v3 = objectTransform_i * mesh_i.vertices[mesh_i.vertex_indices[vertex_i+2]].position;
		for (const auto & vertex : mesh_j.vertices)
			if (is_point_inside_triangle(objectTransform_j * vertex.position, v3, v2, v1))
				return true;
	}
	return false;
}

// AABB collision detection
inline bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2&& dist = glm::abs(motion1.position - motion2.position);
	const vec2 sum_vec = get_bounding_box(motion1) + get_bounding_box(motion2) - dist;

	return sum_vec.x > 0 && sum_vec.y > 0;
}


// Function to check collision between a vertex and a bounding sphere
bool vertexSphereCollision(const ColoredVertex& vertex, const Motion& motion, float sphereRadius) {
	// Check if the vertex is within the sphere
	float distanceSquared = dot(vec2(vertex.position.x, vertex.position.y) - motion.position, vec2(vertex.position.x, vertex.position.y) - motion.position);
	return distanceSquared < (sphereRadius* sphereRadius);
}

// Function to check collision between two motions using vertex mesh and bounding sphere
bool vertexMeshSphereCollision(const Motion& motion, const std::vector<ColoredVertex>& vertexMesh) {
	const vec2 other_bonding_box = get_bounding_box(motion);
	const float r_squared = dot(other_bonding_box, other_bonding_box);
	// Iterate through each vertex of the mesh and check for collision
	return std::any_of(vertexMesh.begin(), vertexMesh.end(),
	                   [&motion, &r_squared](const ColoredVertex& vertex)
	                   {return vertexSphereCollision(vertex, motion, r_squared);});
}


void WorldSystem::handle_player_collisions(float elapsed_ms) {
	auto& motion_container = registry.motions;

	if (registry.deathTimers.has(player)) return;

	// Update player movement displacement
	Motion& player_motion = motion_container.get(player);
	Player& player_component = registry.players.get(player);

	for(uint i = 0; i < motion_container.size(); i++) {
		Motion& motion = motion_container.components[i];
		Entity entity = motion_container.entities[i];
		if (!collides(player_motion, motion)) continue;

		// Checking Player - HardShell collisions
		if (registry.hardShells.has(entity) && player_component.vul) {
			// if the hardshell is a projectile, remove it and deal 20 damage to the player
			if (registry.hardShells.get(entity).isProjectile) {
				float damage_taken = projectile_damage - (player_component.defense * 0.5);
				player_component.hp -= damage_taken;
				registry.remove_all_components_of(entity);
			}
			else {
				// else, does damage to the player the same as before
				player_component.hp -= 1.f;
			}
		}
		// Checking Player - SoftShell collisions
		else if (registry.softShells.has(entity)) {
			// If a cow is collected, remove collision handling against player.
			if (registry.cows.has(entity)) {
				registry.softShells.remove(entity);

				// Leash the cow to the last cow or the player
				if (registry.capturedCows.size())
					registry.tensions.emplace(entity, Tension{registry.capturedCows.entities.back(), 1.f, 100.f});
				else
					registry.tensions.emplace(entity, Tension{player, 1.f, 100.f});

				registry.capturedCows.emplace(entity);
				continue;
			}

			registry.expiry.emplace(entity, Expiry{-1.f});
			Mix_PlayChannel(-1, coin_sound, 0);
			++points;

			continue;
		}
		// Checking Player - Collectable collisions
		else if (registry.collectables.has(entity)) {
			// If not dying
			// Collect if either it doesn't require key input to collect
			// or if it does, check if the player has the key pressed.
			Collectable& collectable = registry.collectables.get(entity);
			if (!collectable.require_key_press ||
			    (player_component.in_pick_up_state && collectable.inventoryCollectable)) {
				// If the collectable has a coin value, add it to the score
				if (registry.coinValues.has(entity)) {
					points += registry.coinValues.get(entity).value;
					registry.expiry.emplace(entity, Expiry{-1.f});
				}
				if (registry.healthPotions.has(entity)) {
					player_component.hp = min(100.f, player_component.hp + 5.f);
					registry.expiry.emplace(entity, Expiry{ -1.f });
				}
				// Collect the item (remove item (don't render it), and check to add to 'inventory' collection)
				if (collectable.inventoryCollectable) {
					Inventory& playerInventory = registry.inventories.get(player);
					if (playerInventory.emptySlots.size() > 0) {
						int collectIndex = playerInventory.emptySlots[playerInventory.emptySlots.size() - 1];
						playerInventory.emptySlots.pop_back();

						playerInventory.items[collectIndex] = (entity);

						ITEM_TYPE itemType = registry.items.get(entity).type;
						on_item_pickup(true, itemType);

						// Remove the entity from rendering and make it uncollectable.
						registry.renderRequests.remove(entity);
						registry.collectables.remove(entity);
					}
					else {
						on_item_pickup(false, ITEM_TYPE::PISTOL);
					}
				}
				vec2 inventory_pos = { window_width_f / 2.f - window_width_f / 20.f, window_height_f / 8.f };
				rerenderUI(inventory_pos);
				Mix_PlayChannel(-1, coin_sound, 0);
			}

			if (registry.keys.has(entity)) {
				Mesh mesh_i = *registry.collisionMeshPtrs.get(entity);
				Mesh mesh_j = *registry.collisionMeshPtrs.get(player);

				mat3 objectTransform_i(1.f);
				objectTransform_i = translate(objectTransform_i, motion.position);
				objectTransform_i = scale(objectTransform_i, motion.scale);

				// repeat for j entity
				mat3 objectTransform_j(1.f);
				objectTransform_j = translate(objectTransform_j, player_motion.position);
				objectTransform_j = scale(objectTransform_j, player_motion.scale);

				if (verify_collision(mesh_i, mesh_j, objectTransform_i, objectTransform_j)) {
					player_component.keys++;
					registry.remove_all_components_of(entity);
					--i;
					continue;
				}
			}
		}
			// Checking Player - Door collisions
		else if (registry.doors.has(entity) && registry.doors.get(entity).type == DOORTYPE::OPEN) {
			// For now, just create basic room
			transition_room(registry.doors.get(entity).to);
			return;
		}
			// Checking Player - Platform Collisions
		else if (registry.meshPlatforms.has(entity)) {
			// Assume AABB collision detection, and axis-aligned meshes.
			Motion& mesh_platform_motion = registry.motions.get(entity);
			resolve_static_collision(mesh_platform_motion, player_motion);

			// if player is colliding with a moving platform, the player's velocity is set to the platform's.
			if (registry.bounces.has(entity)) {
				Bounce& bounce = registry.bounces.get(entity);
				// Set the player's velocity to the platform's velocity
				player_motion.velocity = bounce.get_velocity(since_start_ms);
			}

			player_component.jumps_done = 0;
		}
			// Checking Player - Lever Collisions
		else if (registry.levers.has(entity)) {
			Lever& lever = registry.levers.get(entity);
			if (lever.state != LEVERSTATE::UNTOUCHED)
				continue;
			lever.elapsedTime += elapsed_ms * current_speed;
			
			auto& lever_door_comp = registry.leverDoorLinks.get(entity);
			auto& platform_door = registry.doors.get(lever_door_comp.door);
			lever.state = LEVERSTATE::PULLED;
			platform_door.type = DOORTYPE::OPEN;
		}
			// Checking Player - Chest Collisions
		else if (registry.chests.has(entity)) {
			Chest& chest = registry.chests.get(entity);
			if (chest.state == CHESTSTATE::CLOSED && player_component.keys > 0) {
				chest.elapsedTime += elapsed_ms * current_speed;
				chest.state = CHESTSTATE::OPEN;
				player_component.keys -= 1;
				if (uniform_dist(rng) < 0.1f)
					dropWeapon(registry.motions.get(entity).position);
				if (uniform_dist(rng) < 0.3f)
					dropHealthPotion(registry.motions.get(entity).position);
				dropItem(registry.motions.get(entity).position + vec2({ 0.f,-100.f }));
				dropItem(registry.motions.get(entity).position + vec2({ -10.f,-100.f }));
				dropItem(registry.motions.get(entity).position + vec2({ 30.f,-100.f }));
				dropItem(registry.motions.get(entity).position + vec2({ -10.f,-100.f }));
				dropItem(registry.motions.get(entity).position + vec2({ 30.f,-100.f }));
				dropHealthPotion(registry.motions.get(entity).position + vec2({ 30.f,-30.f }));
			}
		}
	}

	// Kill player
	if (player_component.hp <= 0.f) {


		if (!registry.lifebars.entities.empty()) {
			auto& life_bar_entity = registry.lifebars.entities.back();
			registry.remove_all_components_of(life_bar_entity);
		}

		if (!registry.lifebars.entities.empty()) {
			auto& life_bar_entity = registry.lifebars.entities.back();
			registry.remove_all_components_of(life_bar_entity);
		}

		if (!registry.lifeBarContainers.entities.empty()) {
			auto& life_bar_container_entity = registry.lifeBarContainers.entities.back();
			registry.remove_all_components_of(life_bar_container_entity);
		}

		if (!registry.lifeBarContainers.entities.empty()) {
			auto& life_bar_container_entity = registry.lifeBarContainers.entities.back();
			registry.remove_all_components_of(life_bar_container_entity);
		}

		if (!registry.playerSymbols.entities.empty()) {
			auto& player_symbol = registry.playerSymbols.entities.back();
			registry.remove_all_components_of(player_symbol);
		}

		if (!registry.cowBossSymbols.entities.empty()) {
			auto& cow_boss_symbol = registry.cowBossSymbols.entities.back();
			registry.remove_all_components_of(cow_boss_symbol);
		}

		createLifeBar(renderer, { window_width_f - window_width_f / 5.f, window_height_f - window_height_f / 8.f }, vec2({ 320.f, 35.f }), PLAYER_MAX_HP, 0.f);
		renderPlayerSymbol(renderer, { window_width_f - window_width_f / 5.f - 220.f, window_height_f - window_height_f / 8.f }, vec2({ -PLAYER_BB_WIDTH * 2, PLAYER_BB_HEIGHT * 2 }));

		// Render cow Boss Lifebar
		if (registry.boss.size() > 0) {
			auto& cowBoss_component = registry.boss.entities.back();
			auto& cowBossHit_compoenent = registry.hits.get(cowBoss_component);
			auto& cowBossMotion_component = registry.motions.get(cowBoss_component);

			// Create a lifebar entity initialized with the player component
			createLifeBar(renderer, { window_width_f / 5.f, window_height_f - window_height_f / 8.f }, vec2({ 320.f, 35.f }), COW_BOSS_MAX_HP, cowBossHit_compoenent.hp);
			renderCowBossSymbol(renderer, { window_width_f / 5.f - 260.f, window_height_f - window_height_f / 8.f - 30.f }, vec2({ -COIN_BB_WIDTH * 10, COIN_BB_HEIGHT * 10 }));
		}


		registry.deathTimers.emplace(player);
		player_motion.velocity = vec2({ 0.f, 0.f });

		player_component.state = PLAYER_STATE::DEAD;
		player_component.move = {{0.f, 0.f}, {0.f, 0.f}};

		// Halt BGM
		Mix_HaltMusic();
		Mix_PlayChannel(-1, death_sound, 0);
		registry.colors.get(player) = vec3({ 0.3f, 0.3f, 0.3f });
	}
}

// Compute collisions between entities
void WorldSystem::handle_collisions(float elapsed_ms_since_last_update) {
	// Don't process collisions if the ignore flag is on.
	if (ignore_collision) return;

	handle_player_collisions(elapsed_ms_since_last_update);

	// ASSUMPTION: all attacks are player attacks and all hits represent entities hittable by and only by the player.
	for (size_t a = 0; a < registry.attacks.size(); a++) {
		auto attack_entity = registry.attacks.entities[a];
		auto& attack = registry.attacks.components[a];
		for (size_t h = 0; h < registry.hits.size(); h++) {
			auto hit_entity = registry.hits.entities[h];
			auto& hit = registry.hits.components[h];
			Motion& hit_motion = registry.motions.get(hit_entity);
			Motion& attack_motion = registry.motions.get(attack_entity);

			if (!collides(hit_motion, attack_motion)) continue;

			hit.hp -= attack.damage;

			// Add knockback effect if enemy.
			// TODO: adding knockback to any collidables object for testing
			if (registry.ai.has(hit_entity) || registry.collidables.has(hit_entity)) {
				// Check for existing knockback component
				if (!registry.knockbacks.has(hit_entity) && !registry.boss.has(hit_entity)) {
					KnockBack& knockBack = registry.knockbacks.emplace(hit_entity);
					knockBack.direction = hit_motion.position - attack_motion.position;
					knockBack.power = 3.f;
					knockBack.timer_ms = 100.f;
				}
			}

			// Add enemy death sound effect
			if (!attack.aoe) {
				registry.remove_all_components_of(attack_entity);
				// Move back and retest.
				a--;
				break;
			}
		}
	}

	for (size_t i = 0; i < registry.hits.size(); i++)
	{
		// If health is low, kill it.
		auto& entity = registry.hits.entities[i];
		auto& hit = registry.hits.components[i];
		auto& motion = registry.motions.get(entity);

		if (hit.hp <= 0.f) {
			//spawn coin if enemy
			// handle slime death
			if (registry.slimes.has(entity) && registry.slimes.get(entity).isBigSlime) {
				bool platform = !registry.rooms.get(current_room).topdown;
				int numSmallSlimes = 1 + (int)(uniform_dist(rng) * 3); // Randomize number of small slimes to spawn
				for (int j = 0; j < numSmallSlimes; j++) {
					Entity smallSlime = createEnemySlime(renderer, motion.position, false);
					Motion& slime_motion = registry.motions.get(smallSlime);
					motion.velocity = { registry.motions.get(entity).velocity.x + uniform_dist(rng), registry.motions.get(entity).velocity.y + uniform_dist(rng) };
					motion.angle = registry.motions.get(entity).angle;
					registry.ai.emplace(smallSlime);
					registry.hits.emplace(smallSlime);
					if (platform)
						registry.physics.emplace(smallSlime);
				}
			}
			else if (!(registry.hardShells.has(entity) || registry.softShells.has(entity))){
				if (uniform_dist(rng) < 0.005f)
					dropWeapon(registry.motions.get(entity).position);
				if (uniform_dist(rng) < 0.1f)
					dropHealthPotion(registry.motions.get(entity).position);
			}
			else {
				// check if boss is died, if so drop coins, and create a door to the first level
				if (registry.rooms.get(current_room).room_type == ROOM::BOSS && registry.boss.has(entity)) {
					createDoor(renderer, { (uniform_dist(rng) - .5f) * 100, (uniform_dist(rng) - .5f) * 100 },
						{ 200.f, 200.f }, ROOM::FIRST);
					// randomly drop a lot of coins
					for (int i = 0; i < 50 + (int)(uniform_dist(rng) * 50); i++) {
						dropItemMuted({ registry.motions.get(entity).position.x + (int)(uniform_dist(rng) * 400), registry.motions.get(entity).position.y + (int)(uniform_dist(rng) * 400) });
					}

					// play boss death sound
					Mix_PlayChannel(-1, boss_death_sound, 0);
				}
				else if (!registry.hardShells.get(entity).isProjectile)
					dropItem(registry.motions.get(entity).position);
			}
			registry.remove_all_components_of(entity);
			i--;
		}
	}

	if (registry.rooms.get(current_room).topdown) return;

	for (uint i = 0; i < registry.meshPlatforms.entities.size(); i++) {
		Entity platform = registry.meshPlatforms.entities[i];
		const Motion& platform_motion = registry.motions.get(platform);
		for (uint j = 0; j < registry.ai.entities.size(); j++) {
			Entity enemy = registry.ai.entities[j];
			Motion& enemy_motion = registry.motions.get(enemy);

			if (!collides(platform_motion, enemy_motion)) continue;

			resolve_static_collision(platform_motion, enemy_motion);
		}

		for (Entity coin : registry.coinValues.entities) {
			Motion& coin_motion = registry.motions.get(coin);
			if (!collides(platform_motion, coin_motion)) continue;

			resolve_static_collision(platform_motion, coin_motion);
		}
	}
}


inline void separateEntities(const Entity& entity1, const Entity& entity2) {
	// Get the motion components of both entities
	Motion& motion1 = registry.motions.get(entity1);
	Motion& motion2 = registry.motions.get(entity2);

	// Calculate the separation vector
	vec2 separationVector = motion1.position - motion2.position;
	float distance = length(separationVector);
	float overlap = 100.f - distance; // Minimum distance between entities (adjust as needed)

	if (overlap > 0) {
		// Separate entities along the separation vector
		vec2 separationDirection = normalize(separationVector);
		motion1.position += separationDirection * (overlap / 2.0f);
		motion2.position -= separationDirection * (overlap / 2.0f);
	}
}