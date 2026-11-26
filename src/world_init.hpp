#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"


// These are ahrd coded to the dimensions of the entity texture
const float COIN_BB_WIDTH = 1.f * 16.f;
const float COIN_BB_HEIGHT = 1.f * 16.f;
const float ENEMY_BB_WIDTH = 4.f * 16.f;
const float ENEMY_BB_HEIGHT = 4.f * 16.f;
const float PLAYER_BB_WIDTH = 3.f * 11.f;
const float PLAYER_BB_HEIGHT = 3.f * 19.f;
const float BUTTON_BB_WIDTH = 4.f * 16.f;
const float BUTTON_BB_HEIGHT = 4.f * 16.f;
const float TUTORIAL_BB_WIDTH = 48.f * 16.f;
const float TUTORIAL_BB_HEIGHT = 38.f * 16.f;
const float COW_BOSS_MAX_HP = 10000.f;
const float PLAYER_MAX_HP = 100.f;

// For scaling sprites that are 16x16 pixels
const float _16_BY_16_SCALE = 4.f * 16.f;

const float _64_BY_64_SCALE = 4.f * 64.f;

// Inventory slot size
const float INVENTORY_SLOT_SIZE = 4.f * 16.f;

const float ROOM_WIDTH = 10000.f;
const float ROOM_HEIGHT = 10000.f;

const float ROOM_WALL_THICKNESS = 30.f;

const float ROOM_MIN_X = ROOM_WIDTH * -.5f + ROOM_WALL_THICKNESS * 2.f;
const float ROOM_MAX_X = -ROOM_MIN_X;
const float ROOM_MIN_Y = ROOM_HEIGHT * -.5f + ROOM_WALL_THICKNESS;
const float ROOM_MAX_Y = -ROOM_MIN_Y;

