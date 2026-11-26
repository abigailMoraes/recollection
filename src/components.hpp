#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

enum class ROOM {
	FIRST,
	SECOND,
	THIRD,
	COW,
	BOSS,
	ROSEGARDEN,
	SHOP,
};

// represent player direction
enum class DIRECTION {
	FRONT,
	BACK,
	LEFT,
	RIGHT,
	FRONT_LEFT,
	FRONT_RIGHT,
	BACK_LEFT,
	BACK_RIGHT
};

enum class PLAYER_STATE {
	IDLE,       // standing still
	WALKING,    // moving
	JUMPING,     // in the air
	DEAD
};

enum class BOSS_STATE {
	SPAWNING_COW,
	SPEEDING, 
	RANGE_ATTACKING,
	WALKING
};


// Player component
struct Player
{
	DIRECTION direction = DIRECTION::FRONT;
	PLAYER_STATE state = PLAYER_STATE::IDLE;

	float base_speed = 250.f;
	float base_defense = 0.f;

	float speed = 250.f;
	float defense = 0.f;
	float hp = 100.f;

	// Controlled direction tracking: [[left, up], [right, down]]
	mat2 move {0.f, 0.f, 0.f, 0.f};
	bool vul = true;
	vec2 jump_velocity {0.f, -2.f}; // Initial jump velocity
	

	// Total number of jumps
	int jumps = 2;
	// Number of jumps since last time touching platform/wall
	int jumps_done = 0;

	/*
	control character animation, right now only two frame for the moving animation, 
	one is left leg in the front and the other is right leg in the front
	*/
	int frameCounter = 0;
	int keys = 0;

	float animationDelay = 100.f; // time in seconds before next frame
	float elapsedTime = 0;    // time since last frame update

	// Could make this a component, but it's only used in one place
	// Tracks whether the pick_up_key button is pressed ('e').
	bool in_pick_up_state = false;

	void set_move_up(float mv) {
		move[0][1] = mv;
	}

	void set_move_down(float mv) {
		move[1][1] = mv;
	}

	void set_move_left(float mv) {
		move[0][0] = mv;
	}

	void set_move_right(float mv) {
		move[1][0] = mv;
	}

	void reset_move() {
		move = {0.f, 0.f, 0.f, 0.f};
	}
};

struct Camera
{

};

// Jumping in topdown mode
struct Jump
{
	float jump_length_ms = 750.f;
	float timer_ms = jump_length_ms;
	vec2 jump_height = {0.f, -100.f};
	vec2 speed_boost = {1.5f, 1.5f}; // Temporary speed while jumping
	vec2 z_scale {.7f, .7f}; // Scale at the top of the jump

	vec2 getPos() const {
		return (-pow(2.f * timer_ms/jump_length_ms-1.f, 2.f) + 1.f) * jump_height;
	}
};

// For entities that can be hit.
struct Hit
{
	float hp = 1.f;
};

struct Boss {
	float timer_ms = 6000.f; 
	bool isUsingAbility = false;
	BOSS_STATE state = BOSS_STATE::WALKING;
};

// Damage component of attack
struct Attack
{
	float damage = 1.f;
	// Determines whether attack is an area of effect attack
	bool aoe = false;
};

// Knockback entity
struct KnockBack
{
	vec2 direction;
	float timer_ms = 100.f;
	float power = 1.f;
};

struct Ai {
	float timer_ms = 3000.f;
};

struct LightUp
{
	float timer_ms = 1500.f;
};

// Enemies have a hard shell
struct HardShell
{
	bool isProjectile = false;
};

// Coins and Player have a soft shell
struct SoftShell
{

};
// Slime can spawn pebbles to player
struct Slime {
	bool isBigSlime = true; // bool to indicate whether the slime is big or small
};

// Enemy Flying Unit, can use range attack
struct EnemyFly {

};

enum class DOORTYPE {
	OPEN,
	CLOSED
};

enum class PLATFORMTYPE {
	LARGE,
	MEDIUM,
	TALL,
	SHORT,
	GRASSLARGE,
	GRASSTALL,
	GRASSSHORT,
};

enum class 
	PLATFORMSPIKES {
	SHORT,
	TALL,
	SHORTGRASS,
	TALLGRASS
};

enum class LEVERSTATE {
	PULLED,
	UNTOUCHED
};

enum class PLATFORMFLOOR {
	STRAIGHT,
	LONG,
	SLOPED,
	STAIRS,
	GENERIC,
	GENERICGRASS,
	STAIRSGRASS,
};

enum class CHESTSTATE {
	OPEN,
	CLOSED
};

enum class ITEM_TYPE {
	SWORD,
	ASSAULT_RIFLE,
	LASER_GUN,
	PISTOL,
	SHOTGUN,
	FEATHER,
	STONE,
	GOLDEN_EGG
};

struct Door
{
	DOORTYPE type = DOORTYPE::CLOSED;
	ROOM to; // Which room this does connect to?
};

struct Chest
{
	CHESTSTATE state = CHESTSTATE::CLOSED;
	/*
	control chest animation, right now only two frame for the moving animation, from untouched to pulled states
	*/
	int frameCounter = 0;

