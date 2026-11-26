// internal
#include "render_system.hpp"
#include <SDL.h>

#include "tiny_ecs_registry.hpp"


static bool compareEntityLayers(const Entity& entity1, const Entity& entity2) {
	assert(registry.renderRequests.has(entity1));
	assert(registry.renderRequests.has(entity2));

	const RenderRequest& render_request1 = registry.renderRequests.get(entity1);
	const RenderRequest& render_request2 = registry.renderRequests.get(entity2);

	return render_request1.layer < render_request2.layer;
}

void RenderSystem::drawTexturedMesh(Entity entity,
	const mat3& projection)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	glBindVertexArray(vao);
	gl_has_errors();

	Motion& motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	// We don't generally rotate entities in top down and side scroller.
	// Rotate attacks
	if (registry.attacks.has(entity)) {
		transform.rotate(motion.angle);
	}

	transform.scale(motion.scale);

	assert(registry.renderRequests.has(entity));
	auto& render_request = registry.renderRequests.get(entity);

	const auto used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const auto program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();


	if (registry.players.has(entity)) {  // Check if the entity is a player.
		Player& player = registry.players.get(entity);

		// Determine the texture based on direction.
		TEXTURE_ASSET_ID player_texture;
		if (player.state == PLAYER_STATE::DEAD) {
			player_texture = TEXTURE_ASSET_ID::PLAYER_FAINT;
		}
		else {

			switch (player.direction) {
			case DIRECTION::BACK:
				player_texture = (player.state == PLAYER_STATE::JUMPING) ? TEXTURE_ASSET_ID::PLAYER_BACK_MOVEL :
					(player.state == PLAYER_STATE::IDLE) ? TEXTURE_ASSET_ID::PLAYER_BACK :
					(player.frameCounter == 0) ? TEXTURE_ASSET_ID::PLAYER_BACK_MOVEL : TEXTURE_ASSET_ID::PLAYER_BACK_MOVER;
				break;
			case DIRECTION::FRONT:
				player_texture = (player.state == PLAYER_STATE::JUMPING) ? TEXTURE_ASSET_ID::PLAYER_FRONT_MOVEL :
					(player.state == PLAYER_STATE::IDLE) ? TEXTURE_ASSET_ID::PLAYER_FRONT :
					(player.frameCounter == 0) ? TEXTURE_ASSET_ID::PLAYER_FRONT_MOVEL : TEXTURE_ASSET_ID::PLAYER_FRONT_MOVER;
				break;
			case DIRECTION::LEFT:
				player_texture = (player.state == PLAYER_STATE::JUMPING) ? TEXTURE_ASSET_ID::PLAYER_LEFT_MOVEL :
					(player.state == PLAYER_STATE::IDLE) ? TEXTURE_ASSET_ID::PLAYER_LEFT :
					(player.frameCounter == 0) ? TEXTURE_ASSET_ID::PLAYER_LEFT_MOVEL : TEXTURE_ASSET_ID::PLAYER_LEFT_MOVER;
				break;
			case DIRECTION::RIGHT:
				player_texture = (player.state == PLAYER_STATE::JUMPING) ? TEXTURE_ASSET_ID::PLAYER_RIGHT_MOVEL :
					(player.state == PLAYER_STATE::IDLE) ? TEXTURE_ASSET_ID::PLAYER_RIGHT :
					(player.frameCounter == 0) ? TEXTURE_ASSET_ID::PLAYER_RIGHT_MOVEL : TEXTURE_ASSET_ID::PLAYER_RIGHT_MOVER;
				break;
			case DIRECTION::FRONT_LEFT:
				player_texture = (player.state == PLAYER_STATE::JUMPING) ? TEXTURE_ASSET_ID::PLAYER_FRONT_LEFT_MOVEL :
					(player.state == PLAYER_STATE::IDLE) ? TEXTURE_ASSET_ID::PLAYER_FRONT_LEFT :
					(player.frameCounter == 0) ? TEXTURE_ASSET_ID::PLAYER_FRONT_LEFT_MOVEL : TEXTURE_ASSET_ID::PLAYER_FRONT_LEFT_MOVER;
				break;
			case DIRECTION::FRONT_RIGHT:
				player_texture = (player.state == PLAYER_STATE::JUMPING) ? TEXTURE_ASSET_ID::PLAYER_FRONT_RIGHT_MOVEL :
					(player.state == PLAYER_STATE::IDLE) ? TEXTURE_ASSET_ID::PLAYER_FRONT_RIGHT :
					(player.frameCounter == 0) ? TEXTURE_ASSET_ID::PLAYER_FRONT_RIGHT_MOVEL : TEXTURE_ASSET_ID::PLAYER_FRONT_RIGHT_MOVER;
				break;
			case DIRECTION::BACK_LEFT:
				player_texture = (player.state == PLAYER_STATE::JUMPING) ? TEXTURE_ASSET_ID::PLAYER_BACK_LEFT_MOVEL :
					(player.state == PLAYER_STATE::IDLE) ? TEXTURE_ASSET_ID::PLAYER_BACK_LEFT :
					(player.frameCounter == 0) ? TEXTURE_ASSET_ID::PLAYER_BACK_LEFT_MOVEL : TEXTURE_ASSET_ID::PLAYER_BACK_LEFT_MOVER;
				break;
			case DIRECTION::BACK_RIGHT:
				player_texture = (player.state == PLAYER_STATE::JUMPING) ? TEXTURE_ASSET_ID::PLAYER_BACK_RIGHT_MOVEL :
					(player.state == PLAYER_STATE::IDLE) ? TEXTURE_ASSET_ID::PLAYER_BACK_RIGHT :
					(player.frameCounter == 0) ? TEXTURE_ASSET_ID::PLAYER_BACK_RIGHT_MOVEL : TEXTURE_ASSET_ID::PLAYER_BACK_RIGHT_MOVER;
				break;
			default:
				player_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
			}
		}

		if (player.elapsedTime >= player.animationDelay) {
			player.frameCounter = player.frameCounter == 0 ? 1 : 0;
			player.elapsedTime = 0.0f; // reset the timer
		}

		// Update the render request for the player to use the determined texture.
		render_request.used_texture = player_texture;
	}
	else if (registry.levers.has(entity)) {
		TEXTURE_ASSET_ID lever_texture;
		Lever& lever = registry.levers.get(entity);
		if (lever.state == LEVERSTATE::PULLED) {
			lever_texture = lever.frameCounter == 0 ? TEXTURE_ASSET_ID::PULLED_LEVER : TEXTURE_ASSET_ID::LEVER_TRANSITION;
		}
		else {
			lever_texture = TEXTURE_ASSET_ID::UNTOUCHED_LEVER;
		}

		if (lever.elapsedTime >= lever.animationDelay) {
			lever.frameCounter = lever.frameCounter == 0 ? 1 : 0;
			lever.elapsedTime = 0.0f; // reset the timer
		}

		render_request.used_texture = lever_texture;
	}
	else if (registry.chests.has(entity)) {
		TEXTURE_ASSET_ID chest_texture;
		Chest& chest = registry.chests.get(entity);
		if (chest.state == CHESTSTATE::OPEN) {
			chest_texture = chest.frameCounter == 0 ? TEXTURE_ASSET_ID::CHEST_OPEN : TEXTURE_ASSET_ID::CHEST_CLOSED;
		}
		else {
			chest_texture = TEXTURE_ASSET_ID::CHEST_CLOSED;
		}

		if (chest.elapsedTime >= chest.animationDelay) {
			chest.frameCounter = chest.frameCounter == 0 ? 1 : 0;
			chest.elapsedTime = 0.0f; // reset the timer
		}
		render_request.used_texture = chest_texture;
	}
	else if (registry.doors.has(entity)) {
		TEXTURE_ASSET_ID platform_door_texture;
		auto& platform_door = registry.doors.get(entity);
		if (platform_door.type == DOORTYPE::OPEN) {
			platform_door_texture = TEXTURE_ASSET_ID::OPEN_PLATFORM_DOOR;
		}
		else {
			platform_door_texture = TEXTURE_ASSET_ID::CLOSED_PLATFORM_DOOR;
		}

		render_request.used_texture = platform_door_texture;

	}

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED || render_request.used_effect == EFFECT_ASSET_ID::UI)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)nullptr);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id = texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::PLAYER || render_request.used_effect == EFFECT_ASSET_ID::CPU_PARTICLE)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();

		if (render_request.used_effect == EFFECT_ASSET_ID::PLAYER)
		{
			// Light up?
			GLint light_up_uloc = glGetUniformLocation(program, "light_up");
			assert(light_up_uloc >= 0);
			glUniform1i(light_up_uloc, (int)registry.lightUps.has(entity));
			gl_has_errors();
		}
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::COLOURED )
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();
		
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::REFLECTION)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)nullptr);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id = texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Apply jump

	// Handle jump in topdown mode. It's purely a visual effect.
