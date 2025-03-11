#include <random>
#include "EnemyLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Components//Visuals.h"
#include "../Events/Playevents.h"
#include "../Events/GameStates.h"

using namespace MX; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool MX::EnemyLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	eventPusher = _eventPusher;

	onGameStart.Create([&](const GW::GEvent e) {
		MX::MENU_STATE currState; MX::MENU_STATE_DATA menuData;
		if (+e.Read(currState, menuData) && currState == MX::MENU_STATE::MAINMENU)
		{
			isPaused = menuData.mainMenu;

			if (isPaused)
			{
				CleanupEnemies();
			}
		}
		});
	eventPusher.Register(onGameStart);

	// A System that controls the enemies collisions with other entities
	game->system<Enemy, Health, Damage, Score>("Enemy Collision System")
		.each([this](flecs::entity e, Enemy, Health& h, Damage& d, Score& s) {

		// damage the player if the enemy gets close to him/her
		e.each<CollidedWith>([this, &e, d, s](flecs::entity hit) {
			if (hit.has<Player>())
			{
#if _DEBUG
				//std::cout << "Enemy has Hit the Player!" << "\n";
#endif

				if (hit.has<Health>())
				{
					int current = hit.get<Health>()->value;
					current -= d.dmgValue;
					hit.set<Health>({ current });

					hit.set<Damage>({ 0, true });

					MX::PLAY_EVENT_DATA x;
					GW::GEvent playerTakingDamage;
					x.DmgValue = d.dmgValue;
					x.ScrValue = s.scrValue;
					x.Health = current;
					playerTakingDamage.Write(MX::PLAY_EVENT::PLAYER_DAMAGED, x);
					eventPusher.Push(playerTakingDamage);

					// Play Explosion sound
					GW::AUDIO::GSound explode = *e.get<GW::AUDIO::GSound>();
					explode.Play();
					// destroy the Enemy
					e.destruct();
				}

			}
			});

		if (e.get<Health>()->value <= 0) { 		// if enemy has no health left be destroyed
			// play explode sound
  			GW::AUDIO::GSound explode = *e.get<GW::AUDIO::GSound>();
			explode.Play();
			e.destruct();

			MX::PLAY_EVENT_DATA x;
			GW::GEvent playerDestroysEnemy;
			x.DmgValue = d.dmgValue;
			x.ScrValue = s.scrValue;
			x.Health = game->get<PlayerHealth>()->health;
			playerDestroysEnemy.Write(MX::PLAY_EVENT::ENEMY_DESTROYED, x);
			eventPusher.Push(playerDestroysEnemy);

 			std::cout << "Enemy Destroyed! +" << s.scrValue << "\n";
		}

		onPlayerDeath.Create([this](const GW::GEvent& e) {
			MX::PLAY_EVENT event; MX::PLAY_EVENT_DATA eventData;
			if (+e.Read(event, eventData) && eventData.playerIsDead == true)
			{
				CleanupEnemies();
			}
			});
		eventPusher.Register(onPlayerDeath);
			});

	return true;
}

// Free any resources used to run this system
bool MX::EnemyLogic::Shutdown()
{
	CleanupEnemies();
	game->entity("Enemy System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

bool MX::EnemyLogic::CleanupEnemies()
{
	game->defer_begin();
	game->each([](flecs::entity e, Enemy&) {
		e.destruct();
		std::cout << "Enemy SUCCESSFULLY Destroyed!" << '\n';
		});
	game->defer_end();

	return true;
}

// Toggle if a system's Logic is actively running
bool MX::EnemyLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Enemy System").enable();
	}
	else {
		game->entity("Enemy System").disable();
	}
	return false;
}



// Old Default ini stuff for other enemies
/*[Enemy2] # Light Enemy ( High Speed & High Damage & Low Health)
health=25
damage=30
blue=1
green=0.5
red=0
xscale=0.20
yscale=0.40
ystart=1.3
angle=180
accmax=.75
accmin=0.50
model=Dispatcher
texture=Dispatcher_Blue
[Enemy3] # Heavy Enemy ( Low Speed & High Damage & High Health)
health=100
damage=40
blue=0.5
green=1
red=0
xscale=0.20
yscale=0.40
ystart=1.3
angle=180
accmax=0.25
accmin=0.05
model=Pancake
texture=Pancake_Blue*/