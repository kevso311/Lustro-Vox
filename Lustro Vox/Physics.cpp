#include "Physics.h"
#include "Colors.h"
#include "Particle.h"
#include <iostream>
#include <thread>
#include <SFML/System.hpp>

Physics::Physics(int w, int h)
{
	width = w;
	height = h;
	canvas.assign(width*height, colors::BLACK);
	current_frame.assign(width*height, colors::BLACK);
	particles = randomParticles(5000);
}

void Physics::getFrame(std::vector<sf::Uint8>& frame) {
	current_frame_lock.lock();
	frame = current_frame;
	current_frame_lock.unlock();
}

void Physics::start()
{
	t = std::thread(&Physics::update, this);
}

void Physics::stop()
{
	run = false;
	t.join();
}

void Physics::addWave(Wave & wave)
{
	waves.push_back(wave);
}

void Physics::update()
{
	while (run) {
		sf::Clock time;
		++ticks;
		canvas.assign(width*height, colors::BLACK);
		int n = particles.size();

		//Update waves
		for (auto &w : waves) {
			w.update();
		}

		//Update forces on particles from waves
		for (auto &w : waves) {
			#pragma simd
			#pragma omp parallel for
			for (int i{ 0 }; i < n; ++i) {
				auto &p = particles[i];
				
				float distance = sqrt(pow((p.getX() - w.getX()), 2) + pow((p.getY() - w.getY()), 2));
				
				if (distance < (w.getRadius() + waveMargin) && distance >(w.getRadius() - waveMargin)) {
					sf::Vector2<float> forceExert = p.position - w.position;
					forceExert *= w.getForce();
				}

			}
		}

		//Update gravity and air resistance
		#pragma simd
		#pragma omp parallel for
		for (int i{ 0 }; i < n; ++i) {

			auto &p = particles[i];
			p.updateForces(gravity);
			p.move();
			
			if (p.getY() > 0.5f) {
				p.speed.y = -1 * (float)abs(p.speed.y);
				
				if (rand() % 2 == 0)
					p.speed.x = p.speed.y;
				else
					p.speed.x = -1*p.speed.y;
					
			}
			else if (p.getX() < 0.0f) {
				p.speed.x = (float)abs(p.speed.x);
			}
			else if (p.getX() > 1.0f) {
				p.speed.x = -1 * (float)abs(p.speed.x);
			}
			
			
				

		}

		//Paint particles to canvas
		paintParticles(particles, canvas, width, height);
		
		//Send the new canvas out for rendering
		current_frame_lock.lock();
		std::swap(current_frame, canvas);
		current_frame_lock.unlock();
		if (time.getElapsedTime().asMilliseconds() < 1) 
			sf::sleep(sf::milliseconds(1 - time.getElapsedTime().asMilliseconds()));
	}
}

