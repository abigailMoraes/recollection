#pragma once

#include "particle_system.hpp"

ParticleSystem::ParticleSystem(GLuint shader_arg) {
	// set up mesh and attribute properties
	shader = shader_arg;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
}

size_t ParticleSystem::emit(size_t n_new_particles) {
	for (auto i = 0; i < n_new_particles; i++) {
		Particle new_particle = Particle{
			{ 0.f, 0.f },
		    { 0.f, 0.f},
		    { 0.00001f * (uniform_dist(rng) - .5), -0.00'000'5f * (-.5f + uniform_dist(rng))},
			0.f,
			1'000.f
		};
		particles.push_back(new_particle);
	}

	n_alive += n_new_particles;

	return n_new_particles;
}

void ParticleSystem::update(float elapsed_ms) {
	if (firing) emit(min(static_cast<size_t>(fire_rate * elapsed_ms), max_particles - n_alive));

	for (auto i = 0; i < n_alive; i++) {
		Particle& particle = particles[i];
		particle.time_ms += elapsed_ms;
		if (particle.time_ms > particle.life_time)
			remove(i);
		particle.position += particle.velocity * elapsed_ms;
		particle.velocity += particle.acc * elapsed_ms;
	}
}

void ParticleSystem::reset() {
	firing = false;
}

void ParticleSystem::remove(size_t particle_i) {
	particles[particle_i] = std::move(particles.back());
	n_alive--;
	particles.pop_back();
}

void ParticleSystem::render(const mat3 &transform, const mat3 &projection) {
	glUseProgram(shader);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	gl_has_errors();

	// Just use attrib location
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
	                      sizeof(Particle), (void *)offsetof(Particle, position));
	gl_has_errors();
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
	                      sizeof(Particle), (void *)offsetof(Particle, velocity));
	gl_has_errors();
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
	                      sizeof(Particle), (void *)offsetof(Particle, acc));
	gl_has_errors();
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE,
	                      sizeof(Particle), (void *)offsetof(Particle, time_ms));
	gl_has_errors();
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE,
	                      sizeof(Particle), (void *)offsetof(Particle, life_time));

	// Buffer the data inside the particle vector (array).
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Particle) * n_alive), particles.data(), GL_STATIC_DRAW);

	// Setting uniform values to the currently bound program
	GLint transform_loc = glGetUniformLocation(shader, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform);
	GLint projection_loc = glGetUniformLocation(shader, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);

	gl_has_errors();

	glDrawArrays(GL_POINTS, 0, (GLsizei) n_alive);
	glBindVertexArray(0);
}
