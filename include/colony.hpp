#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include "ant.hpp"
#include "utils.hpp"
#include "world.hpp"


struct Colony
{
  /**
   * @brief Construct a new Colony object
   * 
   * @param x Colony X position
   * @param y Colony Y position
   * @param n Colony Number of ants
   * @param mal_prob Probability of an ant being malicious (fraction of ants being malicious)
   * @param mal_timer_delay Delay after which the attack is launched
   * @param malicious_ants_focus  Should the attack be focused towards food
   * @param ant_tracing_pattern   Should malicious ants trace food pheromone or roam randomly
	 * @param counter_pheromone Will the ants secret counter pheromone?
	 * @param hell_phermn_intensity_multiplier multiplier for the intensity of TO_HELL pheromone
   */
	Colony(float x, float y, uint32_t n, float mal_prob, int mal_timer_delay, bool malicious_ants_focus = true, 
          AntTracingPattern ant_tracing_pattern = AntTracingPattern::RANDOM, bool counter_pheromone = false,
          float hell_phermn_intensity_multiplier = 1.0)
		: position(x, y)
		, last_direction_update(0.0f)
		, ants_va(sf::Quads, 4 * n)
    , mal_timer_delay(mal_timer_delay)
    , timer_count(0)
    , timer_count2(0)
    , confused_count(0)
	{
    // std::cout<<std::abs(((double) rand() / (RAND_MAX)));
      // std::cout<<"Normal";

    for (uint64_t i(0); i < n; ++i) {
      if(i >= mal_prob*n)
      {
        ants.emplace_back(x, y, getRandRange(2.0f * PI), counter_pheromone);

        const uint64_t index = 4 * i;
        ants_va[index + 0].color = Conf::ANT_COLOR;
        ants_va[index + 1].color = Conf::ANT_COLOR;
        ants_va[index + 2].color = Conf::ANT_COLOR;
        ants_va[index + 3].color = Conf::ANT_COLOR;

        ants_va[index + 0].texCoords = sf::Vector2f(0.0f, 0.0f);
        ants_va[index + 1].texCoords = sf::Vector2f(73.0f, 0.0f);
        ants_va[index + 2].texCoords = sf::Vector2f(73.0f, 107.0f);
        ants_va[index + 3].texCoords = sf::Vector2f(0.0f, 107.0f);
      }
      else
      {
          // STARTING ANGLE OF MALICIOUS ANT
          float angle;
          if(malicious_ants_focus)
            angle = -3*PI/4;
          else
            angle = getRandRange(2.0f * PI); // Sets even distribution

          ants.emplace_back(x+400, y+400, angle, false, true, ant_tracing_pattern, hell_phermn_intensity_multiplier); 

          const uint64_t index = 4 * i;
          ants_va[index + 0].color = Conf::MALICIOUS_ANT_COLOR;
          ants_va[index + 1].color = Conf::MALICIOUS_ANT_COLOR;
          ants_va[index + 2].color = Conf::MALICIOUS_ANT_COLOR;
          ants_va[index + 3].color = Conf::MALICIOUS_ANT_COLOR;

          ants_va[index + 0].texCoords = sf::Vector2f(0.0f, 0.0f);
          ants_va[index + 1].texCoords = sf::Vector2f(73.0f, 0.0f);
          ants_va[index + 2].texCoords = sf::Vector2f(73.0f, 107.0f);
          ants_va[index + 3].texCoords = sf::Vector2f(0.0f, 107.0f);
		  }
    }
	}

	void update(const float dt, World& world)
	{	
    confused_count = 0;
    ants_that_found_food = 0; 
    ants_that_delivered_food = 0;
    bool wreak_havoc = timer_count >= mal_timer_delay ? true : false;
		for (Ant& ant : ants) {
      if(!skip_once)
			  ant.checkColony(position);
			ant.update(dt, world, wreak_havoc);
      if(ant.didAntFindFood())
        ants_that_found_food++;
      if(ant.didAntDeliverFood())
        ants_that_delivered_food++;
		}
    skip_once = false;
    if(wreak_havoc)
    {
      timer_count = 0;
    }
    else
      timer_count ++;
    timer_count2 ++;
	}

  static int getAntsThatFoundFood()
  {
    return ants_that_found_food;
  }

  static int getAntsThatDeliveredFood()
  {
    return ants_that_delivered_food;
  }

	void render(sf::RenderTarget& target, const sf::RenderStates& states) const
	{
		for (const Ant& a : ants) {
			a.render_food(target, states);
		}

		uint32_t index = 0;
		for (const Ant& a : ants) {
			a.render_in(ants_va, 4 * (index++));
		}

		sf::RenderStates rs = states;
		rs.texture = &(*Conf::ANT_TEXTURE);
		target.draw(ants_va, rs);
	}

	const sf::Vector2f position;
	std::vector<Ant> ants;
	mutable sf::VertexArray ants_va;
	const float size = 20.0f;

	float last_direction_update;
	const float direction_update_period = 0.25f;

  int mal_timer_delay;
  int timer_count;
  int timer_count2;
  int confused_count;
  float counter_rise_fraction;
  bool skip_once = true;

  inline static int ants_that_found_food;
  inline static int ants_that_delivered_food;
};