//	if (registry.jumps.has(entity)) {
//		auto&& position = registry.jumps.get(entity).getPos();
//		transform.translate(position / vec2{window_width_px, window_height_px});
//	}

	// apply tiling
	GLint tilingLocation = glGetUniformLocation(program, "tiling");
	if (render_request.used_texture == TEXTURE_ASSET_ID::FLOOR || 
		render_request.used_texture == TEXTURE_ASSET_ID::PLATFORM_WALL ||
		render_request.used_texture == TEXTURE_ASSET_ID::WALL || 
		render_request.used_texture == TEXTURE_ASSET_ID::DOOR_CLOSED ||
		render_request.used_texture == TEXTURE_ASSET_ID::DOOR_OPEN) // only apply tiling on walls and floors 
	{
		float referenceSize = 50.0;
		vec2 tilingFactor = motion.scale / referenceSize;
		glUniform2f(tilingLocation, tilingFactor.x, tilingFactor.y);
	}
	else if (render_request.used_texture == TEXTURE_ASSET_ID::PLATFORM_FLOOR_GENERIC){
		float referenceSize_x = 1024.0;
		float referenceSize_y = 161.0;
		vec2 tilingFactor = { motion.scale.x / referenceSize_x, motion.scale.y / referenceSize_y };
		glUniform2f(tilingLocation, tilingFactor.x, tilingFactor.y);
	}
	else if (render_request.used_texture == TEXTURE_ASSET_ID::GENERIC_FLOOR_GRASS) {
		float referenceSize_x = 387.0;
		float referenceSize_y = 49.0;
		vec2 tilingFactor = { motion.scale.x / referenceSize_x, motion.scale.y / referenceSize_y };
		glUniform2f(tilingLocation, tilingFactor.x, tilingFactor.y);
	}
	else if (render_request.used_texture == TEXTURE_ASSET_ID::SKY) {
		float referenceSize_x = 1920.f;
		float referenceSize_y = 1080.f;
		vec2 tilingFactor = { motion.scale.x / referenceSize_x, motion.scale.y/ referenceSize_y};
		glUniform2f(tilingLocation, tilingFactor.x, tilingFactor.y);
	}
	else if (render_request.used_texture == TEXTURE_ASSET_ID::MOUNTAIN) {
		float referenceSize_x = 1020.f;
		vec2 tilingFactor = { motion.scale.x / referenceSize_x, 1.0f };
		glUniform2f(tilingLocation, tilingFactor.x, tilingFactor.y);
	}
	else if (render_request.used_texture == TEXTURE_ASSET_ID::GRASS) {
		float referenceSize_x = 1920.f;
		float referenceSize_y = 1080.f;
		vec2 tilingFactor = { motion.scale.x / referenceSize_x,  motion.scale.y / referenceSize_y };
		glUniform2f(tilingLocation, tilingFactor.x, tilingFactor.y);
	}
	else if (render_request.used_texture == TEXTURE_ASSET_ID::SOIL) {
		float referenceSize_x = 576.f;
		vec2 tilingFactor = { motion.scale.x / referenceSize_x, 1.0f };
		glUniform2f(tilingLocation, tilingFactor.x, tilingFactor.y);
	}
