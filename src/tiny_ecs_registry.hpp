#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "particle_system.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Expiry> expiry;
	ComponentContainer<Timeout> timeouts;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<Mesh*> collisionMeshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<SoftShell> softShells;
	ComponentContainer<HardShell> hardShells;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<LightUp> lightUps;
    ComponentContainer<Glow> glows;
	ComponentContainer<Physics> physics;
	ComponentContainer<Emitter> emitters;
	ComponentContainer<Hit> hits;
	ComponentContainer<Attack> attacks;
	ComponentContainer<Jump> jumps;
	ComponentContainer<Ai> ai;
	ComponentContainer<Room> rooms;
	ComponentContainer<Collectable> collectables;
	ComponentContainer<CoinValue> coinValues;
	ComponentContainer<Door> doors;
	ComponentContainer<Collidable> collidables;
	ComponentContainer<CollisionDamageDealer> collisionDamageDealers;
	ComponentContainer<Text> texts;
	ComponentContainer<Lever> levers;
	//ComponentContainer<HelpButton> helpButtons;
	ComponentContainer<TutorialMessage> tutorialMessages;
	ComponentContainer<MeshPlatform> meshPlatforms;
	ComponentContainer<Inventory> inventories;
	ComponentContainer<InventorySlotUI> inventorySlotUIs;
	ComponentContainer<ParticleSystem> particleSystems;
	ComponentContainer<Gravity> gravities;
	ComponentContainer<Tension> tensions;
	ComponentContainer<Camera> cameras;
	ComponentContainer<Key> keys;
	ComponentContainer<Item> items;
	ComponentContainer<Bounce> bounces;
	ComponentContainer<Chest> chests;
	ComponentContainer<Cow> cows;
	ComponentContainer<Cow> capturedCows;
	ComponentContainer<Slime> slimes;
	ComponentContainer<EnemyFly> enemy_fly;
	ComponentContainer<KnockBack> knockbacks;
	ComponentContainer<LifeBar> lifebars;
	ComponentContainer<LifeBarContainer> lifeBarContainers;
	ComponentContainer<CoinCounter> coinCounters;
	ComponentContainer<CoinSymbol> coinSymbols;
	ComponentContainer<KeyCounter> keyCounters;
	ComponentContainer<KeySymbol> keySymbols;
	ComponentContainer<LeverDoorLink> leverDoorLinks;
	ComponentContainer<WeaponContainer> weaponContainers;
	ComponentContainer<Parallax> parallaxes;
	ComponentContainer<Boss> boss;
	//ComponentContainer<ShopButton> shopButtons;
	ComponentContainer<Shop> shops;
	ComponentContainer<ShopItem> shopItems;
	ComponentContainer<Button> buttons;
	ComponentContainer<Buff> buffs;
	ComponentContainer<PlayerSymbol> playerSymbols;
	ComponentContainer<CowBossSymbol> cowBossSymbols;
	ComponentContainer<HealthPotion> healthPotions;
	ComponentContainer<TutorialBackground> tutorialBackgrounds;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry() {
		update_registry_list();
	}

	void update_registry_list() {
		registry_list.clear();
		registry_list.push_back(&expiry);
		registry_list.push_back(&timeouts);
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&expiry);
		registry_list.push_back(&timeouts);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&collisionMeshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&softShells);
		registry_list.push_back(&hardShells);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&physics);
		registry_list.push_back(&emitters);
		registry_list.push_back(&hits);
		registry_list.push_back(&attacks);
		registry_list.push_back(&jumps);
		registry_list.push_back(&ai);
		registry_list.push_back(&rooms);
		registry_list.push_back(&collectables);
		registry_list.push_back(&coinValues);
		registry_list.push_back(&doors);
		registry_list.push_back(&collidables);
		registry_list.push_back(&collisionDamageDealers);
		registry_list.push_back(&lightUps);
		registry_list.push_back(&glows);
		registry_list.push_back(&texts);
		registry_list.push_back(&levers);
		//registry_list.push_back(&helpButtons);
		registry_list.push_back(&tutorialMessages);
		registry_list.push_back(&meshPlatforms);
		registry_list.push_back(&inventories);
		registry_list.push_back(&inventorySlotUIs);
		registry_list.push_back(&particleSystems);
		registry_list.push_back(&gravities);
		registry_list.push_back(&tensions);
		registry_list.push_back(&cameras);
		registry_list.push_back(&keys);
		registry_list.push_back(&items);
		registry_list.push_back(&bounces);
		registry_list.push_back(&chests);
		registry_list.push_back(&cows);
		registry_list.push_back(&capturedCows);
		registry_list.push_back(&slimes);
		registry_list.push_back(&enemy_fly);
		registry_list.push_back(&knockbacks);
		registry_list.push_back(&lifebars);
		registry_list.push_back(&lifeBarContainers);
		registry_list.push_back(&coinCounters);
		registry_list.push_back(&coinSymbols);
		registry_list.push_back(&keyCounters);
		registry_list.push_back(&keySymbols);
		registry_list.push_back(&leverDoorLinks);
		registry_list.push_back(&weaponContainers);
		registry_list.push_back(&parallaxes);
		registry_list.push_back(&boss);
		//registry_list.push_back(&shopButtons);
		registry_list.push_back(&shops);
		registry_list.push_back(&shopItems);
		registry_list.push_back(&buttons);
		registry_list.push_back(&buffs);
		registry_list.push_back(&playerSymbols);
		registry_list.push_back(&cowBossSymbols);
		registry_list.push_back(&healthPotions);
		registry_list.push_back(&tutorialBackgrounds);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
		printf("clearing components");
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}

	void copy_entity(Entity e, ECSRegistry& target_registry) {
		for (size_t i=0; i < registry_list.size(); i++) {
			if (registry_list[i]->has(e))
				target_registry.registry_list[i]->insert_ptr(e, registry_list[i]->get_ptr(e));
		}
	}

	void copy_global_entities(ECSRegistry& target_registry) {
		for (auto e: screenStates.entities)
			copy_entity(e, target_registry);
		for (auto e: cameras.entities)
			copy_entity(e, target_registry);
		for (auto e: buttons.entities)
			copy_entity(e, target_registry);
		for (auto e: inventorySlotUIs.entities)
			copy_entity(e, target_registry);

		for (auto e: players.entities) {
			copy_entity(e, target_registry);
			if (!inventories.has(e)) continue;

			Inventory& inventory = inventories.get(e);
			for (auto i: inventory.items)
				copy_entity(i, target_registry);
			copy_entity(inventory.selectionBorder, target_registry);
		}

		target_registry.rooms = std::move(rooms);
	}
};

extern ECSRegistry registry;