// the camera(s)
Entity createCamera(Entity focus);
// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// the prey
Entity createCoin(RenderSystem* renderer, vec2 position);
// the enemy
Entity createEnemy(RenderSystem* renderer, vec2 position);
// the cow
Entity createCow(RenderSystem* renderer, vec2 position);
// the cow
Entity createHostileCow(RenderSystem* renderer, vec2 position);
// the enemy fly
Entity createEnemyFly(RenderSystem* renderer, vec2 position);
// the enemy slime
Entity createEnemySlime(RenderSystem* renderer, vec2 position, bool isBigSlime);
// Create a number of enemies randomly placed around a center
void createEnemies(RenderSystem* renderer, uint number, Entity center);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// a pebble
Entity createPebble(vec2 pos, vec2 size, vec3 color);
// a wall
Entity createWall(RenderSystem* renderer, vec2 position, vec2 size);
// a door
Entity createDoor(RenderSystem* renderer, vec2 position, vec2 size, ROOM destination);
// create a simple room with 4 walls
Entity createRoom(RenderSystem* renderer, const Room&& room);
// create a shop 
Entity createShop(RenderSystem* renderer, unsigned int points);
// create shop items
Entity createShopItem(RenderSystem* renderer, TEXTURE_ASSET_ID item, vec2 pos);
// create a platform room
Entity createPlatformRoom(RenderSystem* renderer, const Room&& room);
// create a lever platform
Entity createPlatform(RenderSystem* renderer, vec2 position, vec2 size, PLATFORMTYPE type);
// create a lever
Entity createLever(RenderSystem* renderer, vec2 position, vec2 size, LEVERSTATE state, Entity door);
// create a platform door
Entity createPlatformDoor(RenderSystem* renderer, vec2 position, vec2 size, DOORTYPE type, bool leverOperated, ROOM destination, PLATFORMTYPE floorType);
// create a barrel
Entity createBarrel(RenderSystem* renderer, vec2 position);
// create a spike
Entity createSpike(RenderSystem* renderer, vec2 position);
// create platform spikes
Entity createPlatformSpikes(RenderSystem* renderer, vec2 position, vec2 size, PLATFORMSPIKES type);
// create text
Entity createText(RenderSystem* renderer, vec2 position, std::string message, float scale, vec3 color, int layer);
// create a help button
//Entity createHelpButton(RenderSystem* renderer, vec2 position);
//// create a shop button
//Entity createShopButton(RenderSystem* renderer, vec2 position);
// create a button
Entity createButton(RenderSystem* renderer, vec2 position, BUTTONTYPE type);
// create a tutorial message
Entity createTutorialMessage(RenderSystem* renderer, vec2 position, TUTORIALTYPE type);
// create an item
Entity createGroundItem(RenderSystem* renderer, vec2 position, Item itemType);
// create a  chest
Entity createChest(RenderSystem* renderer, vec2 position, vec2 size, bool open);
// create a life bar
Entity createLifeBar(RenderSystem* renderer, vec2 position, vec2 size, float maxHp, float currHp);
// create a life bar container
Entity createLifeBarContainer(RenderSystem* renderer, vec2 position, vec2 size, float hp);
// render points
Entity renderPoints(RenderSystem* renderer, vec2 pos, std::string message, float scale, vec3 color, int layer);
// render keys
Entity renderKeys(RenderSystem* renderer, vec2 pos, std::string message, float scale, vec3 color);
// render ui coin symbol
Entity renderCoinSymbol(RenderSystem* renderer, vec2 pos, vec2 size, int layer);
// render ui key symbol
Entity renderKeySymbol(RenderSystem* renderer, vec2 pos, vec2 size);
// create a vase
Entity createVase(RenderSystem* renderer, vec2 position, vec2 size);
// create a platform barrel
Entity createPlatformBarrel(RenderSystem* renderer, vec2 position, vec2 size);
// create a box
Entity createBox(RenderSystem* renderer, vec2 position, vec2 size);
// create the shop Room
Entity createRoseGardenRoom(RenderSystem* renderer, const Room&& room);
// render mountain layer for parallax
Entity renderMountain(RenderSystem* renderer, vec2 position, vec2 size, bool reflected);
// render grass layer for parallax
Entity renderGrass(RenderSystem* renderer, vec2 position, vec2 size, bool reflected);
// render soil layer for parallax
Entity renderSoil(RenderSystem* renderer, vec2 position, vec2 size);
// render sky layer for parallax
Entity renderSky(RenderSystem* renderer, vec2 position, vec2 size, bool reflected);
// render player symbol for the health bar
Entity renderPlayerSymbol(RenderSystem* renderer, vec2 pos, vec2 size);
// render cow symbol for the health bar
Entity renderCowBossSymbol(RenderSystem* renderer, vec2 pos, vec2 size);
// render health potion
Entity createHealthPotion(RenderSystem* renderer, vec2 position);
// render health potion
Entity createRoseBush(RenderSystem* renderer, vec2 position);
// render parallax tutorial
Entity renderParallaxTutorial(RenderSystem* renderer, vec2 position, vec2 size, bool reflected);

/** INVENTORY START **/
// create inventory related entities
Entity createInventory(Entity holder, int inventorySize);
Entity createInventorySlot();
Entity createSelectionBorder();

// Render an inventory onto the screen (insert renderRequests)
void renderInventory(RenderSystem* renderer, vec2 position);
void renderInventorySlot(RenderSystem* renderer, vec2 position, Entity slot, Entity item);
void renderSelectionBorder(RenderSystem* renderer, vec2 inventoryPosition, Entity selectionBorder, int index);
void renderItemInSlot(RenderSystem* renderer, vec2 position, Entity slot, Entity item);

/** INVENTORY END **/

/** ATTACK START **/

// Creates and renders a melee attack
Entity createMeleeAttack(RenderSystem* renderer, Entity attacker, vec2 direction, float dist, float damage, float duration);

// Creates a shotgun attack
Entity createShotgunAttack(RenderSystem* renderer, Entity attacker, vec2 direction, float damage);

// Creates a laser attack.
Entity createLaserAttack(RenderSystem* renderer, Entity attacker, vec2 direction, float length, float damage, float duration);

/** ATTACK END **/