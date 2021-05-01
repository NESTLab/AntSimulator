#pragma once

#include <list>
#include "world.hpp"
#include "config.hpp"
#include "direction.hpp"
#include "number_generator.hpp"
#include "ant_mode.hpp"
#include "cooldown.hpp"
#include "colony_base.hpp"

#include <iostream>


struct Ant
{
	// Parameters
	float width = 3.0f;
	float length = 4.7f;
	float move_speed = 50.0f;
	float marker_detection_max_dist = 40.0f;
	float direction_update_period = 0.125f;
	float marker_period = 0.125f;
	float direction_noise_range = PI * 0.1f;
	float repellent_period = 16.0f;

	Mode phase;
	sf::Vector2f position;
	Direction direction;
	uint32_t hits;

	Cooldown direction_update;
	Cooldown marker_add;
	Cooldown search_markers;
	float markers_count;
	float liberty_coef;
	float autonomy;
	float max_autonomy = 250.0f;

	Ant() = default;

	Ant(float x, float y, float angle)
		: position(x, y)
		, direction(angle)
		, direction_update(direction_update_period, RNGf::getUnder(1.0f) * direction_update_period)
		, marker_add(marker_period, RNGf::getUnder(1.0f) * marker_period)
		, search_markers(5.0f, 5.0f)
		, phase(Mode::ToFood)
		, liberty_coef(RNGf::getRange(0.001f, 0.01f))
		, hits(0)
		, markers_count(0.0f)
		, autonomy(0.0f)
	{
	}

	void update(float dt, World& world)
	{
		autonomy += dt;
		updatePosition(world, dt);
		if (phase == Mode::ToFood) {
			checkFood(world);
		}
		// Get current cell
		WorldCell& cell = world.map.get(position);
		cell.addPresence();
		search_markers.update(dt);
		direction_update.update(dt);
		if (direction_update.ready()) {
			if (search_markers.ready()) {
				findMarker(world, dt);
			}
			else {
				cell.degrade();
			}
			direction += RNGf::getFullRange(direction_noise_range);
			direction_update.reset();
		}
		// Add marker
		marker_add.update(dt);
		if (marker_add.ready()) {
			addMarker(world);
			marker_add.reset();
		}
		// Update current direction
		direction.update(dt);
	}

	void updatePosition(World& world, float dt)
	{
		sf::Vector2f v = direction.getVec();
		const sf::Vector2f next_position = position + (dt * move_speed) * v;
		const HitPoint hit = world.map.getFirstHit(position, v, dt * move_speed);
		if (hit.cell) {
			const uint32_t hits_threshold = 8;
			if (hits > hits_threshold) {
				const float new_angle = RNGf::getUnder(2.0f * PI);
				v = sf::Vector2f(cos(new_angle), sin(new_angle));
			}
			else {
				v.x *= hit.normal.x ? -1.0f : 1.0f;
				v.y *= hit.normal.y ? -1.0f : 1.0f;
			}
			++hits;
			direction.setDirectionNow(v);
		}
		else {
			hits = 0;
			position += (dt * move_speed) * v;
			// Ants outside the map go back to home
			position.x = (position.x < 0.0f || position.x > Conf::WIN_WIDTH) ? Conf::COLONY_POSITION.x : position.x;
			position.y = (position.y < 0.0f || position.y > Conf::WIN_HEIGHT) ? Conf::COLONY_POSITION.y : position.y;
		}
	}

	void checkFood(World& world)
	{
		if (world.map.isOnFood(position)) {
			phase = Mode::ToHome;
			direction.addNow(PI);
			autonomy = 0.0f;
			markers_count = 0.0f;
			if (world.map.pickFood(position)) {
				phase = Mode::ToHomeNoFood;
				marker_add.target = repellent_period;
				marker_add.value = RNGf::getUnder(marker_add.target);
				world.addMarkerRepellent(position, getMarkerIntensity(0.1f));
			}
		}
	}

	void checkColony(ColonyBase& base)
	{
		if (getLength(position - base.position) < base.radius) {
			marker_add.target = marker_period;
			if (phase == Mode::ToHome || phase == Mode::ToHomeNoFood) {
				phase = Mode::ToFood;
				base.addFood(1.0f);
				direction.addNow(PI);
			}
			// Refill
			const float refill_cost = 1.0f;
			const float needed_refill = refill_cost * (autonomy / max_autonomy);
			if (base.useFood(needed_refill)) {
				autonomy = 0.0f;
				phase = Mode::ToFood;
			}
			markers_count = 0.0f;
		}
	}

	Mode getMarkersSamplingType() const
	{
		if (phase == Mode::ToHome || phase == Mode::Refill || phase == Mode::ToHomeNoFood) {
			return Mode::ToHome;
		}
		return Mode::ToFood;
	}

