#pragma once

#include <array>
#include <utility>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"
#include "particle_system.hpp"

// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
};

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  // std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER, mesh_path("player.obj"))
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLATFORM, mesh_path("rotated_lever_platform.obj")),
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::KEY, mesh_path("key_test.obj")),
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("player_front.png"),
			textures_path("player_back.png"),
			textures_path("player_left.png"),
			textures_path("player_right.png"),
			textures_path("player_front_left.png"),
			textures_path("player_front_right.png"),
			textures_path("player_back_left.png"),
			textures_path("player_back_right.png"),
			textures_path("coin.png"),
			textures_path("enemy.png"),
			textures_path("wall.png"),
			textures_path("floor.png"),
			textures_path("door_closed.png"),
			textures_path("door_open.png"),
			textures_path("barrel.png"),
			textures_path("cursor.png"),
			textures_path("spike.png"),
			textures_path("platform_room.png"),
			textures_path("platform_floor_straight.png"),
			textures_path("platform_floor_sloped.png"),
			textures_path("platform_floor_straight_long.png"),
			textures_path("platform_stairs.png"),
			textures_path("lever_stand.png"),
			textures_path("untouched_lever.png"),
			textures_path("pulled_lever.png"),
			textures_path("closed_platform_door.png"),
			textures_path("open_platform_door.png"),
			textures_path("help.png"),
			textures_path("tutorial_image.png"),
			textures_path("tutorial_image2.png"),
			textures_path("player_front_movel.png"),
			textures_path("player_front_mover.png"),
			textures_path("player_back_movel.png"),
			textures_path("player_back_mover.png"),
			textures_path("player_left_movel.png"),
			textures_path("player_left_mover.png"),
			textures_path("player_right_movel.png"),
			textures_path("player_right_mover.png"),
			textures_path("player_front_left_movel.png"),
			textures_path("player_front_left_mover.png"),
			textures_path("player_front_right_movel.png"),
			textures_path("player_front_right_mover.png"),
			textures_path("player_back_left_movel.png"),
			textures_path("player_back_left_mover.png"),
			textures_path("player_back_right_movel.png"),
			textures_path("player_back_right_mover.png"),
			textures_path("player_faint.png"),
			textures_path("inventory/inventoryslot.png"),
			textures_path("inventory/inventoryselector.png"),
			textures_path("items/sword.png"),
			textures_path("platform_wall_blank.png"),
			textures_path("platform_wall_brick.png"),
			textures_path("wall_torch.png"),
			textures_path("window.png"),
			textures_path("lever_transition.png"),
			textures_path("platform_floor_generic.png"),
			textures_path("lever_stand_large.png"),
			textures_path("lever_stand_medium.png"),
			textures_path("lever_stand_tall.png"),
			textures_path("shield.png"),
			textures_path("vase.png"),
			textures_path("chest_closed.png"),
			textures_path("chest_opened.png"),
			textures_path("cow.png"),
			textures_path("enemy_slime.png"),
			textures_path("enemy_fly.png"),
			textures_path("attacks/sword_slash_trans.png"),
			textures_path("platform_spikes_short.png"),
			textures_path("platform_spikes_tall.png"),
			textures_path("items/assault_rifle.png"),
			textures_path("items/laser_gun.png"),
			textures_path("items/pistol.png"),
			textures_path("items/shotgun.png"),
			textures_path("attacks/laser_beam.png"),
			textures_path("empty_life_bar.png"),
			textures_path("life_bar_fill.png"),
			textures_path("key.png"),
			textures_path("box.png"),
			textures_path("barrel_platformer.png"),
			textures_path("sky_layer.png"),
			textures_path("mountain_layer.png"),
			textures_path("grass_layer.png"),
			textures_path("soil.png"),
			textures_path("platform_spikes_tall_grass.png"),
			textures_path("platform_spikes_short_grass.png"),
			textures_path("platform_floor_generic_grass.png"),
			textures_path("platform_stairs_grass.png"),
			textures_path("lever_stand_large_grass.png"),
			textures_path("lever_stand_grass.png"),
			textures_path("hostile_cow.png"),
			textures_path("shop_icon.png"),
			textures_path("shop_background.png"),
			textures_path("items/feather.png"),
			textures_path("items/stone.png"),
			textures_path("health_potion_frame5.png"),
			textures_path("rose_bush.png"),
			textures_path("items/golden_egg.png")
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"), // COLOURED
		shader_path("pebble"), // CPU_PARTICLE
		shader_path("player"), // PLAYER
		shader_path("textured"), // TEXTURED
		shader_path("water"), // WATER
    	shader_path("text"), // TEXT
        shader_path("ui"), // UI
		shader_path("particle"), // PARTICLE
		shader_path("reflection"), // REFLECTION
    };

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;
	// This saves the GLuint for characters internally.
	std::map<GLchar, Character> Characters;
	GLuint text_vao;
	GLuint text_vbo;

	GLuint vao; // Default one that is barely used. Just here to prevent error.
	Transform projection_transform;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeFonts();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

	std::array<GLuint, effect_count> effects;

	ScreenState screen_state;
private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawText(Entity entity);
	static inline void drawParticles(Entity entity, const mat3& transform, const mat3& projection);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	void initializeProjectionTransform();

};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
