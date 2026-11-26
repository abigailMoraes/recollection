#include "world_init.hpp"

#include <utility>
#include "tiny_ecs_registry.hpp"
#include "components.hpp"
#include "particle_system.hpp"
#include <chrono>

const size_t N_COWS = 10;


// Static map between ITEMTYPE and texture
static std::map<ITEM_TYPE, TEXTURE_ASSET_ID> itemTextureMap = {
	{ ITEM_TYPE::SWORD, TEXTURE_ASSET_ID::SWORD },
	{ ITEM_TYPE::ASSAULT_RIFLE, TEXTURE_ASSET_ID::ASSAULT_RIFLE},
	{ ITEM_TYPE::PISTOL, TEXTURE_ASSET_ID::PISTOL },
	{ ITEM_TYPE::SHOTGUN, TEXTURE_ASSET_ID::SHOTGUN },
	{ ITEM_TYPE::LASER_GUN, TEXTURE_ASSET_ID::LASER_GUN},
	{ ITEM_TYPE::FEATHER, TEXTURE_ASSET_ID::FEATHER},
	{ ITEM_TYPE::STONE, TEXTURE_ASSET_ID::STONE},
	{ ITEM_TYPE::GOLDEN_EGG, TEXTURE_ASSET_ID::GOLDEN_EGG}
};

// Static helper for rotation
// Function to rotate a 2D vector by a given angle (in radians)
static vec2 rotate(const vec2& v, float angle) {
	float cosTheta = std::cos(angle);
	float sinTheta = std::sin(angle);

	return {
		v.x * cosTheta - v.y * sinTheta,
		v.x * sinTheta + v.y * cosTheta
	};
}

// C++ random number generator
std::default_random_engine rng = std::default_random_engine(
		std::chrono::system_clock::now().time_since_epoch().count());
std::uniform_real_distribution<float> uniform_dist; // number between 0..1

Entity createCamera(Entity focus)
{
	auto entity = Entity();
	registry.cameras.emplace(entity);
	Motion& motion = registry.motions.emplace(entity);
	Motion& player_motion = registry.motions.get(focus);
	motion.position = player_motion.position;
	registry.tensions.insert(entity, Tension{focus, 2.f});
	return entity;
}

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Add inventory to player
	createInventory(entity, 5);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ -PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT });

	// Create and (empty) Player component to be able to refer to all enemies
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_FRONT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		1});

	// For firing projectiles
	registry.emitters.insert(entity, Emitter());
	// For visual effects
	registry.particleSystems.insert(entity, ParticleSystem(renderer->effects[(GLuint)EFFECT_ASSET_ID::PARTICLE]));

	return entity;
}

Entity createEnemy(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { -100.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	// Create and (empty) Enemy component to be able to refer to all enemies
	registry.hardShells.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createEnemyFly(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { -50.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	// Create and (empty) Enemy component to be able to refer to all enemies
	registry.hardShells.emplace(entity);
	registry.enemy_fly.emplace(entity);
	// For firing projectiles
	Emitter& emitter = registry.emitters.insert(entity, Emitter());
	emitter.fire_delay = 4000.f; // hardcoded fire delay for enemy fly for now

	// For visual effects
	registry.particleSystems.insert(entity,
		ParticleSystem(renderer->effects[(GLuint)EFFECT_ASSET_ID::PARTICLE]));

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY_FLY,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}


Entity createEnemySlime(RenderSystem* renderer, vec2 position, bool isBigSlime)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { -100.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = isBigSlime? vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT }): vec2({ 0.5 * -ENEMY_BB_WIDTH, 0.5 * ENEMY_BB_HEIGHT });

	// Create and (empty) Enemy component to be able to refer to all enemies
	registry.hardShells.emplace(entity);

	Slime& slime = registry.slimes.emplace(entity);
	slime.isBigSlime = isBigSlime;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY_SLIME,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createCoin(RenderSystem* renderer, vec2 position) {
	// Reserve an entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -COIN_BB_WIDTH, COIN_BB_HEIGHT });

	// Initialize coin value of coin entity.
	auto& coin = registry.coinValues.emplace(entity);
	coin.value = 1;

	// Make this coin entity collectable.
	registry.collectables.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::COIN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::CPU_PARTICLE,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createPebble(vec2 pos, vec2 size, vec3 color)
{
	auto entity = Entity();
	
	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	registry.colors.emplace(entity, vec4({ color, 1.f }));

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
			EFFECT_ASSET_ID::CPU_PARTICLE,
			GEOMETRY_BUFFER_ID::PEBBLE });

	return entity;
}

Entity createWall(RenderSystem* renderer, vec2 position, vec2 size)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.velocity = { 0.f, 0.f };
	motion.scale =  size;
	motion.angle = 0.f;

	// Add a render request for the wall
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WALL,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createDoor(RenderSystem* renderer, vec2 position, vec2 size, ROOM destination) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;
	motion.angle = 0.f;

	// Add a render request for the door
	auto& door = registry.doors.emplace(entity);
	door.type = DOORTYPE::OPEN;
	door.to = destination;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DOOR_OPEN,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE, 2});

	printf("Door spawned at: %f.1, %f.1\n", position.x, position.y);
	return entity;
}

Entity createRoom(RenderSystem* renderer, const Room&& room) {
	auto entity = Entity();
	auto& position = room.position;
	auto& size = room.size;

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Set the initial motion values for the room
	Motion& motion = registry.motions.emplace(entity);
	motion.position = room.position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = room.size;

	Entity player = registry.players.entities[0];
	auto& player_motion = registry.motions.get(player);
	player_motion.position = { 0.f, 0.f };
	player_motion.velocity = { 0.f, 0.f };

	// Remove gravity from the player
	registry.physics.remove(player);

	// Create and add a RoomComponent
	registry.rooms.emplace(entity, room);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FLOOR,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	// Create walls
	float wall_width = 100.f;

	// Top Wall
	createWall(renderer, { position.x, position.y - (size.y + wall_width) / 2.f }, { size.x, wall_width });
	// Bottom Wall
	createWall(renderer, { position.x, position.y + (size.y + wall_width) / 2.f }, { size.x, wall_width });
	// Left Wall
	createWall(renderer, { position.x - (size.x + wall_width) / 2.f, position.y }, { wall_width, size.y });
	// Right Wall
	createWall(renderer, { position.x + (size.x + wall_width) / 2.f, position.y }, { wall_width, size.y });

	for (int i = 0; i < 10; i++) {
		float x = (uniform_dist(rng) - .5f) * size.x / 2.f;
		float y = (uniform_dist(rng) - .5f) * size.x / 2.f;
		bool barrel_collides = false;
		for (Entity e : registry.collidables.entities) {
			Motion& m = registry.motions.get(e);
			if (length(vec2(x, y) - m.position) < 100.f) {
				barrel_collides = true;
				break;
			}
		}
		if (!barrel_collides) {
			createBarrel(renderer, { x, y });
		}
	}

	// Spawning new cows, but if its boss room, spawn cow boss
	if (room.room_type != ROOM::BOSS) {
		for (size_t i = 0; i < N_COWS; i++) {
			Entity cow = createCow(renderer, { (uniform_dist(rng) - .5f) * size.x, (uniform_dist(rng) - .5f) * size.y });
			// Setting random initial position and constant velocity
			Motion& cow_motion = registry.motions.get(cow);
			// Randomly scale from 1 to 2. Why? For diversity...?
			cow_motion.scale *= uniform_dist(rng) + 1.f;
		}
	}
	else { // spawn cow boss
		Entity cowBoss = createHostileCow(renderer, { (uniform_dist(rng) - .5f) * size.x, (uniform_dist(rng) - .5f) * size.y });
		// Setting random initial position and constant velocity
		Motion& cow_motion = registry.motions.get(cowBoss);
		// Scale the cow boss
		cow_motion.scale *= 5.f;

		Transform transform;
		transform.translate(vec2(1000.f, 1000.f));
		transform.rotate(uniform_dist(rng) * 2.f * M_PI);

		motion.position = transform.mat * vec3(1000.f, 1000.f, 0.f)
			+ vec3(player_motion.position[0], player_motion.position[1], 1.f);
		registry.ai.emplace(cowBoss);
		registry.hits.insert(cowBoss, Hit{ COW_BOSS_MAX_HP });
		registry.boss.emplace(cowBoss);
	}

	return entity;
}

