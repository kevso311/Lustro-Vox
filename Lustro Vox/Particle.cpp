#include "Particle.h"
#include <math.h>
#include <random>
#include <iostream>
#include <time.h>
#include <omp.h>


Particle::Particle(float x, float y)
{
	position.x = x;
	position.y = y;
	speed.x = 0.0f;
	speed.y = 0.0f;
}
Particle::~Particle()
{
}

void Particle::updateForces(float grav)
{
	speed.y += grav;
	if (speed.y > 0.001f)
		speed.y = 0.001f;

	speed.y *= 0.999f;
	speed.x *= 0.999f;
}

void Particle::move()
{
	position.x += speed.x;
	position.y += speed.y;
}

void paintParticles(std::vector<Particle>& particles, std::vector<sf::Uint8>& canvas, int width, int height)
{
	float size = (float)std::max(width, height);

	int n = particles.size();
	#pragma omp parallel for
	for (int i{ 0 }; i < n; ++i) {
		auto p = particles[i];
		if (p.position.x > 1.0 || p.position.y > 1.0) continue;
		
		int row = (int)round(p.position.y * size);
		int column = (int)round(p.position.x * size);

		int index = row * width + column;
		if (index < width*height) {
			canvas[row*width + column] = p.color; 
		}
	}
}

std::vector<Particle> randomParticles(int count) {
	std::vector<Particle> particles;

	std::default_random_engine generator(time(NULL) + rand());
	std::uniform_real_distribution<float> distributionX(0.0, 1.0);
	std::uniform_real_distribution<float> distributionY(0.0, 0.1);
	auto rx = [&]() { return distributionX(generator); };
	auto ry = [&]() { return distributionY(generator); };


	for (int i{ 0 }; i < count; ++i) {
		Particle p{rx(), ry()};
		p.color = (rand() % 15) + 1;
		particles.push_back(p);
	}
	return particles;
}