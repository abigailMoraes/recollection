#Recollection

## Development Plan Alignment
Milestone 4 aligns with what we wanted to complete.

The following features have been implemented from our development plan for Milestone 4:

- Parallax scrolling backgrounds
- 5 different levels
- Cow level
- Mesh collision (against keys)

## User Testing & Changes Based on Feedbacks

| Feedback                                                               | Changes                                                                            |
|------------------------------------------------------------------------|------------------------------------------------------------------------------------|
| The topdown level is too difficult.                                    | The enemy's spawn rates are scaled down. Weapons are more accessible through shop. |
| There is no way to spend money.                                        | Created a shop menu to buy items.                                                  |
| The barrels look amorphous and are unrecognizable.                     | The barrel texture was replaced.                                                   |
| The game is about shooting enemies, and has nothing else going on.     | The platform room was revamped to make it more challenging.                        |
| The coins look like clutters.                                          | The coins were made smaller. They are also attracted to the player.                |
| Collision platforms felt janky.                                        | Fixed the issues by completely rewriting the player-platform collision handling.   |
| Gravity was inconsistent.                                              | Gravity is rewritten to be physics-based.                                          |
| The enemy bullets are too small to notice, and too difficult to dodge. | The bullets are made larger, and destructible.                                     |


## Features

#### A post-processing effect, which includes gradual color distortion (sepia).
The effect is triggered upon player death.
#### A CRT screen vignette effect using fragment shader
This effect is always visible.
#### Attack towards mouse cursor by clicking/dragging
- Default Weapon: Rapid clicking for rapid firing. Keeping the left mouse button down fires pebbles at a steady rate.
- Sword: Swing sword to create a strong aoe attack.
- Attacks knockback enemies.
#### Sound effects + New BGM
New sound effects for attack, jump, death, and for picking up coin.
The new BGM plays from the start.
#### Pseudo-Jump interpolation
Right click to jump, and it works independently of the Motion component. Jump over spikes.
This is not a real physical jump. Only a visual effect that will later be converted
to a "dodge" move with an invulnerability period.
#### 4 directional player
Use WASD to move player up, down, left, right.
#### Collectable and door collisions
Move towards a coin for coin collection, move to door for new room.
#### Basic Collision handling
Separates colliding entities to prevent them from clipping into each other.
#### Particle Effect
When game starts, a particle effect is triggered.
#### Cheat Codes for testing
Enable debug functions by pressing F1 first.

F2: Disable collision against objects

F3: Kills the player

F4: Kills all enemies

F5: **changed** Move the character to the second room

F6: Print player position

F7: Spawn 100 enemies around the player

F8: **added** Disable collision system

F9: **added** Spawn a cow on player

P: **added** Move the player to the boss room.

#### Inventory System
Press "E" to pick up the nearest item.
Press "J" to drop selected item.
Press 1, 2, 3, 4, 5 to switch between items.

#### Camera with inertia
The camera follows with the player.
The camera is tethered to the player with elastic tension between the player and the camera.
Uses Hooke's law for tension.

#### Money Electrostatic Force
Coins gravitate towards the player if they are within a fixed range.

#### Pause on Help
The game is paused while a help message is displayed.

#### Game Name
"Recollection" is rendered using Freetype library

#### Enemy knockback
Enemies are knocked back when attacked.

#### More Enemies!
Slimes and Flies are added.

#### Weapons
Added a new melee weapon that can be picked up with E.

### Cows in the topdown room
If you touch the cows, they will follow you.
Gather 5 of them to enter a secret level (TBA).
They are connected in tandem to the player with invisible springs.

#### Parallax background in Rosegarden room
The game starts in this room.

#### Game balance from User feedback
The second room is a lot easier with less enemies

#### Secret Cow Level
Collect 5 cows in the wild to enter the secret level.

Use the new cheat key (F9) to summon cows for testing.

#### Death effect in platform mode
The player character falls endlessly through all objects.

#### 5 different weapons
They spawn if you break barrels.

#### Shop
Press shop icon on top right corner to access shop