Entity createPlatformFloor(RenderSystem* renderer, vec2 position, vec2 size, PLATFORMFLOOR type) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Set the initial motion values for the floor
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	registry.meshPlatforms.emplace(entity);

	// create and add a PlatformFloor component based on specified type
	switch(type){
	case PLATFORMFLOOR::LONG:
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::PLATFORM_FLOOR_STRAIGHT_LONG,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;

	case PLATFORMFLOOR::SLOPED:
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::PLATFORM_FLOOR_SLOPED,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
	case PLATFORMFLOOR::STAIRS:
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::PLATFORM_STAIRS,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
	case PLATFORMFLOOR::GENERIC:
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::PLATFORM_FLOOR_GENERIC,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
	case PLATFORMFLOOR::GENERICGRASS:
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::GENERIC_FLOOR_GRASS,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
	case PLATFORMFLOOR::STAIRSGRASS:
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::STAIRS_GRASS,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
	default:
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::PLATFORM_FLOOR_STRAIGHT,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
	}

	return entity;

}

Entity createMeshPlatform(RenderSystem* renderer, vec2 position, vec2 size, vec2 velocity, PLATFORMTYPE type) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PLATFORM);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Set the initial motion values for the platform door
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.scale = size;

	registry.meshPlatforms.emplace(entity);
	switch (type) {

	case PLATFORMTYPE::GRASSSHORT:
		registry.renderRequests.insert(entity,
			{
				TEXTURE_ASSET_ID::LEVER_STAND_SHORT_GRASS,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
		
	default:
		registry.renderRequests.insert(entity,
			{
				TEXTURE_ASSET_ID::LEVER_STAND,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
	}
	return entity;
}


Entity createPlatformDoor(RenderSystem* renderer, vec2 position, vec2 size, DOORTYPE type, bool leverOperated, ROOM destination, PLATFORMTYPE floorType) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Set the initial motion values for the platform door
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	auto& door = registry.doors.emplace(entity);
	door.to = destination;

	switch (type) {
	case DOORTYPE::OPEN:
		door.type = DOORTYPE::OPEN;
		registry.renderRequests.insert(entity,
			{
				TEXTURE_ASSET_ID::OPEN_PLATFORM_DOOR,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
	default:
		door.type = DOORTYPE::CLOSED;
		registry.renderRequests.insert(entity,
			{
				TEXTURE_ASSET_ID::CLOSED_PLATFORM_DOOR,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
	}

	if (leverOperated) {
		// Create platform and lever linked to door if door is leverOperated

		createPlatform(renderer, {864.f, 260.f}, {147.f, 40.f}, floorType);
	}

	return entity;

}


Entity createPlatform(RenderSystem* renderer, vec2 position, vec2 size, PLATFORMTYPE type) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	//auto& lever = registry.levers.emplace(entity);
	//lever.door = door;
	// Set the initial motion values for the platform door
	Motion& motion = registry.motions.emplace(entity);
	registry.meshPlatforms.emplace(entity);

	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	switch (type) {
	case PLATFORMTYPE::LARGE:
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::LEVER_STAND_LARGE,
			  EFFECT_ASSET_ID::TEXTURED,
			  GEOMETRY_BUFFER_ID::SPRITE });
		break;
	case PLATFORMTYPE::MEDIUM:
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::LEVER_STAND_MEDIUM,
			  EFFECT_ASSET_ID::TEXTURED,
			  GEOMETRY_BUFFER_ID::SPRITE });
		break;
	case PLATFORMTYPE::TALL:
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::LEVER_STAND_TALL,
			  EFFECT_ASSET_ID::TEXTURED,
			  GEOMETRY_BUFFER_ID::SPRITE });
		break;
	case PLATFORMTYPE::GRASSSHORT:
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::LEVER_STAND_SHORT_GRASS,
			  EFFECT_ASSET_ID::TEXTURED,
			  GEOMETRY_BUFFER_ID::SPRITE });
		break;
	case PLATFORMTYPE::GRASSLARGE:
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::LEVER_STAND_LARGE_GRASS,
			  EFFECT_ASSET_ID::TEXTURED,
			  GEOMETRY_BUFFER_ID::SPRITE });
		break;
	default:
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::LEVER_STAND,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
		break;
	}

	return entity;

}

Entity createKey(RenderSystem* renderer, vec2 position, vec2 size) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::KEY);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::KEY);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	registry.collectables.emplace(entity);

	auto& bounce = registry.bounces.emplace(entity);
	bounce.start = position - vec2({ 0.f, 20.f });
	bounce.end = position + vec2({0.f, 20.f });

	// Set the initial motion values for the platform door
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, -15.f };
	motion.scale = size;

	registry.keys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::KEY
		});

	return entity;
}

Entity createLever(RenderSystem* renderer, vec2 position, vec2 size, LEVERSTATE state, Entity door) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Set the initial motion values for the platform door
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	auto& lever = registry.levers.emplace(entity);
	auto& lever_door_link = registry.leverDoorLinks.emplace(entity);
	lever_door_link.door = door;

	//lever_door_map.insert(entity, door);

	switch (state) {
	case LEVERSTATE::PULLED:
		lever.state = LEVERSTATE::PULLED;
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::PULLED_LEVER,
			  EFFECT_ASSET_ID::TEXTURED,
			  GEOMETRY_BUFFER_ID::SPRITE });
		break;
	default:
		lever.state = LEVERSTATE::UNTOUCHED;
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::UNTOUCHED_LEVER,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});
		break;
	}

	return entity;

}

Entity createBrickWallTile(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ _16_BY_16_SCALE, _16_BY_16_SCALE });

	// Make barrel collidable
	registry.collidables.emplace(entity);


	// Create request to render the barrel
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLATFORM_WALL_BRICK,
				  EFFECT_ASSET_ID::TEXTURED,
				  GEOMETRY_BUFFER_ID::SPRITE, -1 });
	return entity;
}

Entity createWindow(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ _16_BY_16_SCALE, _16_BY_16_SCALE });

	// Make barrel collidable
	registry.collidables.emplace(entity);

	// Create request to render the barrel
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WINDOW,
				  EFFECT_ASSET_ID::TEXTURED,
				  GEOMETRY_BUFFER_ID::SPRITE });
	return entity;
}

Entity createTorch(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ _16_BY_16_SCALE, _16_BY_16_SCALE });

	// Make barrel collidable
	registry.collidables.emplace(entity);

	// Create request to render the barrel
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TORCH,
				  EFFECT_ASSET_ID::TEXTURED,
				  GEOMETRY_BUFFER_ID::SPRITE });
	return entity;
}

