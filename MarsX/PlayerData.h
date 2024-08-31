// This class populates all player entities 
#ifndef PLAYERDATA_H
#define PLAYERDATA_H

// Contains our global game settings
#include "../GameConfig.h"

// example space game (avoid name collisions)
namespace MX
{
	class PlayerData
	{
	public:
		flecs::entity* player;
		// Load required entities and/or prefabs into the ECS 
		bool Load(	std::shared_ptr<flecs::world> _game,
					std::weak_ptr<const GameConfig> _gameConfig);
		// Resets the Entity Back to it's original State
		bool Reset(std::shared_ptr<flecs::world> _game,
				   std::weak_ptr<const GameConfig> _gameConfig);
		// Unload the entities/prefabs from the ECS
		bool Unload(std::shared_ptr<flecs::world> _game);
	};

};

#endif