// The Bullet system is responsible for inflicting damage and cleaning up bullets
#ifndef POWERUPLOGIC_H
#define POWERUPLOGIC_H
// Contains our global game settings
#include "../GameConfig.h"
#include "../Entities/BulletData.h"
#include "../Components/Visuals.h"

// example space game (avoid name collisions)
namespace MX
{
	class PowerUpLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// system for the Pickups
		flecs::system powerUpPickUpSystem;
		// handle to events
		GW::CORE::GEventGenerator eventPusher;
		// Event System
		GW::CORE::GEventResponder onPlayerDeath;
		// Event System
		GW::CORE::GEventResponder OnPlayerPoweredUp;
		GW::CORE::GEventResponder onPlayerDead;
		GW::CORE::GEventResponder onGameStart;

		bool isPaused = true;
	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::CORE::GEventGenerator _eventPusher);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
		bool CleanupPickups();


		void PoweredMissiles(flecs::entity& hit);
	};

};

#endif