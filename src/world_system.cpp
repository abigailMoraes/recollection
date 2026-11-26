#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <stdio.h>

#include "physics_system.hpp"
#include "particle_system.hpp"

// Game configuration

const size_t MAX_ENEMY = 1000;
const size_t ENEMY_DELAY_BASE_MS = 200;
const float COIN_PULL_DISTANCE = 200.0f;
const float COIN_GRAVITY = 50.f;
const size_t REQUIRED_COWS = 5;

// Create the world
WorldSystem::WorldSystem()
	: points(0)
	, next_enemy_spawn(0.f){
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (death_sound != nullptr)
		Mix_FreeChunk(death_sound);
	if (coin_sound != nullptr)
		Mix_FreeChunk(coin_sound);
	if (player_jump_sound != nullptr)
		Mix_FreeChunk(player_jump_sound);
	if (zap_sound != nullptr)
		Mix_FreeChunk(zap_sound);
	if (drop_item_sound != nullptr)
		Mix_FreeChunk(drop_item_sound);
	if (boss_ability1_sound != nullptr)
		Mix_FreeChunk(boss_ability1_sound);
	if (boss_ability2_sound != nullptr)
		Mix_FreeChunk(boss_ability2_sound);
	if (boss_ability3_sound != nullptr)
		Mix_FreeChunk(boss_ability3_sound);
	if (boss_death_sound != nullptr)
		Mix_FreeChunk(boss_death_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();
	room_builders.clear();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer

GLFWwindow* WorldSystem::create_window(int window_width, int window_height)  {

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Initializing window
	
	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width, window_height, "Recollection", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_redirect = [](GLFWwindow* wnd, int button, int action, int mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button(button, action, mods);};
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	death_sound = Mix_LoadWAV(audio_path("death.wav").c_str());
	coin_sound = Mix_LoadWAV(audio_path("coin.wav").c_str());
	player_jump_sound = Mix_LoadWAV(audio_path("jump.wav").c_str());
	zap_sound = Mix_LoadWAV(audio_path("zap.wav").c_str());
	drop_item_sound = Mix_LoadWAV(audio_path("drop_item.wav").c_str());
	boss_ability1_sound = Mix_LoadWAV(audio_path("boss_ability1.wav").c_str());
	boss_ability2_sound = Mix_LoadWAV(audio_path("boss_ability2.wav").c_str());
	boss_ability3_sound = Mix_LoadWAV(audio_path("boss_ability3.wav").c_str());
	boss_death_sound = Mix_LoadWAV(audio_path("boss_death_sound.wav").c_str());


	if (background_music == nullptr || death_sound == nullptr || coin_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds. make sure the data directory is present\n");
		printf("%s\n", Mix_GetError());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;

	// Updating window title
	glfwSetWindowTitle(window, "Recollection");

	// Install room builders
	room_builders[ROOM::ROSEGARDEN] = [this]() -> Entity {
		Entity e = createRoseGardenRoom(renderer,
			Room{ "Level 1 : Rosegarden: Exit to your right",
				 {-3000.f, 0.f},
				 vec2{ 10000.f, 2000.f },
				 false,
				 ROOM::ROSEGARDEN
			});
		registry.motions.get(player).zero();
		registry.motions.get(player).position.x = -4000.f;
		registry.motions.get(camera).zero();
		registry.motions.get(camera).position.x = -4000.f;

		return e;
	};

	room_builders[ROOM::FIRST] = [this]() -> Entity {
		Entity e = createRoom(renderer,
			Room{ "Level 2: FIND THE EXIT AND QUICK",
				 {0.f, 0.f},
				 vec2{ 5000.f, 5000.f },
				 true,
				 ROOM::FIRST
			});
		
		createDoor(renderer, { (uniform_dist(rng) - .5f) * 5000, (uniform_dist(rng) - .5f) * 5000 },
			{ 200.f, 200.f }, ROOM::SECOND);

		// ItemStat: {attack, speed, health, defense}
//		createGroundItem(renderer, { 0.f, 0.f }, { 1, ITEM_TYPE::SWORD, {3, 0, 0, 0} });
//		createGroundItem(renderer, { 0.f, 5.f }, { 1, ITEM_TYPE::LASER_GUN, {3, 0, 0, 0} });
//		createGroundItem(renderer, { 0.f, 10.f }, { 1, ITEM_TYPE::PISTOL, {3, 0, 0, 0} });
//		createGroundItem(renderer, { 0.f, 15.f }, { 1, ITEM_TYPE::ASSAULT_RIFLE, {3, 0, 0, 0} });
//		createGroundItem(renderer, { 0.f, 20.f }, { 1, ITEM_TYPE::SHOTGUN, {3, 0, 0, 0} });
		createGroundItem(renderer, {0.f, 100.f}, {1, ITEM_TYPE::FEATHER, {0, 50, 0, -20}});
		createGroundItem(renderer, { 0.f, 200.f }, {1, ITEM_TYPE::STONE, {0, -50, 0, 20}});
		createGroundItem(renderer, { 0.f, 300.f }, {1, ITEM_TYPE::GOLDEN_EGG, {0, 20, 0, 10}});

		registry.motions.get(player).zero();
		registry.motions.get(camera).zero();
		
		return e;
	};

	room_builders[ROOM::SECOND] = [this]() -> Entity {
		Entity e = createPlatformRoom(renderer,
			Room{ "Level 3: RUN RUN RUN TO THE RIGHT",
				 {0.f, 0.f},
				 vec2{ 10000.f, 2000 },
				 false,
				 ROOM::SECOND
			});
		registry.motions.get(player).zero();
		registry.motions.get(player).position.x = -3500.f;
		registry.motions.get(camera).zero();
		registry.motions.get(camera).position.x = -3500.f;

		return e;
	};

	room_builders[ROOM::THIRD] = [this]() -> Entity {
		Entity e = createRoom(renderer,
		                      Room{"Level 3: Survive for 3 minutes",
		                           {0.f, 0.f},
		                           vec2{ 100'00.f, 100'00.f },
		                           true,
		                           ROOM::FIRST
		                      });

		// Start a 3 minute timeout
		registry.timeouts.emplace(Entity(), Timeout{3.f * 60.f * 1'000.f, [this]() -> void {
			transition_room(ROOM::BOSS);
		}});

		registry.motions.get(player).zero();
		registry.motions.get(camera).zero();

		difficulty_scaling = 10.f;

		return e;
	};


	room_builders[ROOM::COW] = [this]() -> Entity {
		Entity e = createRoom(renderer,
		                      Room{"Cow Level: How did you even get here?",
		                           {0.f, 0.f},
		                           vec2{ 10000.f, 10000.f },
		                           true,
								   ROOM::COW
		                      });

		registry.motions.get(player).zero();
		registry.motions.get(camera).zero();
		return e;
	};

	room_builders[ROOM::BOSS] = [this]() -> Entity {
		Entity e = createRoom(renderer,
		                      Room{"Final Level: DESTROY. THEM. ALL.",
		                           {0.f, 0.f},
		                           vec2{ 2000.f, 1500.f },
		                           true,
		                           ROOM::BOSS
		                      });

		registry.motions.get(player).zero();
		registry.motions.get(camera).zero();
		return e;
	};


	
	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	float tick_ms = elapsed_ms_since_last_update * current_speed;

	// Update time since start
	since_start_ms += tick_ms;
	// Increase by 1 per minute. Enemies are 10 times stronger and spawn 10 times faster after 9 minutes.
	difficulty_scaling = 1.f + since_start_ms * 0.001f / 60.f;

	auto& room = registry.rooms.get(current_room);
	const bool topdown = registry.rooms.get(current_room).topdown;

	Motion& player_motion = registry.motions.get(player);
	auto& player_component = registry.players.get(player);
	int keys = player_component.keys;

	// Modify Player stats according to Buff
	if (registry.buffs.has(player)) {
		auto& buff = registry.buffs.get(player);
		player_component.speed = player_component.base_speed + buff.speed;
		player_component.defense = player_component.base_defense + buff.defense;
	}

	// Keep the player's position within the maximum room size
	player_motion.position.x = std::max(-(room.size.x + player_motion.scale.x) / 2.f,
										std::min(player_motion.position.x, (room.size.x + player_motion.scale.x)/2.f));
	player_motion.position.y = std::max(-room.size.y / 2.f,
										std::min(player_motion.position.y, (room.size.y - player_motion.scale.y)/2.f));

	if (!registry.deathTimers.has(player)) {
		// Player jump update
		if (registry.jumps.has(player)) {
			Jump& player_jump = registry.jumps.get(player);
			player_component.state = PLAYER_STATE::JUMPING;
			if (topdown)
				player_motion.velocity *= player_jump.speed_boost;
			player_jump.timer_ms -= tick_ms;
			if (player_jump.timer_ms < 0.f) {
				registry.jumps.remove(player);
				player_component.vul = true;
				if (player_component.move[0][0] != 0.f || player_component.move[0][1] != 0.f || player_component.move[1][0] != 0.f || player_component.move[1][1] != 0.f) {
					player_component.state = PLAYER_STATE::WALKING;
				}
				else {
					player_component.state = PLAYER_STATE::IDLE;
				}
			}
		}

		//  life bar update

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



		createLifeBar(renderer, { window_width_f - window_width_f / 5.f, window_height_f - window_height_f / 8.f }, vec2({ 320.f, 35.f }), PLAYER_MAX_HP, player_component.hp);
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

		// Player coin count update

		if (!registry.coinCounters.entities.empty()) {
			auto& coin_counter = registry.coinCounters.entities.back();
			registry.remove_all_components_of(coin_counter);
		}
		if (!registry.coinSymbols.entities.empty()) {
			auto& coin_symbol = registry.coinSymbols.entities.back();
			registry.remove_all_components_of(coin_symbol);
		}
		renderPoints(renderer, { window_width_f / 10.f,  window_height_f / 24.f }, std::to_string(points), 1.4f, { 1.0, 1.0, 1.0 }, 9);

		// Player key count update
		if (!registry.keyCounters.entities.empty()) {
			auto& key_counter = registry.keyCounters.entities.back();
			registry.remove_all_components_of(key_counter);
		}
		if (!registry.keySymbols.entities.empty()) {
			auto& key_symbol = registry.keySymbols.entities.back();
			registry.remove_all_components_of(key_symbol);
		}
		renderKeys(renderer, {window_width_f / 4.f, window_height_f / 24.f }, std::to_string(keys), 1.4f, { 1.0, 1.0, 1.0 });

		// Player timer update
		Player& player_comp = registry.players.get(player);
		player_comp.elapsedTime += tick_ms; // assume deltaTime is in seconds
	}

	// Process bounces
	for (int i = 0; i < registry.bounces.size(); i++) {
		auto& bouncing = registry.bounces.entities[i];
		auto& bounce = registry.bounces.components[i];

		auto& motion = registry.motions.get(bouncing);

		float whole;
		float p = std::modf(since_start_ms / bounce.duration_ms, &whole);

		if (static_cast<int>(whole) % 2)
			p = 1 - p;

		motion.position = bounce.start * (1.0f - p) + bounce.end * p;
	}

	// Remove debug info from the last step
	while (!registry.debugComponents.entities.empty())
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Remove attacks based on time-to-live
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (auto ex: registry.expiry.entities) {
		auto& exc = registry.expiry.get(ex);
		if (exc.timer_ms <= 0.f)
			registry.remove_all_components_of(ex);
		else
			exc.timer_ms -= tick_ms;
	}

	for (int i = 0; i < registry.timeouts.entities.size(); i++) {
		auto& timeout = registry.timeouts.components[i];
		auto timeout_entity = registry.timeouts.entities[i];
		if (timeout.timer_ms <= 0.f) {
			timeout.callback();
			if (registry.timeouts.size() > 0) {
				registry.remove_all_components_of(registry.timeouts.entities[i]);
			}
			--i;
		}
		else {
			timeout.timer_ms -= tick_ms;
		}
	}

	next_enemy_spawn -= tick_ms;

	// Spawning new enemies
	if (room.room_type == ROOM::FIRST) {
		if (registry.hardShells.components.size() <= MAX_ENEMY && next_enemy_spawn < 0.f) {
			// Reset timer
			next_enemy_spawn = (ENEMY_DELAY_BASE_MS / difficulty_scaling)
					+ uniform_dist(rng) * (ENEMY_DELAY_BASE_MS / 2.f);
			// Create enemy
			Entity entity;
			if (uniform_dist(rng) < 0.1f) {
				entity = createEnemySlime(renderer, { 0.f,0.f }, true);
			}
			else if (uniform_dist(rng) < 0.2f) {
				entity = createEnemyFly(renderer, { 0.f,0.f });
			}
			else {
				entity = createEnemy(renderer, { 0.f,0.f });
			}
			// Setting random initial position and constant velocity
			Motion& motion = registry.motions.get(entity);

			Transform transform;
			transform.translate(vec2(1000.f, 1000.f));
			transform.rotate(uniform_dist(rng) * 2.f * M_PI);

			motion.position = transform.mat * vec3(1000.f, 1000.f, 0.f)
					+ vec3(player_motion.position[0], player_motion.position[1], 1.f);
			registry.ai.emplace(entity);
			registry.hits.insert(entity, Hit{1.f * difficulty_scaling});
		}

		for (Entity entity : registry.enemy_fly.entities) {
			// Check if enemyfly is ready to fire
			Emitter& enemy_emitter = registry.emitters.get(entity);
			enemy_emitter.next_spawn -= tick_ms;

			if (enemy_emitter.next_spawn < 0.f) {
				// Get enemyfly position
				Motion& enemy_motion = registry.motions.get(entity);
				vec2 enemy_position = enemy_motion.position;

				// Calculate direction towards player
				vec2 direction_to_player = player_motion.position - enemy_position;
				direction_to_player = normalize(direction_to_player); // Normalize the direction vector

				// Reset the spawn timer
				enemy_emitter.next_spawn = enemy_emitter.fire_delay;

				// Create the projectile, similar to the player's pebble
				Entity projectile = createPebble(enemy_position, { 20.f, 20.f }, vec3{0.5f, 0.f, 0.f});

				// Set the projectile's velocity towards the player
				Motion& projectile_motion = registry.motions.get(projectile);
				float projectile_speed = 200.f;
				projectile_motion.velocity = direction_to_player * projectile_speed;

				// Register the projectile as an attack
				registry.hardShells.insert(projectile, HardShell{true});
				registry.expiry.emplace(projectile, Expiry{5000.f});
				registry.hits.emplace(projectile);
			}
		}
	} else if ((room.room_type == ROOM::SECOND) && registry.hardShells.components.size() <= MAX_ENEMY && next_enemy_spawn < 0.f) {
		// Reset timer
		next_enemy_spawn = (ENEMY_DELAY_BASE_MS / difficulty_scaling)
		                   + uniform_dist(rng) * (ENEMY_DELAY_BASE_MS / 2.f);
		// Create enemy
		Entity entity;
		if (uniform_dist(rng) < 0.1f) {
			// Prevent spawning smaller slimes in the platform room
			entity = createEnemySlime(renderer, {0.f, 0.f}, true);
		} else if (uniform_dist(rng) < 0.2f) {
			entity = createEnemyFly(renderer, {0.f, 0.f});
		} else {
			entity = createEnemy(renderer, {0.f, 0.f});
		}
		// Setting random initial position and constant velocity
		Motion &motion = registry.motions.get(entity);

		motion.position = vec2(player_motion.position[0] - 1000.f + uniform_dist(rng) * 200.f, 0.f);
		registry.ai.emplace(entity, Ai{3'000});
		registry.hits.insert(entity, Hit{1.f * difficulty_scaling});

		registry.physics.emplace(entity);
	} else if (room.room_type == ROOM::COW) {
		if (registry.hardShells.components.size() <= MAX_ENEMY && next_enemy_spawn < 0.f) {
			// Reset timer
			next_enemy_spawn = (ENEMY_DELAY_BASE_MS / difficulty_scaling)
			                   + uniform_dist(rng) * (ENEMY_DELAY_BASE_MS / 2.f);
			// Create cow
			Entity entity;

			// TODO:: replace with hostile cow
			entity = createHostileCow(renderer, { 0.f,0.f });
			// Setting random initial position and constant velocity
			Motion& motion = registry.motions.get(entity);

			Transform transform;
			transform.translate(vec2(1000.f, 1000.f));
			transform.rotate(uniform_dist(rng) * 2.f * M_PI);

			motion.position = transform.mat * vec3(1000.f, 1000.f, 0.f)
			                  + vec3(player_motion.position[0], player_motion.position[1], 1.f);
			registry.ai.emplace(entity);
			registry.hits.insert(entity, Hit{5.f});
		}
	}

	// Spawning new pebbles for player
	Emitter& emitter = registry.emitters.get(player);
	// Disable attack while jumping
	if (emitter.firing && !registry.jumps.has(player) && !registry.deathTimers.has(player)) {
		emitter.next_spawn -= tick_ms;
	}

	if (emitter.next_spawn < 0.f) {
		Transform transform;
		transform.rotate(player_motion.angle);
		// Reset timer
		emitter.next_spawn = emitter.fire_delay;
		// Create particle
		Entity entity = createPebble({ player_motion.position[0], player_motion.position[1] },
		                             { 10.f, 10.f },  vec3{1.f, 1.f, 1.f});
		Motion& motion = registry.motions.get(entity);

		motion.velocity = transform.mat * vec3(2000.f, 0.f, 0.f);
		registry.attacks.emplace(entity, Attack{ 1.f, false });
		registry.expiry.emplace(entity, Expiry{10000.f});

		Mix_PlayChannel(-1, zap_sound, 0);
	}

	// Processing the player state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = renderer->screen_state;

	float min_timer_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& timer = registry.deathTimers.get(entity);
		timer.timer_ms -= tick_ms;
		if (timer.timer_ms < min_timer_ms) {
			min_timer_ms = timer.timer_ms;
		}

		// restart the game once the death timer expired
		if (timer.timer_ms < 0) {
			registry.deathTimers.remove(entity);
			restart_game();
			return true;
		}
	}
	// reduce window brightness if any of the present players is dying
	screen.screen_darken_factor = 1 - min_timer_ms / 3000;

	for (Entity entity : registry.lightUps.entities) {
		// progress timer
		auto& timer = registry.lightUps.get(entity);
		timer.timer_ms -= tick_ms;

		// restart the game once the death timer expired
		if (timer.timer_ms < 0) {
			registry.lightUps.remove(entity);
		}
	}

	for (Entity entity : registry.knockbacks.entities) {
		// progress timer
		auto& timer = registry.knockbacks.get(entity);
		timer.timer_ms -= tick_ms;
		// remove component once timer expired (reset velocity)
		if (timer.timer_ms < 0) {
			registry.motions.get(entity).velocity = vec2(0, 0);
			registry.knockbacks.remove(entity);
		}
	}

	// AI update
	if (topdown)
		for (uint i=0; i < registry.ai.entities.size(); i++) {
			// progress timer
			auto entity = registry.ai.entities[i];
			auto& timer = registry.ai.components[i];
			auto& entity_motion = registry.motions.get(entity);
			timer.timer_ms -= tick_ms;

			if (timer.timer_ms > 0) continue;

			Motion& motion = registry.motions.get(entity);

			if (uniform_dist(rng) < 0.5f) {
				// check if ai is boss, if so only make boss move towards player if its in state of walking
				if (registry.boss.has(entity) && registry.boss.get(entity).state != BOSS_STATE::WALKING) {
					// not do anything
				}
				else {
					// Move towards player
					motion.velocity = normalize(
						player_motion.position - entity_motion.position) * 100.f;
					// Refresh
					timer.timer_ms = 3000.f;
				}
			}
			else {
				// check if ai is boss, if so only make boss move towards player if its in state of walking
				if (registry.boss.has(entity) && registry.boss.get(entity).state != BOSS_STATE::WALKING) {
					// not do anything
				}
				else {
					// Randomly walk around
					motion.velocity = normalize(
						vec2(window_width_px * (uniform_dist(rng) - .5),
							window_height_px * (uniform_dist(rng) - .5))
					) * 50.f;
					// Refresh
					timer.timer_ms = 1000.f;
				}
			}
		}
	else
		for (int i = 0; i < registry.ai.size(); i++) {
			// progress timer
			Entity entity = registry.ai.entities[i];
			auto& timer = registry.ai.components[i];
			auto& entity_motion = registry.motions.get(entity);
			timer.timer_ms -= tick_ms;

			if (timer.timer_ms > 0) continue;

			Motion& motion = registry.motions.get(entity);
			motion.velocity.y -= 500.f + uniform_dist(rng) * 500.f;

			motion.velocity.x = (player_motion.position.x - entity_motion.position.x > 0 ? 180.f : -180.f)
					+ uniform_dist(rng) * 40;

			// Refresh
			timer.timer_ms = 1000.f * uniform_dist(rng) + 500.f;
		}

	for (auto& particle_system: registry.particleSystems.components)
		particle_system.update(tick_ms);

	// Gravitate coins
	for (Entity coin: registry.coinValues.entities) {
		auto& motion = registry.motions.get(coin);
		auto dist = glm::distance(motion.position, player_motion.position);
		if (dist > COIN_PULL_DISTANCE) continue;
		auto&& direct = glm::normalize( player_motion.position - motion.position);
		motion.position += COIN_GRAVITY / dist * direct * elapsed_ms_since_last_update;
	}

	// Gravitate healthPotions
	for (Entity potion : registry.healthPotions.entities) {
		auto& motion = registry.motions.get(potion);
		auto dist = glm::distance(motion.position, player_motion.position);
		if (dist > COIN_PULL_DISTANCE) continue;
		auto&& direct = glm::normalize(player_motion.position - motion.position);
		motion.position += COIN_GRAVITY / dist * direct * elapsed_ms_since_last_update;
	}

	// COW RITUAL IS READY!
	if (registry.capturedCows.size() >= REQUIRED_COWS) {
		// Make all cows move towards the player.
		for (auto cow: registry.capturedCows.entities) {
			auto& tension = registry.tensions.get(cow);
			tension.center = player;
			tension.l = 0.f;
			// Delete cows after 10 seconds.
			registry.expiry.emplace(cow, Expiry{3'000.f});
			// Cows are no longer collectable.
			registry.softShells.remove(cow);
		}

		// TODO: spawn a portal to the cow level.
		transition_room(ROOM::COW);

		// Cows no longer function as cows. Their sprites will be deleted in time.
		registry.capturedCows.clear();
	}

	// Teleport enemies close to player if they get too far away
//	if (topdown)
//		for (size_t h = 0; h < registry.attacks.size(); h++) {
//			auto hit_entity = registry.hits.entities[h];
//			auto& hit = registry.hits.components[h];
//			Motion& hit_motion = registry.motions.get(hit_entity);
//			// If an enemy is too far away, relocate near the player.
//			auto dist = hit_motion.position - player_motion.position;
//			if (glm::length(dist) > 1200.f) {
//				hit_motion.position = glm::normalize(dist) * 1000.f + player_motion.position;
//			}
//		}
	

	// Rerender UI
	// rerenderUI();

	return true;
}

void WorldSystem::dropItemMuted(vec2 position) {
	// Spawning new coin (only for testing, coins only spawn after killing enemy)
	Entity entity = createCoin(renderer, { 0,0 });
	CoinValue& coinValue = registry.coinValues.get(entity);
	// Randomize coin value to be between 1-3
	coinValue.value = 1 + (int)(uniform_dist(rng) * 3);

	Motion& motion = registry.motions.get(entity);
	motion.position = position;
	motion.velocity = vec2(0, 0);
  
  auto& room = registry.rooms.get(current_room);
	const bool topdown = room.topdown;

	if (!topdown) {
		motion.velocity = vec2(0, 250.f);
	}
}

void WorldSystem::dropItem(vec2 position) {
	dropItemMuted(position);
	Mix_PlayChannel(-1, drop_item_sound, 0);
}

void WorldSystem::dropHealthPotion(vec2 position) {
	// Spawning new healthPotion 
	Entity entity = createHealthPotion(renderer, { 0,0 });

	Motion& motion = registry.motions.get(entity);
	motion.position = position;
	motion.velocity = vec2(0, 0);
	Mix_PlayChannel(-1, drop_item_sound, 0);
}

void WorldSystem::dropWeapon(vec2 position) {
	// spawning a new weapon
	int random_ground_item_type = 1 + int(uniform_dist(rng) * 5);

	switch (random_ground_item_type) {
	case 1:
		createGroundItem(renderer, position, { 1, ITEM_TYPE::SWORD, {3, 0, 0, 0} });
		break;
	case 2:
		createGroundItem(renderer, position, { 1, ITEM_TYPE::LASER_GUN, {3, 0, 0, 0} });
		break;
	case 3:
		createGroundItem(renderer, position, { 1, ITEM_TYPE::PISTOL, {3, 0, 0, 0} });
		break;
	case 4:
		createGroundItem(renderer, position, { 1, ITEM_TYPE::ASSAULT_RIFLE, {3, 0, 0, 0} });
		break;
	case 5:
		createGroundItem(renderer, position, { 1, ITEM_TYPE::SHOTGUN, {3, 0, 0, 0} });
		break;
	default:
		createGroundItem(renderer, position, { 1, ITEM_TYPE::SHOTGUN, {3, 0, 0, 0} });
	}
	Mix_PlayChannel(-1, drop_item_sound, 0);
}

void WorldSystem::dropInventoryItem() {
	Inventory& inventory = registry.inventories.get(player);
	
	// Check if selected slot is empty. If so, do nothing
	// essentially check inventory.emptySlots.contains(inventory.selectedIndex)
	// https://stackoverflow.com/questions/3450860/check-if-a-stdvector-contains-a-certain-object
	if (std::find(inventory.emptySlots.begin(), inventory.emptySlots.end(), inventory.selectedIndex)
		!= inventory.emptySlots.end()) {
		/* Empty Slot contains Selected Index
		 Thus drop nothing and do nothing. */
		return;
	}
	else {
		/* Selected Index holds an item! Drop the item. */

		// Make slot 'empty'
		inventory.emptySlots.push_back(inventory.selectedIndex);
		Item itemMetaData = registry.items.get(inventory.items[inventory.selectedIndex]);
		
		// Remove item entity from registry.
		registry.remove_all_components_of(inventory.items[inventory.selectedIndex]);
		
		// Insert empty entity into slot (TODO: check for mem leaks here)
		inventory.items[inventory.selectedIndex] = Entity();

		// Create ground item ontop of player. 'Simulating dropping item'. Maybe add a random knockback so that
		// items don't drop on top of each other.
		vec2 playerPosition = registry.motions.get(player).position;
		Entity entity = createGroundItem(renderer, playerPosition, itemMetaData);
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	printf("Restarting\n");

	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");


	// Remove all old entities
	registry.clear_all_components();

	// Reset the game speed
	current_speed = 1.f;
	difficulty_scaling = 1.f;
	ScreenState& screen = renderer->screen_state;
	screen.screen_darken_factor = 0;

	// Debugging for memory/component leaks
	registry.list_all_components();

	if (!registry.players.has(player)) {
		player = createPlayer(renderer, { 0.f, 0.f});
		camera = createCamera(player);
		registry.colors.insert(player, { 1, 0.8f, 0.8f });
	}

	Motion& player_motion = registry.motions.get(player);
	player_motion.position = { 200.f, 700.f};
	Player& player_info = registry.players.get(player);
	player_info.jump_velocity = { 0.f, -700.f };

	player_info.state = PLAYER_STATE::IDLE;
	registry.colors.get(player) = { 1, 0.8f, 0.8f };

	transition_room(ROOM::ROSEGARDEN);


	registry.list_all_components();

	if (registry.tutorialMessages.entities.empty()) {
		createButton(renderer, { window_width_f - 100.f, 100.f }, BUTTONTYPE::HELP);
	}
	createButton(renderer, { window_width_f - 100.f, 200.f }, BUTTONTYPE::SHOP);


	Entity text = createText(renderer, { 100.f, 100.f }, "Recollection", 1.f, { 1.0, 1.0, 1.0 }, 0);
	registry.expiry.emplace(text, Expiry{3000.f});

	// Debugging for memory/component leaks
	registry.list_all_components();
	registry.particleSystems.get(player).emit(100);

	vec2 inventory_pos = { window_width_f / 2.f - window_width_f / 20.f, window_height_f / 8.f };

	rerenderUI(inventory_pos);
}

float lerp(float start, float end, float t) {
	return start + t * (end - start);
}


// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		restart_game();
		return;
	}

	// Debugging
	if (key == GLFW_KEY_F1 && action == GLFW_RELEASE) {
		debugging.in_debug_mode = !debugging.in_debug_mode;
		return;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
		current_speed = fmax(0.f, current_speed);
		return;
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
		current_speed = fmax(0.f, current_speed);
		return;
	}

	// If the player is dead, disable key movement.
	if (registry.deathTimers.has(player)) return;
	if (!registry.players.has(player)) return;
	Player& player_comp = registry.players.get(player);

	if (debugging.in_debug_mode && action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_F2:
				player_comp.vul = !player_comp.vul;
				break;
			case GLFW_KEY_F3:
				registry.deathTimers.emplace(player);
				break;
			case GLFW_KEY_F4:
				for (const auto& entity : registry.ai.entities)
					registry.remove_all_components_of(entity);
				break;
			case GLFW_KEY_F5:
				transition_room(ROOM::SECOND);
				break;
			case GLFW_KEY_F6:
				printf("Player at: %s\n", glm::to_string(registry.motions.get(player).position).c_str());
				break;
			case GLFW_KEY_F7:
				createEnemies(renderer, 100, player);
				break;
			case GLFW_KEY_F8:
				ignore_collision = !ignore_collision;
				break;
			case GLFW_KEY_F9:
				Entity cow = createCow(renderer, registry.motions.get(player).position);
				break;
		}
	}

	//exit tutorial
	if (action == GLFW_PRESS && !registry.tutorialMessages.components.empty() && key == GLFW_KEY_ESCAPE) {
		printf("Escape pressed\n");
		Entity tutorial_messages = registry.tutorialMessages.entities[0];
		current_speed = 1.0f;
		registry.remove_all_components_of(tutorial_messages);
	}

	//exit shop
	if (action == GLFW_PRESS && !registry.shops.components.empty() && key == GLFW_KEY_ESCAPE) {
		printf("Escape pressed\n");
		vec2 inventory_pos = { window_width_f / 2.f - window_width_f / 20.f, window_height_f / 8.f };

		rerenderUI(inventory_pos);

		for (Entity shopUI : registry.shops.entities) {
			registry.remove_all_components_of(shopUI);
		}
		for (Entity shopItem : registry.shopItems.entities) {
			registry.remove_all_components_of(shopItem);
		}

		current_speed = 1.0f;
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_P) {
		transition_room(ROOM::BOSS);
	}
	vec2 inventory_pos = { window_width_f / 2.f - window_width_f / 20.f, window_height_f / 8.f };

	if (action == GLFW_PRESS && registry.shops.components.empty()) {
		switch(key) {
			case GLFW_KEY_W:
				player_comp.set_move_up(1.f);
				break;
			case GLFW_KEY_S:
				player_comp.set_move_down(1.f);
				break;
			case GLFW_KEY_A:
				player_comp.set_move_left(1.f);
				break;
			case GLFW_KEY_D:
				player_comp.set_move_right(1.f);
				break;
			case GLFW_KEY_1:
				registry.inventories.get(player).selectedIndex = 0;
				rerenderUI(inventory_pos);
				break;
			case GLFW_KEY_2:
				registry.inventories.get(player).selectedIndex = 1;
				rerenderUI(inventory_pos);
				break;
			case GLFW_KEY_3:
				registry.inventories.get(player).selectedIndex = 2;
				rerenderUI(inventory_pos);
				break;
			case GLFW_KEY_4:
				registry.inventories.get(player).selectedIndex = 3;
				rerenderUI(inventory_pos);
				break;
			case GLFW_KEY_5:
				registry.inventories.get(player).selectedIndex = 4;
				rerenderUI(inventory_pos);
				break;
			case GLFW_KEY_E:
				player_comp.in_pick_up_state = true;
				break;
			case GLFW_KEY_J:
				// Drop selected inventory item
				dropInventoryItem();
			case GLFW_KEY_B:
				// change inventory buff_selection
				on_inventory_selection_change();
			default:
				return;
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_W:
				player_comp.set_move_up(0.f);
				break;
			case GLFW_KEY_S:
				player_comp.set_move_down(0.f);
				break;
			case GLFW_KEY_A:
				player_comp.set_move_left(0.f);
				break;
			case GLFW_KEY_D:
				player_comp.set_move_right(0.f);
				break;
			case GLFW_KEY_E:
				player_comp.in_pick_up_state = false;
				break;
		default:
			return;
		}
	}


	/*
	.move[0][0]: right velocity
	.move[0][1]: back velocity
	.move[1][0]: left velocity
	.move[1][1]: front velocity
	*/

	// handlling character directions on keys

	if (action == GLFW_PRESS || action == GLFW_RELEASE) {
		bool isMovingRight = player_comp.move[0][0] == 1.f;
		bool isMovingBack = player_comp.move[0][1] == 1.f;
		bool isMovingLeft = player_comp.move[1][0] == 1.f;
		bool isMovingFront = player_comp.move[1][1] == 1.f;

		if (isMovingRight && isMovingLeft) {
			isMovingRight = false;
			isMovingLeft = false;
		}
		if (isMovingFront && isMovingBack) {
			isMovingFront = false;
			isMovingBack = false;
		}

		player_comp.state = PLAYER_STATE::WALKING;

		if (isMovingLeft && isMovingBack) {
			player_comp.direction = DIRECTION::BACK_LEFT;
		}
		else if (isMovingFront && isMovingLeft) {
			player_comp.direction = DIRECTION::FRONT_LEFT;
		}
		else if (isMovingFront && isMovingRight) {
			player_comp.direction = DIRECTION::FRONT_RIGHT;
		}
		else if (isMovingBack && isMovingRight) {
			player_comp.direction = DIRECTION::BACK_RIGHT;
		}
		else if (isMovingLeft) {
			player_comp.direction = DIRECTION::LEFT;
		}
		else if (isMovingRight) {
			player_comp.direction = DIRECTION::RIGHT;
		}
		else if (isMovingBack) {
			player_comp.direction = DIRECTION::BACK;
		}
		else if (isMovingFront) {
			player_comp.direction = DIRECTION::FRONT;
		}
		else {
			if (!registry.jumps.has(player))
				player_comp.state = PLAYER_STATE::IDLE;
		}
	}


}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	Motion& player_motion = registry.motions.get(player);
	vec2& camera_position = registry.motions.get(camera).position;

	vec2&& relative = mouse_position - window_center - player_motion.position + camera_position;

	if (!registry.deathTimers.has(player))
		player_motion.angle = atan2(relative[1], relative[0]);
}

void WorldSystem::on_inventory_selection_change() {
	// Assigns a buff to player according to the item they have activated.
	Inventory& player_inventory = registry.inventories.get(player);

	Entity& player_item = player_inventory.items[player_inventory.selectedIndex];

	// Remove old buff
	if (registry.buffs.has(player)) {
		registry.buffs.remove(player);
	}

	// Check if there is an item in the player's hand.
	if (!registry.items.has(player_item)) {
		// No item, do nothing.
		return;
	}

	Item& item = registry.items.get(player_item);

	// Convert ItemStat on item to a Buff component to assign to player
	Buff& buff = registry.buffs.emplace(player);

	// Display item buff description text
	if (item.type == ITEM_TYPE::STONE) {
		auto text = createText(renderer, vec2{ 200.f, 200.f }, std::string("Stone buff activated! speed:" + std::to_string(item.stats.speed) + "  " + "health:" + std::to_string(item.stats.health) + "  " + "defense:" + std::to_string(item.stats.defense) + "  "), .5f, { 1.0f, 1.0f, 1.0f }, 0);
		// Remove it after 1 seconds
		registry.expiry.emplace(text, Expiry{ 2000.f });
	}
	 
	else if (item.type == ITEM_TYPE::FEATHER) {
		auto text = createText(renderer, vec2{ 200.f, 50.f }, std::string("Feather buff activated! speed:" + std::to_string(item.stats.speed) + "  " + "health:" + std::to_string(item.stats.health) + "  " + "defense:" + std::to_string(item.stats.defense) + "  "), .5f, { 1.0f, 1.0f, 1.0f }, 0);
		// Remove it after 1 seconds
		registry.expiry.emplace(text, Expiry{ 5000.f });
	}

	else if (item.type == ITEM_TYPE::GOLDEN_EGG) {
		auto text = createText(renderer, vec2{ 200.f, 50.f }, std::string("Golden Egg buff activated! speed:" + std::to_string(item.stats.speed) + "  " + "health:" + std::to_string(item.stats.health) + "  " + "defense:" + std::to_string(item.stats.defense) + "  "), .5f, { 1.0f, 1.0f, 1.0f }, 0);
		// Remove it after 1 seconds
		registry.expiry.emplace(text, Expiry{ 5000.f });
	}
	
	// buff.attack = item.stats.attack;
	buff.defense = item.stats.defense;
	buff.health = item.stats.health;
	buff.speed = item.stats.speed;
}

void WorldSystem::on_item_pickup(bool isSuccessful, ITEM_TYPE itemType) {
	if (!isSuccessful) {
		auto text = createText(renderer, vec2{ 200.f, 200.f }, std::string("Inventory Full! Press 'J' to Drop item"),.5f, { 1.0f, 1.0f, 1.0f }, 0);
		// Remove it after 1 seconds
		registry.expiry.emplace(text, Expiry{ 2000.f });
	}
	
	switch (itemType) {
		default:
			break;
		// Fallthrough buff items
		case ITEM_TYPE::FEATHER:
		case ITEM_TYPE::GOLDEN_EGG:
		case ITEM_TYPE::STONE:
			auto text = createText(renderer, vec2{ 200.f, 200.f }, std::string("Picked up Buff item! Press 'B' to activate."), .5f, { 1.0f, 1.0f, 1.0f }, 0);
			// Remove it after 1 seconds
			registry.expiry.emplace(text, Expiry{ 2000.f });
			break;
		
	}
}

void WorldSystem::handle_attack_req() {
	auto& emitter = registry.emitters.get(player);
	Inventory& player_inventory = registry.inventories.get(player);
	Entity& player_item = player_inventory.items[player_inventory.selectedIndex];
	
	
	// Check if there is an item in the player's hand.
	if (!registry.items.has(player_item)){
		// No item, just shoot pebbles.
		emitter.firing = true;
		emitter.next_spawn = 0.f;
		emitter.fire_delay = 300.f;
		return;
	} 

	Item& item = registry.items.get(player_item);
	if (item.type == ITEM_TYPE::STONE || item.type == ITEM_TYPE::FEATHER ) {
		// buff item, just shoot pebbles.
		emitter.firing = true;
		emitter.next_spawn = 0.f;
		emitter.fire_delay = 300.f;
		return;
	}

	Motion& player_motion = registry.motions.get(player);

	// Since player_motion.angle tracks the mouse, we can use it to determine the direction of the attack.
	// However we need to convert it to a vec2
	vec2 attack_dir = { cos(player_motion.angle), sin(player_motion.angle) };
	// Else check weapon type and use it.

	switch (item.type) {
		case ITEM_TYPE::SWORD:
		    // TODO: add delay so it can't be spammed. (need step so maybe use attack system to create attacks
			// and have this switch statement create new attackReq with field ITEM_TYPE)
			// Maybe add an attack delay component to the player.
			createMeleeAttack(renderer, player, attack_dir, 128, 8, 200);
			break;
		case ITEM_TYPE::PISTOL:
			// Default weapon.
			emitter.firing = true;
			// emitter.next_spawn = 0.f;
			emitter.fire_delay = 300.f;
			break;
		case ITEM_TYPE::ASSAULT_RIFLE:
			// Weapon without needing to spam.
			emitter.firing = true;
			// emitter.next_spawn = 0.f;
			emitter.fire_delay = 150.f;
			break;
		case ITEM_TYPE::SHOTGUN:
			// Weapon with spread. TODO: add delay so it can't be spammed.
			createShotgunAttack(renderer, player, attack_dir, 5);
			break;
		case ITEM_TYPE::LASER_GUN:
			// Weapon with laser.
			createLaserAttack(renderer, player, attack_dir, 35, 8, 200);
			break;
		default:
			printf("No item selected!");
	}
}

bool WorldSystem::isButtonPressed(Entity button) {
	Motion& button_motion = registry.motions.get(button);
	int button_posx = static_cast<int>(button_motion.position[0]);
	int button_posy = static_cast<int>(button_motion.position[1]);
	int button_width = static_cast<int>(button_motion.scale[0]);
	int button_height = static_cast<int>(button_motion.scale[1]);

	int button_left_bound = button_posx - button_width / 2;
	int button_right_bound = button_posx + button_width / 2;

	int button_top_bound = button_posy + button_height / 2;
	int button_bottom_bound = button_posy - button_height / 2;

	// mouse location
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	return button_left_bound <= xpos && xpos <= button_right_bound &&
		button_bottom_bound <= ypos && ypos <= button_top_bound;
}

void WorldSystem::on_mouse_button(int button, int action, int) {
	auto& emitter = registry.emitters.get(player);

	// If the player is dead, disable mouse press actions.
	if (registry.deathTimers.has(player)) return;

	auto& room = registry.rooms.get(current_room);
	const bool topdown = room.topdown;

	for (Entity entity : registry.buttons.entities) {

		if (isButtonPressed(entity) && !registry.buttons.components.empty()) {
			printf("Button pressed\n");
			Button& button = registry.buttons.get(entity);
			printf("button type: %d\n", button.type);
			switch (button.type) {
			case BUTTONTYPE::HELP:
				if (registry.tutorialMessages.components.empty() && registry.shops.components.empty()) {
					if (topdown) {
						createTutorialMessage(renderer, { window_width_f / 2.f, window_height_f / 2.f },
							TUTORIALTYPE::TOPDOWN);
					}
					else {
						createTutorialMessage(renderer, { window_width_f / 2.f, window_height_f / 2.f },
							TUTORIALTYPE::PLATFORM);
					}
					current_speed = 0.f;
				}
				break;
			case BUTTONTYPE::SHOP:
				if (registry.shops.components.empty() && registry.tutorialMessages.components.empty())
				{
					createShop(renderer, points);
					current_speed = 0.f;
					vec2 inventory_pos = { window_width_f - window_width_f / 1.6f, window_height_f - window_height_f / 7.8f };
					rerenderUI(inventory_pos);

				}
				break;
			case BUTTONTYPE::SHOPITEM:
				ShopItem& shopItem = registry.shopItems.get(entity);
				Motion& button_motion = registry.motions.get(entity);
				Item& item = registry.items.get(entity);
				registry.buttons.remove(entity);

				if (!registry.shops.components.empty() && !registry.shopItems.components.empty() &&
					shopItem.price <= points)
				{

					auto text = createText(renderer, { button_motion.position.x - window_width_f / 24.f, button_motion.position.y }, "Bought", 1.f, { 1.0, 1.0, 1.0 }, 10);
					registry.shops.emplace(text);

					points -= shopItem.price;

					Inventory& playerInventory = registry.inventories.get(player);
					if (playerInventory.emptySlots.size() > 0) {
						int collectIndex = playerInventory.emptySlots[playerInventory.emptySlots.size() - 1];
						playerInventory.emptySlots.pop_back();
						
						// Create item entity
						Entity new_item_entity = Entity();
						auto& new_item = registry.items.emplace(new_item_entity);
						new_item.type = item.type;

						playerInventory.items[collectIndex] = (new_item_entity);
					}

					Mix_PlayChannel(-1, coin_sound, 0);

					vec2 inventory_pos = { window_width_f - window_width_f / 1.6f, window_height_f - window_height_f / 7.8f };
					rerenderUI(inventory_pos);
				}
				else if (!registry.shops.components.empty() && !registry.shopItems.components.empty() &&
										shopItem.price > points)
				{
					Mix_PlayChannel(-1, drop_item_sound, 0);

					auto text = createText(renderer, { button_motion.position.x - window_width_f/12.f, button_motion.position.y }, "Not enough coins", 1.f, { 1.0, 1.0, 1.0 }, 10);
					registry.shops.emplace(text);
				}
				break;
			}

		}
	}

	if (!registry.shops.components.empty() || !registry.tutorialMessages.components.empty()) return;

	switch (action) {
		case GLFW_PRESS:
			switch (button) {
				case GLFW_MOUSE_BUTTON_1:
					handle_attack_req();
					break;
				case GLFW_MOUSE_BUTTON_2:
					if (!registry.jumps.has(player)) {
						Mix_PlayChannel(-1, player_jump_sound, 0);
						if (topdown) {
							registry.players.get(player).vul = false;
						} else {
							player_jump();
						}
						registry.jumps.insert(player,
							Jump{});
					}
					break;
				default:
					return;
			}
			break;
		case GLFW_RELEASE:
			switch (button) {
				case GLFW_MOUSE_BUTTON_1:
					emitter.firing = false;
					break;
				default:
					return;
			}
			break;
		default:
			return;
	}
}

void WorldSystem::rerenderUI(vec2 inventory_pos) {
	// Render inventory UI
	renderInventory(renderer, inventory_pos);

	// Render lifebar UI
	if (registry.players.size() > 0) {
		auto& player_component = registry.players.get(player);

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

		// Create a lifebar entity initialized with the player component
		createLifeBar(renderer, { window_width_f - window_width_f / 5.f, window_height_f - window_height_f / 8.f }, vec2({ 320.f, 35.f }), PLAYER_MAX_HP, player_component.hp);
		renderPlayerSymbol(renderer, { window_width_f - window_width_f / 5.f - 220.f, window_height_f - window_height_f / 8.f }, vec2({ -PLAYER_BB_WIDTH * 2, PLAYER_BB_HEIGHT * 2 }));

	}

	// Render cow Boss Lifebar
	if (registry.boss.size() > 0) {
		auto& cowBoss_component = registry.boss.entities.back();
		auto& cowBossHit_compoenent = registry.hits.get(cowBoss_component);
		auto& cowBossMotion_component = registry.motions.get(cowBoss_component);

		// Create a lifebar entity initialized with the player component
		createLifeBar(renderer, { window_width_f / 5.f, window_height_f - window_height_f / 8.f }, vec2({ 320.f, 35.f }), COW_BOSS_MAX_HP, cowBossHit_compoenent.hp);
		renderCowBossSymbol(renderer, { window_width_f / 5.f - 260.f, window_height_f - window_height_f / 8.f - 30.f }, vec2({ -COIN_BB_WIDTH * 10, COIN_BB_HEIGHT * 10 }));
	}
	
}

void WorldSystem::transition_room(ROOM room_name) {
	ECSRegistry new_registry;
	registry.copy_global_entities(new_registry);
	registry.clear_all_components();

	registry = std::move(new_registry);
	registry.update_registry_list();

	// Room builders
	current_room = room_builders[room_name]();
	// Reset difficulty
	difficulty_scaling = 1.f;
	auto& room = registry.rooms.get(current_room);

	// Display room description text
	auto text = createText(renderer, vec2{100.f, 300.f}, room.name, .5f, { 1.0f, 1.0f, 1.0f }, 3);
	// Remove it after 5 seconds
	registry.expiry.emplace(text, Expiry{ 5000.f });

	if (room_name == ROOM::FIRST) {
		auto helper_text_one = createText(renderer, { 100.f, 400.f }, "Use WASD to move", .5f, { 1.0, 1.0, 1.0 }, 3);
		registry.expiry.emplace(helper_text_one, Expiry{ 6000.f });

		auto helper_text_two = createText(renderer, { 100.f, 450.f }, "Right click to jump", .5f, { 1.0, 1.0, 1.0 }, 3);
		registry.expiry.emplace(helper_text_two, Expiry{ 7000.f });

		auto helper_text_three = createText(renderer, { 100.f, 500.f }, "Left click to shoot", .5f, { 1.0, 1.0, 1.0 }, 3);
		registry.expiry.emplace(helper_text_three, Expiry{ 8000.f });

		auto helper_text_four = createText(renderer, { 100.f, 550.f }, "Press 1, 2, 3, 4, 5 to switch between weapons", .5f, { 1.0, 1.0, 1.0 }, 3);
		registry.expiry.emplace(helper_text_four, Expiry{ 9000.f });

		auto helper_text_five = createText(renderer, { 100.f, 600.f }, "Press E to pick up weapons", .5f, { 1.0, 1.0, 1.0 }, 3);
		registry.expiry.emplace(helper_text_five, Expiry{ 10000.f });

		auto helper_text_six = createText(renderer, { 100.f, 650.f }, "Press J to drop weapons", .5f, { 1.0, 1.0, 1.0 }, 3);
		registry.expiry.emplace(helper_text_six, Expiry{ 11000.f });

		auto helper_text_seven = createText(renderer, { 100.f, 700.f }, "Press B to activate selected buff item", .5f, { 1.0, 1.0, 1.0 }, 3);
		registry.expiry.emplace(helper_text_seven, Expiry{ 12000.f });
	}

	
}

void WorldSystem::player_jump() {
	auto& player_component = registry.players.get(player);
	if (player_component.jumps_done < player_component.jumps) {
		registry.motions.get(player).velocity = player_component.jump_velocity;
		player_component.jumps_done++;
	}
}