	float animationDelay = 100.f; // time in seconds before next frame
	float elapsedTime = 0;    // time since last frame update
};

struct Lever
{
	LEVERSTATE state = LEVERSTATE::UNTOUCHED;
	Entity doorEntity; // Target door
	/*
	control lever animation, right now only two frame for the moving animation, from untouched to pulled states
	*/
	int frameCounter = 0;
	float animationDelay = 100.f; // time in seconds before next frame
	float elapsedTime = 0;    // time since last frame update
};

// All data relevant to the room
struct Room {
	std::string name;
	vec2 position {.0f, .0f}; // start position
	vec2 size; // room size

	// Track if it is a topDownLevel
	bool topdown = true;
	ROOM room_type;
};

// All data relevant to the platform floor
struct PlatformFloor {
	PLATFORMFLOOR type = PLATFORMFLOOR::STRAIGHT;
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0.f, 0.f };
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
	float angle = 0.f;

	void zero() {
		position = {0.f, 0.f};
		velocity = {0.f, 0.f};
		angle = 0.f;
	}
};

// Gravitating behaviour towards another entity
// Acceleration/force decreases with distance.
struct Gravity {
	Entity center;
	float g{0.f};
};

struct LeverDoorLink {
	Entity door;
};

// Gravitating behaviour towards another entity
// Acceleration/force increases with distance.
struct Tension {
	Entity center;
	float k{0.f};
	float l{0.f};
};

// Struct to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	Collision(Entity& other_entity) { this->other_entity = other_entity; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

struct MeshPlatform
{

};

// Sets the brightness of the screen
struct ScreenState
{
	float screen_darken_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying player
struct DeathTimer
{
	float timer_ms = 5000.f;
};

struct Expiry
{
	float timer_ms = 0.f; // Immediately call
};

struct Timeout
{
	float timer_ms = 5000.f;
	std::function<void()> callback;
};

// Linear tween
struct Bounce
{
	vec2 start = {0.f, 0.f};
	vec2 end = {0.f, 0.f};
	float duration_ms = 2000.f; // duration one way

	vec2 get_velocity(float since_start_ms) const {
		float whole;
		float p = std::modf(since_start_ms / duration_ms, &whole);

		if (static_cast<int>(whole) % 2)
			return (start - end) / duration_ms * 1000.f;
		return (end - start) / duration_ms * 1000.f;
	}
};

struct LifeBar
{
	float maxHP;
	float currHP;
};

struct HealthPotion
{

};


struct LifeBarContainer
{

};

struct Glow
{
    vec4 color;
};

struct Physics
{
	vec2 gravity = {0.f, 1500.f}; // Gravitational acceleration not force.
	float mass = 100.f;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & player.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<TexturedVertex> texture_vertices;
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct Text {
	std::string message;
	vec2 position = { 0.f, 0.f };
	float scale = 1.0f;
	vec3 color = { 0.f, 0.f, 0.f };
	int bit_shift = 1.f;
};

// Entity for collision detection for collectable items/clutter.
struct Collectable
{
	// If false, the entity will be collected on collision
	// or example, if 'E' is pressed, entity will be collected
	bool require_key_press = false;

	// If true, the entity will be stored into inventory when collected.
	// Leave default value as false.
	bool inventoryCollectable = false;
};

// Entity for collision detection for breakable items.
struct Breakable
{
	float hp = 1.f;
};

// Entity for collision detection between entities. Used in trying to prevent 'Collidable' objects
// from overlapping.
struct Collidable
{

};

// Entity for any entity that can deal damage via Collision
struct CollisionDamageDealer
{
	float damage = 1.f;
};


// cow entity that follows player
struct Cow
{
	bool follow = false;
};


// Entity for associating a coin value to an entity
struct CoinValue
{
	int value = 1;
};

struct Key
{

};

struct HelpButton
{
	bool show = false;
};

struct ShopButton
{

};

enum class BUTTONTYPE {
	HELP,
	SHOP,
	SHOPITEM,
};

struct Button
{
	BUTTONTYPE type;

};

struct Shop
{
};

struct ShopItem
{
	ITEM_TYPE type;
	std::string name;
	int price;
};

enum class TUTORIALTYPE {
	PLATFORM,
	TOPDOWN
};

struct TutorialMessage
{
	TUTORIALTYPE type = TUTORIALTYPE::TOPDOWN;
	bool show = false;
};

// Component for inventory
// For entities that are inventory slots.
struct InventorySlotUI {
	bool selected = false;
	Entity item;
};


// Stats that items can give
struct ItemStat {
	int attack = 3;
	int speed = 0;
	int health = 0;
	int defense = 0;
};

// Buff component that actually is involved during stat calculations
struct Buff {
	int attack = 0;
	int speed = 0;
	int health = 0;
	int defense = 0;
};

// Data for items
struct Item {
	int quantity{0};
	ITEM_TYPE type;
	ItemStat stats;
};

struct Inventory
{
	// Represents which slot is currently selected (default at index 0)
	int selectedIndex = 0;
	// array of entities to represent item entities
	std::vector<Entity> items;