// Create a room based on the metadata inside the room. Prototype-based builder.
Entity createPlatformRoom(RenderSystem* renderer, const Room&& room) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	auto player = registry.players.entities[0];
	if (!registry.physics.has(player))
		registry.physics.emplace(player);
	auto& player_motion = registry.motions.get(player);
	player_motion.position = {0.f, 100.f};

	// Set the initial motion values for the room
	Motion& motion = registry.motions.emplace(entity);
	motion.position = room.position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = room.size;

	// Create and add the tiled wall
	registry.rooms.emplace(entity, room);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLATFORM_WALL,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		 -1});

	float start_cord = -room.size.x/2.5f;
	while (start_cord < room.size.x/2.5f) {

		// re-use barrel code to add bricks, torches and windows randomly on the backwall without overlap
		for (int i = 0; i < 100; i++) {
			float x = uniform_dist(rng) * 1600.f;
			float y = uniform_dist(rng) * 800.f;
			bool object_collides = false;
			for (Entity e : registry.motions.entities) {
				Motion& m = registry.motions.get(e);
				if (length(vec2(start_cord+x, y) - m.position) < 100.f) {
					object_collides = true;
					break;
				}
			}
			if (!object_collides) {
				if ((int)x % 2 == 0) {
					createBrickWallTile(renderer, { start_cord + x, y });
				}
				else if ((int)x % 3 == 0) {
					createTorch(renderer, { start_cord + x, y });
				}
				else {
					createWindow(renderer, { start_cord + x, y });
				}
			}
		}

		start_cord += 1600.f;
	}



	// Create left straight platform floor segment
	createPlatformFloor(renderer, { 200.f, 400.f }, { room.size.x, 161.f }, PLATFORMFLOOR::GENERIC);


	// stamp sections sections along the platform Room size is 10000 by 10000
	start_cord = -room.size.x / 3.f;
	while (start_cord < room.size.x / 3.f) {
		// create tall lever platforms to mount entrance door
		createPlatform(renderer, { 195.f + start_cord, 270.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);

		// create spikes near large platform
		createPlatformSpikes(renderer, { 310.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::SHORT);
		// create tall lever platforms to mount exit door
	//createLeverPlatform(renderer, { 950.f, 225.f }, { 144.f, 144.f }, LEVERSTAND::LARGE);

		// create spikes near platform floor segment
		createPlatformSpikes(renderer, { 600.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);
		// create spikes near platform floor segment
		createPlatformSpikes(renderer, { 700.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);
		// Create right staircase and platform floor segment
		createPlatformFloor(renderer, { 910.f + start_cord, 300.f }, { 340.f, 46.f }, PLATFORMFLOOR::STAIRS);
		// Create spikes on the stairs
		createPlatformSpikes(renderer, { 1010.f + start_cord, 270.f }, { 90.f, 25.f }, PLATFORMSPIKES::SHORT);

		Entity moving_platform = createMeshPlatform(renderer,
			{ 620.f + start_cord, 100.f },
			{ 144.f, 30.f },
			{ 0.f, 0.f }, PLATFORMTYPE::SHORT);


		auto& bounce = registry.bounces.emplace(moving_platform);
		bounce.start = vec2({ 620.f + start_cord, 0.f });
		bounce.end = vec2({ 620.f + start_cord, 250.f });

		createMeshPlatform(renderer, { 820.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::SHORT);

		createMeshPlatform(renderer, { 420.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::SHORT);

		createKey(renderer, { 820.f + start_cord, -100.f }, { 15.f, 40.f });

		createChest(renderer, { 420.f + start_cord, -80.f }, { 53.f, 45.f }, false);
		createPlatformSpikes(renderer, { 460.f + start_cord, -60.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORT);

		createMeshPlatform(renderer, { 1020.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::SHORT);
		createMeshPlatform(renderer, { 220.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::SHORT);
		createKey(renderer, { 1020.f + start_cord, -100.f }, { 15.f, 40.f });
		createPlatformSpikes(renderer, { 1060.f + start_cord, -60.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORT);

		createChest(renderer, { 220.f + start_cord, -80.f }, { 53.f, 45.f }, false);
		createPlatformSpikes(renderer, { 260.f + start_cord, -60.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORT);
		createMeshPlatform(renderer, { 1220.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::SHORT);
		createVase(renderer, vec2({ 1190.f + start_cord, -80.f }), { 40.f, 50.f });
		createBox(renderer, vec2({ 1240.f + start_cord, -80.f }), { 70.f, 50.f });
		createPlatformBarrel(renderer, vec2({ 1270.f + start_cord, -80.f }), { 40.f, 50.f });

		// create tall lever platforms to mount entrance door
		createPlatform(renderer, { 1400.f + start_cord, 270.f }, { 160.f, 144.f }, PLATFORMTYPE::LARGE);
		createPlatform(renderer, { 1400.f + start_cord, 128.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);
		// create spikes near platform floor segment
		createPlatformSpikes(renderer, { 1510.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);

		createMeshPlatform(renderer, { 1650.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::SHORT);
		createVase(renderer, vec2({ 1690.f + start_cord, -80.f }), { 40.f, 50.f });
		createBox(renderer, vec2({ 1640.f + start_cord, -80.f }), { 70.f, 50.f });
		createPlatformBarrel(renderer, vec2({ 1670.f + start_cord, -80.f }), { 40.f, 50.f });

		createPlatform(renderer, { 1900.f + start_cord, 270.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);
		createPlatformSpikes(renderer, { 1880.f + start_cord, 190.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);
		createPlatform(renderer, { 1900.f + start_cord, 128.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);
		createPlatformSpikes(renderer, { 1880.f + start_cord, 60.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);
		createPlatform(renderer, { 1900.f + start_cord, -15.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);

		createPlatform(renderer, { 2200.f + start_cord, 128.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);
		createPlatformSpikes(renderer, { 2240.f + start_cord, 50.f }, { 40.f, 12.f }, PLATFORMSPIKES::SHORT);
		createVase(renderer, vec2({ 2220.f + start_cord, 30.f }), { 40.f, 50.f });
		createBox(renderer, vec2({ 2180.f + start_cord, 30.f }), { 70.f, 50.f });
		createPlatformBarrel(renderer, vec2({ 2200.f + start_cord, 30.f }), { 40.f, 50.f });

		createPlatformSpikes(renderer, { 2200.f + start_cord, 280.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);

		createPlatformFloor(renderer, { 2310.f + start_cord, 300.f }, { 340.f, 46.f }, PLATFORMFLOOR::STAIRS);
		createPlatformSpikes(renderer, { 2420.f + start_cord, -90.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORT);
		createPlatform(renderer, { 2380.f + start_cord, -15.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);
		createKey(renderer, { 2380.f + start_cord, -110.f }, { 15.f, 40.f });
		createChest(renderer, { 2380.f + start_cord, 270.f }, { 53.f, 45.f }, false);

		createPlatform(renderer, { 2620.f + start_cord, -15.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);
		createPlatformSpikes(renderer, { 2650.f + start_cord, -90.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORT);

		createPlatform(renderer, { 2720.f + start_cord, -215.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);
		createPlatformSpikes(renderer, { 2750.f + start_cord, -290.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORT);
		createPlatformSpikes(renderer, { 2600.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);
		createPlatformSpikes(renderer, { 2690.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);
		createPlatformSpikes(renderer, { 2780.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);
		createPlatformSpikes(renderer, { 2865.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALL);

		start_cord += room.size.x / 3.f;
	}

	start_cord -= 1250.f;

	// Create platform exit door
	createMeshPlatform(renderer, { 980.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::SHORT);
	Entity door = createPlatformDoor(renderer, { 950.f + start_cord, 280.f }, { 101.f, 102.f }, DOORTYPE::CLOSED, false, ROOM::THIRD, PLATFORMTYPE::SHORT);
	createLever(renderer, { 980.f + start_cord, -90.f }, { 54.6f, 58.f }, LEVERSTATE::UNTOUCHED, door);


	createPlatform(renderer, { 1120.f + start_cord, 270.f }, { 144.f, 144.f }, PLATFORMTYPE::LARGE);
	createPlatformSpikes(renderer, { 1140.f + start_cord, 190.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORT);


	return entity;
}

Entity createShop(RenderSystem* renderer, unsigned int points) {

	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = { window_width_f / 2.f, window_height_f / 2.f };

	registry.shops.emplace(entity);

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ window_width_f, window_height_f });

	//float item_width = (window_width_f / 3.f) - (2.f * margin);
	//float item_height = (window_height_f / 3.f) - (2.f * margin);

	float item_xpos_increment = window_width_f/6.f;
	float item_ypos_increment = window_height_f/6.f;

	vec2 item1_pos = { item_xpos_increment, item_ypos_increment + window_height_f / 12.f };
	vec2 item2_pos = { item_xpos_increment * 3, item_ypos_increment + window_height_f / 12.f };
	vec2 item3_pos = { item_xpos_increment * 5, item_ypos_increment + window_height_f / 12.f };
	vec2 item4_pos = { item_xpos_increment, item_ypos_increment * 3 + window_height_f / 24.f };
	vec2 item5_pos = { item_xpos_increment * 3, item_ypos_increment * 3 + window_height_f / 24.f };
	vec2 item6_pos = { item_xpos_increment * 5, item_ypos_increment * 3 + window_height_f / 24.f };

	createShopItem(renderer, TEXTURE_ASSET_ID::SWORD, item1_pos);
	createShopItem(renderer, TEXTURE_ASSET_ID::ASSAULT_RIFLE, item2_pos);
	createShopItem(renderer, TEXTURE_ASSET_ID::LASER_GUN, item3_pos);
	createShopItem(renderer, TEXTURE_ASSET_ID::PISTOL, item4_pos);
	createShopItem(renderer, TEXTURE_ASSET_ID::SHOTGUN, item5_pos);
	

	Entity points_text = createText(renderer, { window_width_f / 2.f - 300.f, window_height_f / 8.f - 150.f}, "Store", 3.f, {1.0, 1.0, 1.0}, 8);
	registry.shops.emplace(points_text);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SHOP_BACKGROUND,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE,
		7 });

	return entity;

}

Entity createShopItem(RenderSystem* renderer, TEXTURE_ASSET_ID item, vec2 pos) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	float margin = 200.f;
	float item_width = (window_width_f / 3.f) - (2.f * margin);
	float item_height = (window_height_f / 3.f) - (2.f * margin);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = pos;
	motion.scale = vec2({ _64_BY_64_SCALE, _64_BY_64_SCALE });
	//printf("values: %f.1, %f.1\n", motion.scale.x, motion.scale.y);

	auto& shop_item = registry.shopItems.emplace(entity);
	auto& button = registry.buttons.emplace(entity);
	button.type = BUTTONTYPE::SHOPITEM;

	auto& _item = registry.items.emplace(entity);

	//render item
	registry.renderRequests.insert(
		entity,
		{ item,
		  EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE,
		8 });


	switch (item) {
		case TEXTURE_ASSET_ID::SWORD:
			shop_item.price = 50;
			shop_item.name = "Sword";
			_item.type = ITEM_TYPE::SWORD;
			break;
		case TEXTURE_ASSET_ID::ASSAULT_RIFLE:
			shop_item.price = 80;
			shop_item.name = "Assault Rifle";
			_item.type = ITEM_TYPE::ASSAULT_RIFLE;
			break;
		case TEXTURE_ASSET_ID::LASER_GUN:
			shop_item.price = 100;
			shop_item.name = "Laser Gun";
			_item.type = ITEM_TYPE::LASER_GUN;
			break;
		case TEXTURE_ASSET_ID::PISTOL:
			shop_item.price = 40;
			shop_item.name = "Pistol";
			_item.type = ITEM_TYPE::PISTOL;
			break;
		case TEXTURE_ASSET_ID::SHOTGUN:
			shop_item.price = 30;
			shop_item.name = "Shotgun";
			_item.type = ITEM_TYPE::SHOTGUN;
			break;
	}
	

	auto item_name = createText(renderer, { pos.x - window_width_f / 24.f, pos.y + 200.f }, shop_item.name, 1.f, { 1.0, 1.0, 1.0 }, 8);
	auto& item_name_text = registry.texts.get(item_name);

	if (shop_item.name.length() % 2 == 0) {
		item_name_text.bit_shift = shop_item.name.length() / 2;
	}
	else {
		item_name_text.bit_shift = (shop_item.name.length() + 1) / 2;
	}
	registry.shops.emplace(item_name);

	auto item_price = createText(renderer, { pos.x, pos.y + 250.f }, "$" + std::to_string(shop_item.price), 1.f, {1.0, 1.0, 1.0}, 8);
	registry.shops.emplace(item_price);

	return entity;
}

Entity createRoseGardenRoom(RenderSystem* renderer, const Room&& room) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	auto player = registry.players.entities[0];
	if (!registry.physics.has(player))
		registry.physics.emplace(player);
	auto& player_motion = registry.motions.get(player);
	player_motion.position = { 0.f, 100.f };

	// Set the initial motion values for the room
	Motion& motion = registry.motions.emplace(entity);
	motion.position = room.position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = room.size;

	// Create and add the tiled wall
	registry.rooms.emplace(entity, room);

	// render parallax background
	renderSky(renderer, vec2{ (room.position.x, room.position.y + 1500.f) }, 3.f * room.size, false);
	renderMountain(renderer, vec2{ (room.position.x, room.position.y + 70.f)}, vec2({ 3.f * room.size.x, 680.f }), false);
	renderGrass(renderer, vec2{ (room.position.x, room.position.y - 180.f) }, vec2({ 3.f * room.size.x, 1080.f }), false);
	renderSky(renderer, vec2{ (room.position.x, room.position.y - 1000.f) }, vec2({ 3.f * room.size.x, 1080.f }), false);
	

	//render blurred reflection
	renderSky(renderer, vec2{ (room.position.x, room.position.y - 5000.f) }, 3.f * room.size, true);
	renderMountain(renderer, vec2{ (room.position.x, room.position.y - 750.f) }, vec2({ 9.f * room.size.x + 280.f, 680.f }), true);
	renderGrass(renderer, vec2{ (room.position.x, room.position.y - 950.f) }, vec2({ 9.f * room.size.x + 280.f, 1080.f }), true);
	renderSky(renderer, vec2{ (room.position.x, room.position.y) - 5000.f }, vec2({ 9.f * room.size.x + 280.f, 1080.f }), true);

	renderParallaxTutorial(renderer, vec2({-3500.f, 80.f}), vec2({600.f, 400.f}), false);


	// Create left straight platform floor segment
	createPlatformFloor(renderer, { 200.f, 400.f }, { 10000.f, 161.f }, PLATFORMFLOOR::GENERICGRASS);

	// stamp sections sections along the platform Room size is 10000 by 10000
	float start_cord = -room.size.x/ 3.f;
	while (start_cord < room.size.x/ 3.f) {
		// create tall lever platforms to mount entrance door
		createPlatform(renderer, { 195.f + start_cord, 270.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		createRoseBush(renderer, { 195.f + start_cord, 150.f });

		// create spikes near large platform
		createPlatformSpikes(renderer, { 310.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::SHORTGRASS);
		// create tall lever platforms to mount exit door
	//createLeverPlatform(renderer, { 950.f, 225.f }, { 144.f, 144.f }, LEVERSTAND::LARGE);
		
		// create spikes near platform floor segment
		createPlatformSpikes(renderer, { 600.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);
		// create spikes near platform floor segment
		createPlatformSpikes(renderer, { 700.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);
		// Create right staircase and platform floor segment
		createPlatformFloor(renderer, { 910.f + start_cord, 300.f }, { 340.f, 46.f }, PLATFORMFLOOR::STAIRSGRASS);
		// Create spikes on the stairs
		createPlatformSpikes(renderer, { 1010.f + start_cord, 270.f }, { 90.f, 25.f }, PLATFORMSPIKES::SHORTGRASS);

		Entity moving_platform = createMeshPlatform(renderer,
			{ 620.f + start_cord, 100.f },
			{ 144.f, 30.f },
			{ 0.f, 0.f }, PLATFORMTYPE::GRASSSHORT);
		

		auto& bounce = registry.bounces.emplace(moving_platform);
		bounce.start = vec2({ 620.f + start_cord, 0.f });
		bounce.end = vec2({ 620.f + start_cord, 250.f });

		createMeshPlatform(renderer, { 820.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::GRASSSHORT);
		createRoseBush(renderer, { 820.f + start_cord, -100.f });

		createMeshPlatform(renderer, { 420.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::GRASSSHORT);
		createRoseBush(renderer, { 420.f + start_cord, -100.f });

		createKey(renderer, { 820.f + start_cord, -100.f }, { 15.f, 40.f });

		createChest(renderer, { 420.f + start_cord, -80.f }, { 53.f, 45.f }, false);
		createPlatformSpikes(renderer, { 460.f + start_cord, -60.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORTGRASS);

		createMeshPlatform(renderer, { 1020.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::GRASSSHORT);
		createRoseBush(renderer, { 1020.f + start_cord, -100.f });
		createMeshPlatform(renderer, { 220.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::GRASSSHORT);
		createKey(renderer, { 1020.f + start_cord, -100.f }, { 15.f, 40.f });
		createPlatformSpikes(renderer, { 1060.f + start_cord, -60.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORTGRASS);
		createRoseBush(renderer, { 220.f + start_cord, -100.f });
	
		createChest(renderer, { 220.f + start_cord, -80.f }, { 53.f, 45.f }, false);
		createPlatformSpikes(renderer, { 260.f + start_cord, -60.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORTGRASS);
		createMeshPlatform(renderer, { 1220.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::GRASSSHORT);
		createVase(renderer, vec2({ 1190.f + start_cord, -80.f }), { 40.f, 50.f });
		createBox(renderer, vec2({ 1240.f + start_cord, -80.f }), { 70.f, 50.f });
		createPlatformBarrel(renderer, vec2({ 1270.f + start_cord, -80.f }), { 40.f, 50.f });

		// create tall lever platforms to mount entrance door
		createPlatform(renderer, { 1400.f + start_cord, 270.f }, { 160.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		createPlatform(renderer, { 1400.f + start_cord, 128.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		// create spikes near platform floor segment
		createPlatformSpikes(renderer, { 1510.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);

		createMeshPlatform(renderer, { 1650.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::GRASSSHORT);
		createVase(renderer, vec2({ 1690.f + start_cord, -80.f }), { 40.f, 50.f });
		createBox(renderer, vec2({ 1640.f + start_cord, -80.f }), { 70.f, 50.f });
		createPlatformBarrel(renderer, vec2({ 1670.f + start_cord, -80.f }), { 40.f, 50.f });

		createPlatform(renderer, { 1900.f + start_cord, 270.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		createPlatformSpikes(renderer, { 1880.f + start_cord, 190.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);
		createPlatform(renderer, { 1900.f + start_cord, 128.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		createPlatformSpikes(renderer, { 1880.f + start_cord, 60.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);
		createPlatform(renderer, { 1900.f + start_cord, -15.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		createRoseBush(renderer, { 1900.f + start_cord, -120.f });

		createPlatform(renderer, { 2200.f + start_cord, 128.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		createPlatformSpikes(renderer, { 2240.f + start_cord, 50.f }, { 40.f, 12.f }, PLATFORMSPIKES::SHORTGRASS);
		createVase(renderer, vec2({ 2220.f + start_cord, 30.f }), { 40.f, 50.f });
		createBox(renderer, vec2({ 2180.f + start_cord, 30.f }), { 70.f, 50.f });
		createPlatformBarrel(renderer, vec2({ 2200.f + start_cord, 30.f }), { 40.f, 50.f });

		createPlatformSpikes(renderer, { 2200.f + start_cord, 280.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);

		createPlatformFloor(renderer, { 2310.f + start_cord, 300.f }, { 340.f, 46.f }, PLATFORMFLOOR::STAIRSGRASS);
		createRoseBush(renderer, { 2310.f + start_cord, 250.f });
		createRoseBush(renderer, { 2350.f + start_cord, 250.f });
		createRoseBush(renderer, { 2400.f + start_cord, 250.f });
		createRoseBush(renderer, { 2450.f + start_cord, 250.f });
		createPlatformSpikes(renderer, { 2420.f + start_cord, -90.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORTGRASS);
		createPlatform(renderer, { 2380.f + start_cord, -15.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		createKey(renderer, { 2380.f + start_cord, -110.f }, { 15.f, 40.f });
		createChest(renderer, { 2380.f + start_cord, 270.f }, { 53.f, 45.f }, false);

		createPlatform(renderer, { 2620.f + start_cord, -15.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		createPlatformSpikes(renderer, { 2650.f + start_cord, -90.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORTGRASS);

		createPlatform(renderer, { 2720.f + start_cord, -215.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
		createPlatformSpikes(renderer, { 2750.f + start_cord, -290.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORTGRASS);
		createPlatformSpikes(renderer, { 2600.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);
		createPlatformSpikes(renderer, { 2690.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);
		createPlatformSpikes(renderer, { 2780.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);
		createPlatformSpikes(renderer, { 2865.f + start_cord, 310.f }, { 90.f, 25.f }, PLATFORMSPIKES::TALLGRASS);
		createRoseBush(renderer, { 2910.f + start_cord, 270.f });
		createRoseBush(renderer, { 2950.f + start_cord, 270.f });
		createRoseBush(renderer, { 3000.f + start_cord, 270.f });
		createRoseBush(renderer, { 3050.f + start_cord, 270.f });
		
		start_cord += room.size.x/3.f;
	}

	start_cord -= 1250.f;

	// Create platform exit door
	createMeshPlatform(renderer, { 980.f + start_cord, -50.f }, { 144.f, 21.f }, { 0.f, 0.f }, PLATFORMTYPE::GRASSSHORT);
	Entity door = createPlatformDoor(renderer, { 950.f + start_cord, 280.f }, { 101.f, 102.f }, DOORTYPE::CLOSED, false, ROOM::FIRST, PLATFORMTYPE::GRASSSHORT);
	createLever(renderer, { 980.f + start_cord, -90.f }, { 54.6f, 58.f }, LEVERSTATE::UNTOUCHED, door);


	createPlatform(renderer, { 1120.f + start_cord, 270.f }, { 144.f, 144.f }, PLATFORMTYPE::GRASSLARGE);
	createPlatformSpikes(renderer, { 1140.f + start_cord, 190.f }, { 60.f, 12.f }, PLATFORMSPIKES::SHORTGRASS);


	//renderSoil(renderer, vec2{ room.position.x, room.position.y + 720.f }, vec2{ 4.f*room.size.x, 680.f});
	
	return entity;

}


Entity renderSky(RenderSystem* renderer, vec2 position, vec2 size, bool reflected) {
	auto entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	auto& parallax = registry.parallaxes.emplace(entity);
	parallax.parallaxSpeed = 5.f;

	if (reflected) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::SKY,
					  EFFECT_ASSET_ID::REFLECTION,
					  GEOMETRY_BUFFER_ID::SPRITE, -1 });
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::SKY,
					  EFFECT_ASSET_ID::TEXTURED,
					  GEOMETRY_BUFFER_ID::SPRITE, -1 });
	}
	return entity;
};

Entity renderMountain(RenderSystem* renderer, vec2 position, vec2 size, bool reflected) {
	auto entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	auto& parallax = registry.parallaxes.emplace(entity);
	parallax.parallaxSpeed = 10.f;
	if (reflected) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::MOUNTAIN,
					  EFFECT_ASSET_ID::REFLECTION,
					  GEOMETRY_BUFFER_ID::SPRITE, -1 });
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::MOUNTAIN,
					  EFFECT_ASSET_ID::TEXTURED,
					  GEOMETRY_BUFFER_ID::SPRITE, -1 });
	}
	return entity;
};

Entity renderGrass(RenderSystem* renderer, vec2 position, vec2 size, bool reflected) {
	auto entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	auto& parallax = registry.parallaxes.emplace(entity);
	parallax.parallaxSpeed = 30.f;

	if (reflected) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::GRASS,
					  EFFECT_ASSET_ID::REFLECTION,
					  GEOMETRY_BUFFER_ID::SPRITE, -1 });
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::GRASS,
					  EFFECT_ASSET_ID::TEXTURED,
					  GEOMETRY_BUFFER_ID::SPRITE, -1 });
	}
	return entity;
};

Entity renderParallaxTutorial(RenderSystem* renderer, vec2 position, vec2 size, bool reflected) {
	auto entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	auto& tutorial_background = registry.tutorialBackgrounds.emplace(entity);
	auto& parallax = registry.parallaxes.emplace(entity);
	parallax.parallaxSpeed = 0.f;

	if (reflected) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TUTORIAL_PlATFORM,
					  EFFECT_ASSET_ID::REFLECTION,
					  GEOMETRY_BUFFER_ID::SPRITE, 0 });
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TUTORIAL_PlATFORM,
					  EFFECT_ASSET_ID::TEXTURED,
					  GEOMETRY_BUFFER_ID::SPRITE, 0 });
	}
	return entity;
};


Entity renderSoil(RenderSystem* renderer, vec2 position, vec2 size) {
	auto entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;
	auto& parallax = registry.parallaxes.emplace(entity);
	parallax.parallaxSpeed = 30.f;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SOIL,
				  EFFECT_ASSET_ID::TEXTURED,
				  GEOMETRY_BUFFER_ID::SPRITE});
	return entity;
};



Entity createBarrel(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 30.f, 40.f });

	// Make barrel collidable
	registry.collidables.emplace(entity);

	// TODO: make global map that maps hittable object to their health + hit
	auto& hit = registry.hits.emplace(entity);
	hit.hp = 8;

	// Create request to render the barrel
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BARREL,
				  EFFECT_ASSET_ID::TEXTURED,
				  GEOMETRY_BUFFER_ID::SPRITE });
	return entity;
}

Entity createCow(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	// Create and (empty) Enemy component to be able to refer to all enemies
	registry.cows.emplace(entity);
	registry.softShells.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::COW,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}


Entity createHostileCow(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	// Create and (empty) Enemy component to be able to refer to all enemies
	registry.hardShells.emplace(entity);
	registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::HOSTILE_COW,
			  EFFECT_ASSET_ID::TEXTURED,
			  GEOMETRY_BUFFER_ID::SPRITE,
			  1});

	return entity;
}


Entity createSpike(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ _16_BY_16_SCALE, _16_BY_16_SCALE });

	// Make spike a CollisionDamageDealer entity.
	auto& cdd = registry.hardShells.emplace(entity);

	// Create request to render the spike
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SPIKE,
						  EFFECT_ASSET_ID::TEXTURED,
						  GEOMETRY_BUFFER_ID::SPRITE,
		1});
	return entity;
}

Entity createPlatformSpikes(RenderSystem* renderer, vec2 position, vec2 size, PLATFORMSPIKES type) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	// Make spike a CollisionDamageDealer entity.
	auto& cdd = registry.hardShells.emplace(entity);

	switch (type) {
	case PLATFORMSPIKES::SHORT:
		// Create request to render the spike
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::PLATFORM_SPIKES_SHORT,
							  EFFECT_ASSET_ID::TEXTURED,
							  GEOMETRY_BUFFER_ID::SPRITE,
			1 });
		break;
	case PLATFORMSPIKES::SHORTGRASS:
		// Create request to render the spike
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::GRASS_SPIKES_SHORT,
							  EFFECT_ASSET_ID::TEXTURED,
							  GEOMETRY_BUFFER_ID::SPRITE,
			1 });
		break;
	case PLATFORMSPIKES::TALLGRASS:
		// Create request to render the spike
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::GRASS_SPIKES_TALL,
							  EFFECT_ASSET_ID::TEXTURED,
							  GEOMETRY_BUFFER_ID::SPRITE,
			1 });
		break;
	default:
		// Create request to render the spike
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::PLATFORM_SPIKES_TALL,
							  EFFECT_ASSET_ID::TEXTURED,
							  GEOMETRY_BUFFER_ID::SPRITE,
			1 });
		break;

	}
	
	return entity;
}

Entity createButton(RenderSystem* renderer, vec2 position, BUTTONTYPE type) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	//add to help button
	auto& button = registry.buttons.emplace(entity);
	button.type = type;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ BUTTON_BB_WIDTH, BUTTON_BB_HEIGHT });

	if (type == BUTTONTYPE::HELP) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::HELP_BUTTON,
						 EFFECT_ASSET_ID::UI,
						 GEOMETRY_BUFFER_ID::SPRITE,
						3 });
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::SHOP_BUTTON,
						 EFFECT_ASSET_ID::UI,
						 GEOMETRY_BUFFER_ID::SPRITE,
						3 });
	}

	return entity;
}

Entity createTutorialMessage(RenderSystem* renderer, vec2 position, TUTORIALTYPE type) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// add to tutorial message
	auto& tutorialMessage = registry.tutorialMessages.emplace(entity);
	tutorialMessage.show = true;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ window_width_f - 400.f, window_height_f - 400.f });

	if (type == TUTORIALTYPE::PLATFORM) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TUTORIAL_PlATFORM,
			 EFFECT_ASSET_ID::UI,
			 GEOMETRY_BUFFER_ID::SPRITE,
			18});
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TUTORIAL,
			 EFFECT_ASSET_ID::UI,
			 GEOMETRY_BUFFER_ID::SPRITE,
			18});
	}
	return entity;
}

// Creates an item entity and render it on the screen for pickup.
Entity createGroundItem(RenderSystem* renderer, vec2 position, Item item) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ _16_BY_16_SCALE, _16_BY_16_SCALE });

	// Make item collidable
	registry.collidables.emplace(entity);

	// Make item collectable (require key press 'e' to pick up)
	Collectable& collectable = registry.collectables.emplace(entity);
	collectable.require_key_press = true;
	collectable.inventoryCollectable = true;

	// Maps item type to texture id using static map itemTextureMap
	registry.renderRequests.insert(
		entity,
		{ itemTextureMap[item.type],
		  EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		2 });

	// Create item component
	auto& item_ = registry.items.emplace(entity);
	item_.quantity = item.quantity;
	item_.type = item.type;
	item_.stats = item.stats;

	return entity;
}

// Inventory related functions
Entity createInventory(Entity holder, int inventorySize) {
	Inventory& inventory = registry.inventories.emplace(holder);
	inventory.items.resize(inventorySize);
	inventory.emptySlots.resize(inventorySize);
	inventory.slots.resize(inventorySize);
	for (int i = 0; i < inventorySize; i++) {
		inventory.slots[i] = createInventorySlot();
		inventory.emptySlots[inventorySize - i - 1] = i;
	}
	inventory.selectionBorder = createSelectionBorder();
	return holder;
}

Entity createInventorySlot() {
	auto entity = Entity();
	registry.inventorySlotUIs.emplace(entity);
	// Setting initial motion values
	registry.motions.emplace(entity);
	return entity;
}

Entity createSelectionBorder() {
	printf("Creating selection border\n");
	auto entity = Entity();
	// Setting initial motion values
	registry.motions.emplace(entity);
	return entity;
}

// Render inventory on the screen via adding RenderRequests
void renderInventory(RenderSystem* renderer, vec2 position) {
	auto player = registry.players.entities[0];
	Inventory& inventory = registry.inventories.get(player);

	// Render inventory slots
	for (int i = 0; i < inventory.slots.size(); i++) {
		float xOffset = static_cast<float>(i) * INVENTORY_SLOT_SIZE;
		vec2 slotPosition = { position.x + xOffset, position.y };

		// Temp for testing
		auto empty_item = Entity();
		registry.items.emplace(empty_item);
		//renderInventorySlot(renderer, slotPosition, inventory.slots[i], empty_item);
		// TODO: inventory needs to be copied over;
		renderInventorySlot(renderer, slotPosition, inventory.slots[i], inventory.items[i]);
	}

	// Render selected slot to have special border
	float xOffset_border = static_cast<float>(inventory.selectedIndex) * INVENTORY_SLOT_SIZE;
	vec2 slotPosition = { position.x + xOffset_border, position.y };
	renderSelectionBorder(renderer, position, inventory.selectionBorder, inventory.selectedIndex);
}

void renderInventorySlot(RenderSystem* renderer, vec2 position, Entity slot, Entity item) {
	// Render slot (TODO: could optimize later)
	if (!registry.renderRequests.has(slot)) {
		registry.renderRequests.insert(
			slot,
			{ TEXTURE_ASSET_ID::INVENTORY_SLOT,
			  EFFECT_ASSET_ID::UI,
			  GEOMETRY_BUFFER_ID::SPRITE,
			9 });
	}

	// Set position of slot.
	Motion& motion = registry.motions.get(slot);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ _16_BY_16_SCALE, _16_BY_16_SCALE });
	

	// Render item in slot if there is one
	if (registry.items.has(item)) {
		renderItemInSlot(renderer, position, slot, item);
	}
}

void renderItemInSlot(RenderSystem* renderer, vec2 position, Entity slot, Entity item) {
	// Render item in slot (TODO: change to render sprite based on item type)
	Item item_ = registry.items.get(item);
	if (!registry.renderRequests.has(item)) {
		registry.renderRequests.insert(
			item,
			{ itemTextureMap[item_.type],
			  EFFECT_ASSET_ID::UI,
			  GEOMETRY_BUFFER_ID::SPRITE,
			15 });
	}
	else {
		RenderRequest& renderRequest = registry.renderRequests.get(item);
		renderRequest.used_texture = itemTextureMap[item_.type];
		renderRequest.used_effect = EFFECT_ASSET_ID::UI;
		renderRequest.used_geometry = GEOMETRY_BUFFER_ID::SPRITE;
	}

	// Get position of slot (motion)
	Motion& slotMotion = registry.motions.get(slot);
	vec2 slotPosition = slotMotion.position;

	if (!registry.motions.has(item)) {
		registry.motions.emplace(item);
	}

	Motion& itemMotion = registry.motions.get(item);
	itemMotion.position = slotPosition;
	itemMotion.angle = 0.f;
	itemMotion.velocity = { 0, 0.f };
	itemMotion.scale = vec2({ _16_BY_16_SCALE, _16_BY_16_SCALE });

}

void renderSelectionBorder(RenderSystem* renderer, vec2 inventoryPosition, Entity selectionBorder, int index) {
	// Render selection border (TODO: could optimize later)
	if (!registry.renderRequests.has(selectionBorder)) {
		registry.renderRequests.insert(
			selectionBorder,
			{ TEXTURE_ASSET_ID::INVENTORY_SLOT_SELECTOR,
			  EFFECT_ASSET_ID::UI,
			  GEOMETRY_BUFFER_ID::SPRITE,
			14 });
	}
	else {
		RenderRequest& renderRequest = registry.renderRequests.get(selectionBorder);
		renderRequest.used_texture = TEXTURE_ASSET_ID::INVENTORY_SLOT_SELECTOR;
		renderRequest.used_effect = EFFECT_ASSET_ID::UI;
		renderRequest.used_geometry = GEOMETRY_BUFFER_ID::SPRITE;
	}

	/*registry.renderRequests.remove(selectionBorder);
	registry.renderRequests.insert(
		selectionBorder,
		{ TEXTURE_ASSET_ID::INVENTORY_SLOT_SELECTOR,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });*/

	Motion& motion = registry.motions.get(selectionBorder);
	float xOffset = static_cast<float>(index) * INVENTORY_SLOT_SIZE;
	motion.position = { inventoryPosition.x + xOffset, inventoryPosition.y };
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ _16_BY_16_SCALE, _16_BY_16_SCALE });
}


Entity createChest(RenderSystem* renderer, vec2 position, vec2 size, bool open) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);


	registry.chests.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	// Make barrel collidable
	registry.collidables.emplace(entity);

	// Create request to render the barrel
	if (open) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::CHEST_OPEN,
					  EFFECT_ASSET_ID::TEXTURED,
					  GEOMETRY_BUFFER_ID::SPRITE });
		return entity;
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::CHEST_CLOSED,
					  EFFECT_ASSET_ID::TEXTURED,
					  GEOMETRY_BUFFER_ID::SPRITE });
	}
	return entity;
}


