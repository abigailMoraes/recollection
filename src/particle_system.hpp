#pragma once

#include <random>
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"


struct Particle {
	vec2 position = { 0.f, 0.f };
	vec2 velocity = { 0.f, 0.f };
	vec2 acc = { 0.f, 0.f };
	float time_ms = 0.f;
	float life_time = 10'000.f;
};


class ParticleSystem {
	std::vector<Particle> particles;
	size_t n_alive = 0;
	bool firing = false;
	float fire_rate = 1.f;
	size_t max_particles = 1000; // Maximum particles allowed by this
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
	GLuint shader; // shader location
	GLuint VAO{0};
	GLuint VBO{0};

public:
	explicit ParticleSystem(GLuint shader_arg);
	size_t emit(size_t n_particles);
	void update(float elapsed_ms);
	void reset();
	void remove(size_t particle_i);
	void render(const mat3 &transform, const mat3 &projection);
};
