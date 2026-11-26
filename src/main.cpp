
#define GL3W_IMPLEMENTATION
#include <gl3w.h>
#include <GLFW/glfw3.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "common.hpp"
#include <GLFW/glfw3.h>
#include "ai_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	
	// Global systems
	WorldSystem world_system;
	RenderSystem render_system;
	PhysicsSystem physics_system;
	AISystem ai_system;
	


	// Initializing window

	glfwInit();

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	window_width_px = mode->width;
	window_height_px = mode->height;
	window_width_f = (float) mode->width;
	window_height_f = (float) mode->height;
	window_center = vec2{window_width_f / 2.0f,window_height_f / 2.0f};

	GLFWwindow* window = world_system.create_window(window_width_px, window_height_px);

	if (!window) {

		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems


	render_system.init(window);
	world_system.init(&render_system);
	ai_system.init(&world_system);

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;
		world_system.step(elapsed_ms);
		physics_system.step(elapsed_ms * world_system.current_speed);
		ai_system.step(elapsed_ms);

		world_system.handle_collisions(elapsed_ms * world_system.current_speed);

		render_system.draw();
	}

	return EXIT_SUCCESS;
}