// Creates melee attack with render request
Entity createMeleeAttack(RenderSystem* renderer, Entity attacker, vec2 direction, float dist, float damage, float duration) {

	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);

	// Calculate position of attack based on position of attacker and direction.
	vec2 attack_position = registry.motions.get(attacker).position + direction * dist;
	motion.position = attack_position;

	// Set angle of attack based on direction
	motion.angle = atan2(direction.y, direction.x);

	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 160.f, 160.f });

	// Add attack component
	Attack& attack = registry.attacks.emplace(entity);
	registry.expiry.emplace(entity, Expiry{duration});
	attack.damage = damage;
	attack.aoe = true;

	// Create request to render the barrel
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SWORD_SLASH,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });
	return entity;
}

// Create shotgun attack with spread within a cone (OPTIONAL: make spread a parameter)
Entity createShotgunAttack(RenderSystem* renderer, Entity attacker, vec2 direction, float damage) {
	Motion attacker_motion = registry.motions.get(attacker);

	float coneSpread = 0.1f;

	// Create 5 pebbles in a cone
	for (int i = 0; i < 5; ++i) {
		// Calculate the rotated direction within the cone
		float angleOffset = (i - 2) * coneSpread;
		vec2 rotatedDirection = rotate(direction, angleOffset);

		// Create pebble
		Entity pebble = createPebble(attacker_motion.position, { 10.f, 10.f }, vec3{ 1.f, 1.f, 0.8f });

		// Set motion of pebble to come out with a cone-like shape
		Motion& motion = registry.motions.get(pebble);
		motion.velocity = vec3(rotatedDirection * 2000.f, 0.f);

		// Assign damage and expiry to pebble
		registry.attacks.emplace(pebble, Attack{ damage, false });
		registry.expiry.emplace(pebble, Expiry{ 10000.f });
	}

	return Entity();
}

