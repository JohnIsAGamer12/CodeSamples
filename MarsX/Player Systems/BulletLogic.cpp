#include <random>
#include "BulletLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"
#include "../Events/GameStates.h"



using namespace MX; // Example Space Game
// Connects logic to traverse any players and allow a controller to manipulate them
bool MX::BulletLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig, GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	eventPusher = _eventPusher;

	onGameStart.Create([&](const GW::GEvent e) {
		MX::PAUSE_STATE currState; MX::PAUSE_STATE_DATA menuData;
		if (+e.Read(currState, menuData) && currState == MX::PAUSE_STATE::PAUSED)
		{
			isPaused = menuData.isPaused;

			if (isPaused)
			{
				CleanUpBullets();
			}
		}
		});
	eventPusher.Register(onGameStart);

	onPlayerDeath.Create([this](const GW::GEvent& e) {
		MX::PLAY_EVENT event; MX::PLAY_EVENT_DATA eventData;
		if (+e.Read(event, eventData) && eventData.playerIsDead == true)
		{
			CleanUpBullets();
		}
		});
	eventPusher.Register(onPlayerDeath);

	// destroy any bullets that have the CollidedWith relationship
	game->system<Bullet, Transform::Position, RigidBody::Acceleration, Damage>("Bullet System")
		.each([this](flecs::entity e, Bullet&, Transform::Position& p, RigidBody::Acceleration& a, Damage& d) {

		// damage anything we come into contact with
		e.each<CollidedWith>([this, &e, d](flecs::entity hit) {
			if (hit.has<Enemy>())
			{
				if (hit.has<Health>()) {
					int current = hit.get<Health>()->value;

					hit.set<Health>({ current - d.dmgValue });
					std::cout << "Enemy Took " << d.dmgValue << '\n';

					int currentEnemyDmg = hit.get<Damage>()->dmgValue;

					hit.set<Damage>({ currentEnemyDmg, true });

					// destroy the bullet on collision
					e.destruct();
				}
			}
			});

		e.each<FiredBy>([this, &e, p](flecs::entity player) 
			{
				if (player.has<Player>())
				{
					GW::MATH::GVECTORF currentPlayerPos = player.get<Transform::Position>()->value;


					if (p.value.z > currentPlayerPos.z + 300.0f || p.value.z < currentPlayerPos.z + -300.0f) 
					{
						e.destruct();
					}
				}
			});
			});

	return true;
}

// Free any resources used to run this system
bool MX::BulletLogic::Shutdown()
{
	CleanUpBullets();
	game->entity("Bullet System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

bool MX::BulletLogic::CleanUpBullets()
{
	game->defer_begin();
	game->each([](flecs::entity e, Bullet&) {
		e.destruct();
		std::cout << "Bullet SUCCESSFULLY Destroyed!" << '\n';
		});
	game->defer_end();

	return true;
}

// Toggle if a system's Logic is actively running
bool MX::BulletLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Bullet System").enable();
	}
	else {
		game->entity("Bullet System").disable();
	}
	return false;
}