else 
	{
		glUniform2f(tilingLocation, 1.0f, 1.0f); // no tiling
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	auto num_indices = static_cast<GLsizei>(size / sizeof(uint16_t));
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);

	GLint projection_loc = glGetUniformLocation(currProgram, "projection");

	if (render_request.used_effect == EFFECT_ASSET_ID::UI) {
		const auto ui_projection = createProjectionMatrix();
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&ui_projection);
	} else {
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	}

	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();

	drawParticles(entity, transform.mat, projection);
}

// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the water texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WATER]);
	gl_has_errors();
	glBindVertexArray(vao);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	// glDepthRange(0, 10);
	// glClearColor(1.f, 0, 0, 1.0);
	// glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint water_program = effects[(GLuint)EFFECT_ASSET_ID::WATER];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(water_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(water_program, "screen_darken_factor");
	GLuint intensity_uloc = glGetUniformLocation(water_program, "intensity");
	GLuint spread_uloc = glGetUniformLocation(water_program, "spread");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	glUniform1f(dead_timer_uloc, screen_state.screen_darken_factor);
	glUniform1f(intensity_uloc, 4.f);
	glUniform1f(spread_uloc, .4f);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

inline void RenderSystem::drawParticles(Entity entity, const mat3 &transform, const mat3 &projection)
{
	if (registry.particleSystems.has(entity))
		registry.particleSystems.get(entity).render(transform, projection);
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();

	mat3 projection_2D = createProjectionMatrix();

	auto& camera_motion = registry.motions.get(registry.cameras.entities[0]);
	Transform projection_transform_copy = projection_transform;

	projection_transform_copy.translate(-camera_motion.position);
	projection_2D *= projection_transform_copy.mat;

	registry.renderRequests.sort(compareEntityLayers);
	// Draw all textured meshes that have a position and size component
	for (Entity entity : registry.renderRequests.entities)
	{
		if (registry.texts.has(entity)) {
			drawText(entity);
		}

		if (!registry.motions.has(entity))
			continue;
		// Note, it's not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize
		drawTexturedMesh(entity, projection_2D);
	}

	// Write to screen

	// Truly render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float)window_width_px;
	float bottom = (float)window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

// adapted from LearnOpenGL
void  RenderSystem::drawText(Entity entity)
{
	mat3 projection = createProjectionMatrix();

	Text& text = registry.texts.get(entity);
	std::string message = text.message;
	float x = text.position.x;
	float y = text.position.y;
	float scale = text.scale;

	const RenderRequest& render_request = registry.renderRequests.get(entity);

	const GLuint program = (GLuint)effects[(GLuint)render_request.used_effect];
	glUseProgram(program);
	gl_has_errors();

	glBindVertexArray(vao);

	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();


	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);

	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	gl_has_errors();
	assert(in_texcoord_loc >= 0);

	// same code as in drawTexturedMesh
	glEnableVertexAttribArray(in_position_loc);
	gl_has_errors();
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
		sizeof(TexturedVertex), (void*)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position

	glActiveTexture(GL_TEXTURE0);



	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = text.color;
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);

	Transform transform;

	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// iterate through all characters
	for (const auto& c : message)
	{
		Character ch = Characters[c];

		float xpos = x + static_cast<float>(ch.Bearing.x) * scale;
		float ypos = y - static_cast<float>(ch.Size.y - ch.Bearing.y) * scale;

		float w = static_cast<float>(ch.Size.x) * scale;
		float h = static_cast<float>(ch.Size.y) * scale;

		std::vector<TexturedVertex> textured_vertices(4);
		textured_vertices[0].position = { xpos, ypos + h, 0.f };
		textured_vertices[1].position = { xpos + w, ypos + h, 0.f };
		textured_vertices[2].position = { xpos + w, ypos, 0.f };
		textured_vertices[3].position = { xpos, ypos, 0.f };
		textured_vertices[0].texcoord = { 0.f, 1.f };
		textured_vertices[1].texcoord = { 1.f, 1.f };
		textured_vertices[2].texcoord = { 1.f, 0.f };
		textured_vertices[3].texcoord = { 0.f, 0.f };

		const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };

		// Load texture as quads
		bindVBOandIBO(GEOMETRY_BUFFER_ID::TEXT, textured_vertices, textured_indices);
		gl_has_errors();
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
		// The unit for advance is 1/64 pixels.
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
}
