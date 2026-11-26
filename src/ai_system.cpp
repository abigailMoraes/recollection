// internal
#include "ai_system.hpp"
#include "world_system.hpp"
#include "world_init.hpp"

#include "physics_system.hpp"
#include "particle_system.hpp"

const size_t ENEMY_DELAY_BASE_MS = 200.f;    // time between each cow spawns
const float SPEED_UP_PREP = 1000.f;      // wait time before boss speeds up
const float NEXT_RANGE_ATTACK = 400.f;    // time between each range attack
const float MAX_DISTANCE_TO_PLAYER = 1000.f; // max distance to player before boss starts using range attack
const float MIN_DISTANCE_TO_PALYER = 200.f; // min distance to player before boss starts using speed up

// const for boss abilities
const int SPAWNING_COW = 0;
const int SPEEDING = 1;
const int RANGE_ATTACKING = 2;

// Create the AI system
AISystem::AISystem()
{
    // Seeding rng with random device
    rng = std::default_random_engine(std::random_device()());

}

void AISystem::init(WorldSystem* world_system)
{
	this->world_system = world_system;
}

int AISystem::chooseNextAbility(Boss& boss, Motion& boss_motion, Player& player, Motion& player_motion)
{
    /* Choose ability based on the distance to the player
     * If the player is close, it will have a higher chance of using the speed up ability
     * If the player is far, it will have a higher chance of using the range attack ability
     * The closer the player is from the midpoint, the higher chance the boss will spawn cows
    */

    float distance = glm::distance(boss_motion.position, player_motion.position);
    printf("Distance to player: %f\n", distance);

        // Calculate the normalized distance 
        float normalizedDistance = (distance - MIN_DISTANCE_TO_PALYER) / (MAX_DISTANCE_TO_PLAYER - MIN_DISTANCE_TO_PALYER);
        if (normalizedDistance < 0.f) normalizedDistance = 0.f; 
        if (normalizedDistance > 1.f) normalizedDistance = 1.f;

        // Calculate the probabilities of speed up and range attack
        float speedUpProbability = 1.0f - normalizedDistance; // Highest if player is at min_distance, or smaller than this value
        float rangeAttackProbability = normalizedDistance; // Highest if player is at max_distance, or larger than this value

        // Calculate the probability of spawning cows
        float midPoint = (MIN_DISTANCE_TO_PALYER + MAX_DISTANCE_TO_PLAYER) / 2.0f;
        float distanceFromMidpoint = std::abs(distance - midPoint);
        float normalizedDistanceFromMidpoint = distanceFromMidpoint / (midPoint - MIN_DISTANCE_TO_PALYER);
        if (normalizedDistanceFromMidpoint >  1.f) normalizedDistanceFromMidpoint = 1.f;
        float spawnCowProbability = 1.0f - normalizedDistanceFromMidpoint; // Highest at midpoint, decreases away from midpoint

        // Ensure total probability is 1
        float totalProbability = spawnCowProbability + rangeAttackProbability + speedUpProbability;
        spawnCowProbability /= totalProbability;
        rangeAttackProbability /= totalProbability;
        speedUpProbability /= totalProbability;

        printf("spawnCowProbability: %f\n", spawnCowProbability);
        printf("rangeAttackProbability: %f\n", rangeAttackProbability);
        printf("speedUpProbability: %f\n", speedUpProbability);


        // Randomly choose an ability based on the probabilities
        float randValue = uniform_dist(rng);
        if (randValue < spawnCowProbability) {
            return SPAWNING_COW;
        }
        else if (randValue < spawnCowProbability + speedUpProbability) {
            return SPEEDING;
        }
        else {
            return RANGE_ATTACKING;
        }
}