// Create laser attack.
Entity createLaserAttack(RenderSystem* renderer, Entity attacker, vec2 direction, float length, float damage, float duration) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);

	// Calculate position of attack based on position of attacker and direction.
	vec2 attack_position = registry.motions.get(attacker).position + direction * length * 9.5f;
	motion.position = attack_position;

	// Set angle of attack based on direction
	motion.angle = atan2(direction.y, direction.x);

	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ length * 20, 80.f });

	// Add attack component
	Attack& attack = registry.attacks.emplace(entity);
	registry.expiry.emplace(entity, Expiry{ duration });
	attack.damage = damage;
	attack.aoe = true;

	// Create request to render the barrel. TODO: add laser beam texture.
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::LASER_BEAM,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });
	return entity;
}

Entity createText(RenderSystem* renderer, vec2 pos, std::string message, float scale, vec3 color, int layer)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the text component
	auto& text = registry.texts.emplace(entity);
	text.message = std::move(message);
	text.position = pos;
	text.scale = scale;
	text.color = color;

	// Create an (empty) Bug component to be able to refer to all bug
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::TEXT,
			GEOMETRY_BUFFER_ID::TEXT,
		layer});

	return entity;
}

void createEnemies(RenderSystem* renderer, uint number, Entity center) {
	for (uint i = 0; i < number; i++) {
		Transform transform;
		transform.translate(vec2(1000.f, 1000.f));
		transform.rotate(uniform_dist(rng) * 2.f * M_PI);

		Entity entity;
		if (uniform_dist(rng) < 0.1f) {
			entity = createEnemySlime(renderer, { 0.f,0.f }, true);
		}
		else {
			entity = createEnemy(renderer, { 0.f,0.f });
		}

		// Setting random initial position and constant velocity
		Motion& motion = registry.motions.get(entity);
		Motion& center_motion = registry.motions.get(center);

		motion.position = transform.mat * vec3(1000.f, 1000.f, 0.f)
		                  + vec3(center_motion.position[0], center_motion.position[1], 1.f);
		registry.ai.emplace(entity);
		registry.hits.insert(entity, Hit{5.f});
	}
}


