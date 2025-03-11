#include <random>
#include "PowerUpLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"
#include "../Events/GameStates.h"


using namespace MX; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool MX::PowerUpLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	eventPusher = _eventPusher;
	void* powerUpPickup = nullptr;

	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();

	// std::string model = (*readCfg).at("PowerUp-Missiles").at("model").as<std::string>();


	onGameStart.Create([&](const GW::GEvent e) {
		MX::MENU_STATE currState; MX::MENU_STATE_DATA menuData;
		if (+e.Read(currState, menuData) && currState == MX::MENU_STATE::MAINMENU)
		{
			isPaused = menuData.mainMenu;

			if (!isPaused)
			{
				CleanupPickups();
			}
		}
		});
	_eventPusher.Register(onGameStart);

	// give Player
	powerUpPickUpSystem = game->system<Pickup, Transform::Orientation>("PickUp System")
		.each([this](flecs::entity e, Pickup, Transform::Orientation& o) {

		if (e.get<Transform::Position>()->value.y < 6.0f)
			e.destruct();
		GW::MATH::GMatrix::RotateYLocalF(o.value, 0.01f, o.value);

		// gives the player a "PowerUp" upon colliding with the pickup entitiy
		e.each<CollidedWith>([this, &e](flecs::entity hit) {
			if (hit.has<Player>())
			{
				if (!hit.has<PoweredUp>())
				{
					GW::AUDIO::GSound explode = *e.get<GW::AUDIO::GSound>();
					explode.Play();
					
					PoweredMissiles(hit);
					e.destruct();
					game->set<PlayerEmpowered>({ true });
					
					MX::PLAY_EVENT_DATA x;
					GW::GEvent OnPlayerPickupPowerUp;
					OnPlayerPickupPowerUp.Write(MX::PLAY_EVENT::PLAYER_POWEREDUP, x);
					eventPusher.Push(OnPlayerPickupPowerUp);
					std::cout << "POWERED UP" << "\n";
				}
			}
			});
			});

	onPlayerDeath.Create([this](const GW::GEvent& e) {
		MX::PLAY_EVENT event; MX::PLAY_EVENT_DATA eventData;
		if (+e.Read(event, eventData) && eventData.playerIsDead == true)
		{
			CleanupPickups();
		}
		});
	_eventPusher.Register(onPlayerDeath);
	return true;
}

// Free any resources used to run this system
bool MX::PowerUpLogic::Shutdown()
{
	CleanupPickups();
	game->entity("PickUp System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

bool MX::PowerUpLogic::CleanupPickups()
{
	game->defer_begin();
	game->each([](flecs::entity e, Pickup&) {
		e.destruct();
		std::cout << "Pickup SUCCESSFULLY Destroyed!" << '\n';
		});
	game->defer_end();

	return true;
}

// Toggle if a system's Logic is actively running
bool MX::PowerUpLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("PickUp System").enable();
	}
	else {
		game->entity("PickUp System").disable();
	}
	return false;
}

void MX::PowerUpLogic::PoweredMissiles(flecs::entity& hit)
{
	hit.add<PoweredUp>();
	game->set<PlayerEmpowered>({ true });
}