#### Buff items
Pick them up by pressing E, and activate them using B

## License & Credits

### Sound Effects

- zap: https://pixabay.com/sound-effects/zap-c-07-82067/
- jump: https://pixabay.com/sound-effects/cartoon-jump-6462/
- Dorian: https://samplefocus.com/samples/dorian-quiet-processed-chord-progression\
- Coin_C_02: https://pixabay.com/sound-effects/coin-c-02-102844/
- Old Church Bell: https://pixabay.com/sound-effects/old-church-bell-6298/

### Images

- platform room (platform_room.png): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform room floor (platform_floor_straight.png): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform floor sloped (platform_floor_sloped.png):  https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform floor straight (platform_floor_straight_long.png): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform floor stairs (platform_stairs.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- lever platform ("lever_stand.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- untouched lever ("untouched_lever.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- pulled lever ("pulled_lever.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- closed platform door ("closed_platform_door.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- open platform door ("open_platform_door.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform wall ("platform_wall_blank.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform wall brick ("platform_wall_brick.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform wall brick ("wall_torch.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- window ("window.png"):  https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform floor generic ("platform_floor_generic.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform lever stand large ("lever_stand_large.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform lever stand medium ("lever_stand_medium.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platform lever stand tall ("lever_stand_tall.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- shield ("shield.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- vase ("vase.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- closed chest ("chest_closed.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- open chest ("chest_opened.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- short platform spikes ("platform_spikes_short.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- tall platform spikes ("platform_spikes_tall.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- life bar ("empty_life_bar.png"): https://www.seekpng.com/ipng/u2q8y3i1t4q8t4w7_health-bar-health-bar-sprite-2d/
- life bar ("life_bar_fill.png"): https://www.seekpng.com/ipng/u2q8y3i1t4q8t4w7_health-bar-health-bar-sprite-2d/
- key symbol for key counter ("key.png"): https://www.vectorstock.com/royalty-free-vector/set-of-different-style-pixelated-keys-vector-23754314
- box ("box.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- platformer rooms barrel ("barrel_platformer.png"): https://craftpix.net/freebies/free-medieval-tileset-pixel-art-pack/
- parallax background sky ("sky_layer.png"): https://craftpix.net/freebies/free-nature-backgrounds-pixel-art/?num=1&count=138&sq=parallax%20background%20mountains&pos=3
- parallax background mountain layer ("mountain_layer.png"): https://craftpix.net/freebies/free-nature-backgrounds-pixel-art/?num=1&count=138&sq=parallax%20background%20mountains&pos=3
- parallax background grass ("grass_layer.png"): https://craftpix.net/freebies/free-nature-backgrounds-pixel-art/?num=1&count=138&sq=parallax%20background%20mountains&pos=3
- parallax background soil ("soil.png"): https://craftpix.net/freebies/free-nature-backgrounds-pixel-art/?num=1&count=138&sq=parallax%20background%20mountains&pos=3
- grass platform tall spikes ("platform_spikes_tall_grass.png"): https://craftpix.net/freebies/nature-pixel-art-environment-free-assets-pack/
- grass platform short spikes ("platform_spikes_short_grass.png"): https://craftpix.net/freebies/nature-pixel-art-environment-free-assets-pack/
- grass platform blocks ("platform_floor_generic_grass.png"): https://craftpix.net/freebies/nature-pixel-art-environment-free-assets-pack/
- grass raised platform ("platform_stairs_grass.png"): https://craftpix.net/freebies/nature-pixel-art-environment-free-assets-pack/
- grass lever stand large ("lever_stand_large_grass.png"): https://craftpix.net/freebies/nature-pixel-art-environment-free-assets-pack/
- grass lever stand ("lever_stand_grass.png"): https://craftpix.net/freebies/nature-pixel-art-environment-free-assets-pack/
- health potion ("health_potion_frame5.png"): https://dribbble.com/shots/5756455-Health-Potion
- rose bush ("rose_bush.png"):https://www.vecteezy.com/vector-art/9257744-games-concept-resizable-vector-pixel-art-flowers
