#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "world_init.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window

	GLFWwindow* create_window(int window_width, int window_height);

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// For enemies to drop coins
	void dropItem(vec2 position);


	// same functionality as drop item but no sound
	void dropItemMuted(vec2 position);

	// For enemies to health potions
	void dropHealthPotion(vec2 position);


	// For barrels, vases and boxes to drop weapons in the platform level
	void WorldSystem::dropWeapon(vec2 position);

	// For player to drop items
	void dropInventoryItem();
	
	// Check for collisions
	void handle_collisions(float elapsed_ms);

	// Check and handle collisions against the player
	void handle_player_collisions(float elapsed_ms);

	// helper for attacks
	void handle_attack_req();

	bool isButtonPressed(Entity button);

	// Should the game be over ?
	bool is_over()const;

	// Updates ui to reflect game state
	void rerenderUI(vec2 inventory_pos);

	// Move the player to the next room
	void transition_room(ROOM room_name);

	// World running speed. Set to 0 to pause.
	float current_speed;

	// Total time run by the world
	float since_start_ms = 0.f;

	// Difficulty scaling used to scale game difficulty
	float difficulty_scaling = 1.f;

	// Projectile damage
	float projectile_damage = 15.f;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* death_sound;
	Mix_Chunk* coin_sound;
	Mix_Chunk* player_jump_sound;
	Mix_Chunk* zap_sound;
	Mix_Chunk* drop_item_sound;
	Mix_Chunk* boss_ability1_sound;
	Mix_Chunk* boss_ability2_sound;
	Mix_Chunk* boss_ability3_sound;
	Mix_Chunk* boss_death_sound;


private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button(int button, int action, int mods);
	void on_inventory_selection_change();
	void on_item_pickup(bool isSuccessful, ITEM_TYPE itemType);

	// restart level
	void restart_game();

	// OpenGL window handle
	GLFWwindow* window;

	// Number of points collected by the player, displayed in the window title
	unsigned int points;

	// Track if collisions should be processed
	bool ignore_collision = false;

	// Game state
	RenderSystem* renderer;
	float next_enemy_spawn;
	Entity player;
	Entity camera;
	Entity current_room;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// Builder functions that create rooms.
	std::map<ROOM, std::function<Entity()>> room_builders;

	void player_jump();
};
