// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "components.hpp"
#include "common.hpp"


void PhysicsSystem::step(float elapsed_ms)
{
	// Move coin based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_container = registry.motions;

	float step_seconds = elapsed_ms / 1000.f;

	// Update player movement displacement
	Entity player = registry.players.entities[0];
	auto& player_component = registry.players.components[0];
	Motion& player_motion = motion_container.get(player);
	vec2 old_player_position = player_motion.position;
	player_motion.position += player_component.speed * player_component.move * vec2({ -1.0f, 1.0f }) * step_seconds;
	vec2 diff = player_motion.position - old_player_position;

	// Handle parallax
	updateParallaxBackground(elapsed_ms, diff.x);

	for(uint i = 0; i < motion_container.size(); i++)
	{
		Motion& motion = motion_container.components[i];
		Entity entity = motion_container.entities[i];
		motion.position += motion.velocity * step_seconds;

		// Handle tensions
		if (registry.tensions.has(entity)) {
			auto& tension = registry.tensions.get(entity);
			auto& motion_other = motion_container.get(tension.center);
			auto dist = distance(motion.position, motion_other.position) - tension.l;
			// Required to prevent division by zero
			if (dist != 0.f) {
				auto direct = normalize( motion_other.position - motion.position);
				motion.position += tension.k * dist * direct * step_seconds;
			}
		}

		// Handle gravities
		if (registry.gravities.has(entity)) {
			auto& gravity = registry.gravities.get(entity);
			auto& motion_other = motion_container.get(gravity.center);
			auto d = motion.position - motion_other.position;
			auto dist = dot(d, d);
			// Required to prevent division by zero
			if (dist != 0.f) {
				auto direct = normalize(motion_other.position - motion.position);
				motion.position += gravity.g / dist * direct * step_seconds;
			}
		}

		// Handle physics
		if (registry.physics.has(entity)) {
			auto& physics = registry.physics.get(entity);
			motion.velocity += physics.gravity * step_seconds;
		}

		// Handle knock backs
		if (registry.knockbacks.has(entity)) {
			auto& knock_back = registry.knockbacks.get(entity);
			// Set velocity in direction of knock back
			motion.velocity = knock_back.direction * knock_back.power;
			motion.position += knock_back.direction * elapsed_ms / 1000.f;
		}

		
	}
}

void PhysicsSystem::updateParallaxBackground(float elapsed_ms, float playerSpeed)
{
	// Assuming you have background layers as part of your registry
	auto& background_layers = registry.parallaxes.entities;


	// Update each background layer based on parallax speeds
	for (auto& entity : background_layers)
	{
		Motion& layer_motion = registry.motions.get(entity);
		Parallax& parallax_effect = registry.parallaxes.get(entity);
		float parallax_speed = parallax_effect.parallaxSpeed;
		if (parallax_effect.reflected) {
			parallax_speed = 0 - parallax_speed;
		}

		// Adjust the parallax speed based on the player's speed
		float adjustedParallaxSpeed = parallax_speed * playerSpeed;


		// Update the background layer's position
		layer_motion.position += adjustedParallaxSpeed * vec2({ -1.0f, 0.0f }) * elapsed_ms/1000.f;

		if (layer_motion.position.x < -layer_motion.scale.x && !registry.tutorialBackgrounds.has(entity))
		{
			layer_motion.position.x += layer_motion.scale.x;
		}
		
		if (layer_motion.position.x < -4100.f && registry.tutorialBackgrounds.has(entity)) {
			layer_motion.position.x += 600.f;
		}

		if (layer_motion.position.x > layer_motion.scale.x && !registry.tutorialBackgrounds.has(entity))
		{
			layer_motion.position.x -= layer_motion.scale.x;
		}

		if (layer_motion.position.x > -2100.f && registry.tutorialBackgrounds.has(entity)) {
			layer_motion.position.x -= 600.f;
		}

	}


}