	// array of ints that represents empty slot indexes
	std::vector<int> emptySlots;

	// array of entities to represent inventory slots
	std::vector<Entity> slots;
	// selection border (todo: move elsewhere, just here for convenience)
	Entity selectionBorder;
};

struct CoinCounter
{

};

struct CoinSymbol
{

};

struct KeyCounter
{

};

struct KeySymbol
{

};

struct PlayerSymbol {

};

struct CowBossSymbol {

};


struct WeaponContainer
{

};


// Parallax move in the oposite direction of player movement
struct Parallax
{
	float parallaxSpeed;
	bool reflected = false;
};


struct TutorialBackground
{

};





/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example 
 are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	PLAYER_FRONT,
	PLAYER_BACK,
	PLAYER_LEFT,
	PLAYER_RIGHT,
	PLAYER_FRONT_LEFT,
	PLAYER_FRONT_RIGHT,
	PLAYER_BACK_LEFT,
	PLAYER_BACK_RIGHT,
	COIN,
	ENEMY,
	WALL,
	FLOOR,
	DOOR_CLOSED,
	DOOR_OPEN,
	BARREL,
	CURSOR,
	SPIKE,
	PLATFORM_ROOM,
	PLATFORM_FLOOR_STRAIGHT,
	PLATFORM_FLOOR_SLOPED,
	PLATFORM_FLOOR_STRAIGHT_LONG,
	PLATFORM_STAIRS,
	LEVER_STAND,
	UNTOUCHED_LEVER,
	PULLED_LEVER,
	CLOSED_PLATFORM_DOOR,
	OPEN_PLATFORM_DOOR,
	HELP_BUTTON,
	TUTORIAL,
	TUTORIAL_PlATFORM,
	PLAYER_FRONT_MOVEL,
	PLAYER_FRONT_MOVER,
	PLAYER_BACK_MOVEL,
	PLAYER_BACK_MOVER,
	PLAYER_LEFT_MOVEL,
	PLAYER_LEFT_MOVER,
	PLAYER_RIGHT_MOVEL,
	PLAYER_RIGHT_MOVER,
	PLAYER_FRONT_LEFT_MOVEL,
	PLAYER_FRONT_LEFT_MOVER,
	PLAYER_FRONT_RIGHT_MOVEL,
	PLAYER_FRONT_RIGHT_MOVER,
	PLAYER_BACK_LEFT_MOVEL,
	PLAYER_BACK_LEFT_MOVER,
	PLAYER_BACK_RIGHT_MOVEL,
	PLAYER_BACK_RIGHT_MOVER,
	PLAYER_FAINT,
	INVENTORY_SLOT,
	INVENTORY_SLOT_SELECTOR,
	SWORD,
	PLATFORM_WALL,
	PLATFORM_WALL_BRICK,
	TORCH,
	WINDOW,
	LEVER_TRANSITION,
	PLATFORM_FLOOR_GENERIC,
	LEVER_STAND_LARGE,
	LEVER_STAND_MEDIUM,
	LEVER_STAND_TALL,
	SHIELD,
	VASE,
	CHEST_CLOSED,
	CHEST_OPEN,
	COW,
	ENEMY_SLIME,
	ENEMY_FLY,
	SWORD_SLASH,
	PLATFORM_SPIKES_SHORT,
	PLATFORM_SPIKES_TALL,
	ASSAULT_RIFLE,
	LASER_GUN,
	PISTOL,
	SHOTGUN,
	LASER_BEAM,
	LIFE_BAR_EMPTY,
	LIFE_BAR,
	KEY,
	BOX,
	PLATFORM_BARREL,
	SKY,
	MOUNTAIN,
	GRASS,
	SOIL,
	GRASS_SPIKES_TALL,
	GRASS_SPIKES_SHORT,
	GENERIC_FLOOR_GRASS,
	STAIRS_GRASS,
	LEVER_STAND_LARGE_GRASS,
	LEVER_STAND_SHORT_GRASS,
	HOSTILE_COW,
	SHOP_BUTTON,
	SHOP_BACKGROUND,
	FEATHER,
	STONE,
	HEALTH_POTION,
	ROSE_BUSH,
	GOLDEN_EGG,
	TEXTURE_COUNT
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
    COLOURED,
    CPU_PARTICLE,
    PLAYER,
    TEXTURED,
    WATER ,
	TEXT,
    UI,
	PARTICLE,
	REFLECTION,
    // Keep this as the last item to automatically assign cardinality
    EFFECT_COUNT,
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	PLAYER,
	SPRITE,
	PEBBLE,
	DEBUG_LINE,
	SCREEN_TRIANGLE,
	TEXT,
    UI,
	PLATFORM,
	KEY,
    // Keep this as the last item to automatically assign cardinality
	GEOMETRY_COUNT
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	int layer = 0; // Default layer = 0, Bigger layer value overlaps smaller layer value requests.
};

struct Emitter
{
	bool firing = false;
	RenderRequest renderRequest;
	float fire_delay = 300.0f;
	float next_spawn = 0.f;
	Motion motion;
};