Entity createLifeBarContainer(RenderSystem* renderer, vec2 position, vec2 size, float hp) {
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	auto& lifebarContainer = registry.lifeBarContainers.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::LIFE_BAR_EMPTY,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE,
		2});

	return entity;
}




Entity createLifeBar(RenderSystem* renderer, vec2 position, vec2 size, float maxHp, float currHp) {
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;
	motion.scale.x = size.x - (((maxHp - currHp)/ maxHp)*size.x);
	motion.position.x = position.x - ((((maxHp - currHp) / maxHp) * size.x) /2.5f);

	auto& lifebar = registry.lifebars.emplace(entity);
	lifebar.maxHP = maxHp;
	lifebar.currHP = currHp;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::LIFE_BAR,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE,
		1});

	createLifeBarContainer(renderer, position, vec2({size.x+20.f, size.y+30.f}), maxHp);

	return entity;


}

Entity createVase(RenderSystem* renderer, vec2 position, vec2 size) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);
	registry.weaponContainers.emplace(entity);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	// Make barrel collidable
	registry.collidables.emplace(entity);

	// TODO: make global map that maps hittable object to their health + hit
	auto& hit = registry.hits.emplace(entity);
	hit.hp = 2;

	// Create request to render the barrel
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::VASE,
				  EFFECT_ASSET_ID::TEXTURED,
				  GEOMETRY_BUFFER_ID::SPRITE });
	return entity;
}


