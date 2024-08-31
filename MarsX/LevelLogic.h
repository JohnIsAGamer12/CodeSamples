// The level system is responsible for transitioning the various levels in the game
#ifndef LEVELLOGIC_H
#define LEVELLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
// Entities for players, enemies & bullets
#include "../Systems/PlayerLogic.h"
#include "../Systems/EnemyLogic.h"
#include "../Systems/BulletLogic.h"
#include "../Systems/PowerUpLogic.h"
#include "../Entities/BulletData.h"

// example space game (avoid name collisions)
namespace MX
{
	class LevelLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// async version of above for threaded operations
		flecs::world gameAsync;
		// mutex used to protect access to gameAsync 
		GW::CORE::GThreadShared gameLock;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// Level system will also load and switch music
		GW::AUDIO::GAudio audioEngine;
		GW::AUDIO::GMusic currentTrack;
		// Used to spawn enemies at a regular intervals on another thread
		GW::SYSTEM::GDaemon randomEnemySpawningEvent;
		GW::SYSTEM::GDaemon proceduralPickupSpawingEvent;

		MX::BulletLogic bulletSystemRef;
		bool startSpawningEnemies = true;
		MX::EnemyLogic enemySystemRef;
		bool startSpawningPickUps = true;
		MX::PowerUpLogic pickUpSystemRef;

		bool isPaused = true;
		GW::CORE::GEventGenerator eventPusher;
		GW::CORE::GEventResponder onPaused;
		GW::CORE::GEventResponder onGameOver;
		GW::CORE::GEventResponder onGameStart;
		GW::CORE::GEventResponder onPlayerDeath;

		std::vector<flecs::entity> listofEnemies;
	public:
		// attach the required logic to the ECS 
		bool Init(	std::shared_ptr<flecs::world> _game,
					std::weak_ptr<const GameConfig> _gameConfig, GW::AUDIO::GAudio _audioEngine, 
					GW::CORE::GEventGenerator _eventPusher, MX::PlayerLogic& _playerSystem, MX::BulletLogic& _bulletSystem,
					MX::EnemyLogic& _enemySystem, MX::PowerUpLogic& _pickUpSystem);
		void SpawnEnemies(float enemySpawnDelay, float scale, float enemy1startX, float enemy1accmax, 
			float enemy1accmin, const flecs::entity& enemyentity);
		void SpawnPickUps(float pickupSpawnDelay, const char* playerName, const flecs::entity& pickUpEntity,
			MX::PlayerLogic& _playerSystem, MX::EnemyLogic& _enemySystem, MX::BulletLogic& _bulletSystem, MX::PowerUpLogic& _pickUpSystem);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
	};

};

#endif