void AISystem::step(float elapsed_ms)
{
	// perform boss logic, only do it if there is a boss
	if (registry.boss.size() > 0) {
		auto& boss_component = registry.boss.entities.back();
        Boss& boss = registry.boss.get(boss_component);
		Motion& boss_motion = registry.motions.get(boss_component);

		auto& player_component = registry.players.entities.back();
        Player& player = registry.players.get(player_component);
		Motion& player_motion = registry.motions.get(player_component);

        // Update the boss timer
        boss.timer_ms -= elapsed_ms;
        if (boss.timer_ms <= 0) {
            if (boss.isUsingAbility) {
                // Ability ends, go to cooldown and switch to WALKING state
                boss.timer_ms = ABILITY_COOLDOWN; 
                boss.state = BOSS_STATE::WALKING;
                boss.isUsingAbility = false;
            }
            else {

                int next_ability = chooseNextAbility(boss, boss_motion, player, player_motion);

                switch ( next_ability ) {
                case SPAWNING_COW:
                    boss.state = BOSS_STATE::SPAWNING_COW;
                    boss.timer_ms = COW_SPAWNING_DURATION;

                    next_enemy_spawn = ENEMY_DELAY_BASE_MS + uniform_dist(rng) * (ENEMY_DELAY_BASE_MS / 2.f);
                    Mix_PlayChannel(-1, world_system->boss_ability1_sound, 0);
					break;
                case SPEEDING: 
					boss.state = BOSS_STATE::SPEEDING;  
                    boss.timer_ms = SPEEDING_DURATION;

                    speed_up_prep = SPEED_UP_PREP;
                    Mix_PlayChannel(-1, world_system->boss_ability2_sound, 0);
                    break;
                case RANGE_ATTACKING:
                    boss.state = BOSS_STATE::RANGE_ATTACKING;
                    boss.timer_ms = RANGE_ATTACKING_DURATION;

                    next_range_attack = NEXT_RANGE_ATTACK;
                    Mix_PlayChannel(-1, world_system->boss_ability3_sound, 0);
                    break;
                default:
                    boss.state = BOSS_STATE::WALKING;
                    boss.timer_ms = ABILITY_COOLDOWN;

                    break;
                }
                boss.isUsingAbility = true;
                boss_motion.velocity = { 0.f,0.f };

            }
        }

        // Implement ability functionality
        switch (boss.state) {
        case BOSS_STATE::SPAWNING_COW:
            // printf("Boss is spawning cows");
            // spawn cows randomly around the boss
            next_enemy_spawn -= elapsed_ms;
            // printf("next_enemy_spawn timer: %f\n", next_enemy_spawn);
            if (next_enemy_spawn <= 0) {
                // Reset timer
                next_enemy_spawn = ENEMY_DELAY_BASE_MS + uniform_dist(rng) * (ENEMY_DELAY_BASE_MS / 2.f);
                // Create cow
                Entity entity;

                entity = createHostileCow(renderer, { 0.f,0.f });
                // Setting random initial position and constant velocity
                Motion& motion = registry.motions.get(entity);

                motion.position = boss_motion.position + vec2{ uniform_dist(rng) * 500.f - 250.f, uniform_dist(rng) * 500.f - 250.f};
                registry.ai.emplace(entity);
                registry.hits.insert(entity, Hit{ 5.f });

                Mix_PlayChannel(-1, world_system->drop_item_sound, 0);
            }

            break;
        case BOSS_STATE::SPEEDING:  
            // significantly increase boss speed

            speed_up_prep -= elapsed_ms;
            if (speed_up_prep <= 0) {
                speed_up_prep = SPEEDING_DURATION;  // just some random big numeber to make if statement only run once
                // Move towards player
                boss_motion.velocity = normalize(player_motion.position - boss_motion.position) * 800.f;

			}

            break;
        case BOSS_STATE::RANGE_ATTACKING:

            next_range_attack -= elapsed_ms;
            if (next_range_attack <= 0) {

                float coneSpread = 0.3f;

                // Create 10 pebbles in a cone
                for (int i = 0; i < 10; ++i) {
                    // Calculate the rotated direction within the cone
                    float angleOffset = (i - 2) * coneSpread;
                    vec2 direction_to_player = player_motion.position - boss_motion.position;
                    direction_to_player = normalize(direction_to_player); // Normalize the direction vector

                    float cosTheta = std::cos(angleOffset);
                    float sinTheta = std::sin(angleOffset);


                    vec2 rotatedDirection = {
                            direction_to_player.x * cosTheta - direction_to_player.y * sinTheta,
                            direction_to_player.x * sinTheta + direction_to_player.y * cosTheta
                    };

                    // Create projectile
                    Entity projectile = createPebble(boss_motion.position, { 20.f, 20.f }, vec3{ 0.5f, 0.f, 0.f });

                    // Set motion of pebble to come out with a cone-like shapeas
                    Motion& motion = registry.motions.get(projectile);
                    float projectile_speed = 300.f;
                    motion.velocity = rotatedDirection * projectile_speed;

                    // Register the projectile as an attack
                    registry.hardShells.insert(projectile, HardShell{ true });
                    registry.expiry.emplace(projectile, Expiry{ 5000.f });
					registry.hits.insert(projectile, Hit{ 1.f });
                    ///////// 

                }

                next_range_attack = NEXT_RANGE_ATTACK;
            }
            break;
        default:
            break;
        }
	}
}