Entity createBox(RenderSystem* renderer, vec2 position, vec2 size) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	// Make barrel collidable
	registry.collidables.emplace(entity);
	registry.weaponContainers.emplace(entity);

	// TODO: make global map that maps hittable object to their health + hit
	auto& hit = registry.hits.emplace(entity);
	hit.hp = 4;

	// Create request to render the barrel
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BOX,
				  EFFECT_ASSET_ID::TEXTURED,
				  GEOMETRY_BUFFER_ID::SPRITE });
	return entity;
}

Entity createPlatformBarrel(RenderSystem* renderer, vec2 position, vec2 size) {
	auto entity = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;
	registry.weaponContainers.emplace(entity),

		// Make barrel collidable
		registry.collidables.emplace(entity);

	// TODO: make global map that maps hittable object to their health + hit
	auto& hit = registry.hits.emplace(entity);
	hit.hp = 6;

	// Create request to render the barrel
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLATFORM_BARREL,
				  EFFECT_ASSET_ID::TEXTURED,
				  GEOMETRY_BUFFER_ID::SPRITE });
	return entity;
}

Entity renderPoints(RenderSystem* renderer, vec2 pos, std::string message, float scale, vec3 color, int layer)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the text component
	auto& text = registry.texts.emplace(entity);

	// Initialize the coin counter
	auto& coinCounter = registry.coinCounters.emplace(entity);

	text.message = std::move(message);
	text.position = pos;
	text.scale = scale;
	text.color = color;

	renderCoinSymbol(renderer, vec2({pos.x - 50.f, pos.y + 20.f}), vec2({ 50.f, 50.f }), 9);

	// Create an (empty) Bug component to be able to refer to all bug
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::TEXT,
			GEOMETRY_BUFFER_ID::TEXT, layer
		});

	return entity;
}