	void findMarker(World& world, float dt)
	{
		// Init
		const float sample_angle_range = PI * 0.85f;
		const float current_angle = direction.getCurrentAngle();
		float max_intensity = 0.0f;
		// To objective stuff
		sf::Vector2f max_direction = direction.getVec();
		WorldCell* max_cell = nullptr;
		bool found_permanent = false;
		// Repellent stuff
		float max_repellent = 0.0f;
		WorldCell* repellent_cell = nullptr;
		// Explore
		sf::Vector2f max_explore;
		float max_wall_dist = -1.0f;
		float wall_sum = 0.0f;
		float total_wall_cell = 0.0f;
		// Sample the world
		const Mode marker_phase = getMarkersSamplingType();
		const uint32_t sample_count = 32;
		for (uint32_t i(sample_count); i--;) {
			// Get random point in range
			const float sample_angle = current_angle + RNGf::getRange(sample_angle_range);
			const float distance = RNGf::getUnder(marker_detection_max_dist);
			const sf::Vector2f to_marker(cos(sample_angle), sin(sample_angle));
			auto* cell = world.map.getSafe(position + distance * to_marker);
			// Check cell
			if (!cell) {
				continue;
			}
			// Check for food or colony
			if (cell->isPermanent(marker_phase)) {
				max_direction = to_marker;
				found_permanent = true;
				break;
			}
			// Avoid walls
			total_wall_cell += 1.0f;
			wall_sum += cell->wall_dist;
			const float dir_coef = 1.0f;
			max_explore += to_marker * (1.0f - cell->wall_dist);
			// Flee if repellent
			if (cell->repellent > max_repellent) {
				max_repellent = cell->repellent;
				repellent_cell = cell;
			}
			// Check for the most intense marker
			if (cell->getIntensity(marker_phase) > max_intensity) {
				max_intensity = cell->getIntensity(marker_phase);
				max_direction = to_marker;
				max_cell = cell;
			}
			// Randomly choose own path
			if (RNGf::proba(liberty_coef)) {
				break;
			}
		}
		// Check for repellent
		if (phase == Mode::ToFood && max_repellent && !found_permanent) {
			const float repellent_probe_factor = 0.1f;
			if (RNGf::proba(repellent_probe_factor*(1.0f - max_intensity / Conf::MARKER_INTENSITY))) {
				//phase = Mode::Flee;
				direction.addNow(RNGf::getUnder(2.0f * PI));
				search_markers.reset();
				return;
			}
		}
		// Remove repellent if still food
		if (repellent_cell && phase == Mode::ToHome) {
			repellent_cell->repellent *= 0.95f;
		}
		// Update direction
		if (max_intensity) {
			if (RNGf::proba(0.8f) && phase == Mode::ToFood) {
				max_cell->intensity[static_cast<uint32_t>(phase)] *= 0.98f;
			}
			direction = getAngle(max_direction);
		}

		if (wall_sum) {
			const float avoid_coef = 2.0f;
			const float avoid_ratio = std::min(1.0f, avoid_coef * wall_sum / total_wall_cell);
			max_explore = getNormalized(max_explore);
			const sf::Vector2f dir = avoid_ratio * max_explore + (1.0f - avoid_ratio) * max_direction;
			direction = getAngle(dir);
		}
	}

	void addMarker(World& world)
	{
		markers_count += marker_add.target;
		if (phase == Mode::ToHome || phase == Mode::ToFood) {
			const float intensity = getMarkerIntensity(0.05f);
			world.addMarker(position, phase == Mode::ToFood ? Mode::ToHome : Mode::ToFood, intensity);
		}
		else if (phase == Mode::ToHomeNoFood) {
			const float intensity = getMarkerIntensity(0.1f);
			world.addMarkerRepellent(position, intensity);
		}
	}

	void render_food(sf::RenderTarget& target, const sf::RenderStates& states) const
	{
		if (phase == Mode::ToHome) {
			const float radius = 2.0f;
			sf::CircleShape circle(radius);
			circle.setOrigin(radius, radius);
			circle.setPosition(position + length * 0.65f * direction.getVec());
			circle.setFillColor(Conf::FOOD_COLOR);
			target.draw(circle, states);
		}
	}

	void render_in(sf::VertexArray& va, const uint32_t index) const
	{
		const sf::Vector2f dir_vec(direction.getVec());
		const sf::Vector2f nrm_vec(-dir_vec.y, dir_vec.x);

		va[index + 0].position = position - width * nrm_vec + length * dir_vec;
		va[index + 1].position = position + width * nrm_vec + length * dir_vec;
		va[index + 2].position = position + width * nrm_vec - length * dir_vec;
		va[index + 3].position = position - width * nrm_vec - length * dir_vec;
	}

	float getMarkerIntensity(float coef)
	{
		return Conf::MARKER_INTENSITY * exp(-coef * markers_count);
	}
};