Entity renderKeys(RenderSystem* renderer, vec2 pos, std::string message, float scale, vec3 color)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the text component
	auto& text = registry.texts.emplace(entity);

	// Initialize the coin counter
	auto& coinCounter = registry.keyCounters.emplace(entity);

	text.message = std::move(message);
	text.position = pos;
	text.scale = scale;
	text.color = color;

	renderKeySymbol(renderer, vec2({ pos.x - 50.f, pos.y + 20.f }), vec2({ 35.f, 50.f }));

	// Create an (empty) Bug component to be able to refer to all bug
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::TEXT,
			GEOMETRY_BUFFER_ID::TEXT, 2
		});

	return entity;
}

Entity renderCoinSymbol(RenderSystem* renderer, vec2 pos, vec2 size, int layer) {
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;

	auto& key_symbol = registry.coinSymbols.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::COIN,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE, layer
		});

	return entity;
}


Entity renderPlayerSymbol(RenderSystem* renderer, vec2 pos, vec2 size) {
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;

	auto& key_symbol = registry.playerSymbols.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_FRONT,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE,
			2
		});

	return entity;
}


Entity renderCowBossSymbol(RenderSystem* renderer, vec2 pos, vec2 size) {
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;

	auto& key_symbol = registry.cowBossSymbols.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HOSTILE_COW,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE,
			2
		});

	return entity;
}



Entity renderKeySymbol(RenderSystem* renderer, vec2 pos, vec2 size) {
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;

	auto& key_symbol = registry.keySymbols.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::KEY,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE, 2
		});

	return entity;
}

Entity createHealthPotion(RenderSystem* renderer, vec2 position) {
	// Reserve an entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	registry.healthPotions.emplace(entity);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = vec2({ 40.f, 80.f });


	// Make this coin entity collectable.
	registry.collectables.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HEALTH_POTION,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createRoseBush(RenderSystem* renderer, vec2 position) {
	// Reserve an entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Mesh& collision_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.collisionMeshPtrs.emplace(entity, &collision_mesh);

	// Make barrel collidable
	registry.collidables.emplace(entity);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = vec2({ 50.f, 100.f });


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ROSE_BUSH,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}