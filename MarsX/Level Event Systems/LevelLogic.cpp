#include <random>
#include "LevelLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Entities/Prefabs.h"
#include "../Events/GameStates.h"
#include "../Utils/Macros.h"

using namespace MX; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool MX::LevelLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig, GW::AUDIO::GAudio _audioEngine,
	GW::CORE::GEventGenerator _eventPusher,
	MX::PlayerLogic& _playerSystem, MX::BulletLogic& _bulletSystem,
	MX::EnemyLogic& _enemySystem, MX::PowerUpLogic& _pickUpSystem)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	audioEngine = _audioEngine;
	eventPusher = _eventPusher;

	bulletSystemRef = _bulletSystem;
	enemySystemRef = _enemySystem;
	pickUpSystemRef = _pickUpSystem;

	// create an asynchronus version of the world
	gameAsync = game->async_stage(); // just used for adding stuff, don't try to read data
	gameLock.Create();
	// Pull enemy Y start location from config file
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	float enemy1startX = (*readCfg).at("Enemy-Basic").at("xStart").as<float>();
	float enemy1accmax = (*readCfg).at("Enemy-Basic").at("accmax").as<float>();
	float enemy1accmin = (*readCfg).at("Enemy-Basic").at("accmin").as<float>();
	// Pull enemy Y start location from config file
	float enemy2startX = (*readCfg).at("Enemy-Light").at("xStart").as<float>();
	float enemy2accmax = (*readCfg).at("Enemy-Light").at("accmax").as<float>();
	float enemy2accmin = (*readCfg).at("Enemy-Light").at("accmin").as<float>();
	// Pull enemy Y start location from config file
	float enemy3startX = (*readCfg).at("Enemy-Heavy").at("xStart").as<float>();
	float enemy3accmax = (*readCfg).at("Enemy-Heavy").at("accmax").as<float>();
	float enemy3accmin = (*readCfg).at("Enemy-Heavy").at("accmin").as<float>();
	// getting the Enemy Standard Scale
	float scale = (*readCfg).at("Enemy-Basic").at("scale").as<float>();
	// level one info
	float enemySpawnDelay = (*readCfg).at("Level").at("enemyspawndelay").as<float>();
	float pickupSpawnDelay = (*readCfg).at("Level").at("pickupspawndelay").as<float>();
	// Level one Music
	std::string music = (*readCfg).at("Level").at("music").as<std::string>();
	// Get Player Model
	const char* playerName = (*readCfg).at("Player").at("model").as<const char*>();


	// gets Enemy-Type Model for their Colliders
	const char* EnemyTypeModel = (*readCfg).at("Enemy-Basic").at("model").as<const char*>();
	const char* PickUpModel = (*readCfg).at("PowerUp-Pickups").at("model").as<const char*>();
	auto& enemyentity = _game->lookup(EnemyTypeModel);
	auto& pickUpEntity = _game->lookup(PickUpModel);



	onPaused.Create([&](const GW::GEvent e) {
		MX::PAUSE_STATE currState; MX::PAUSE_STATE_DATA pauseData;
		if (+e.Read(currState, pauseData) && currState == MX::PAUSE_STATE::PAUSED || currState == MX::PAUSE_STATE::UNPAUSED)
		{
			isPaused = pauseData.isPaused;
		}
		});
	eventPusher.Register(onPaused);

	onGameOver.Create([&](const GW::GEvent e) {
		MX::MENU_STATE currState; MX::MENU_STATE_DATA menuData;
		if (+e.Read(currState, menuData) && currState == MENU_STATE::GAMEOVERMENU)
		{
			isPaused = menuData.gameOverMenu;
		}
		});
	eventPusher.Register(onGameOver);

	onGameStart.Create([&](const GW::GEvent e) {
		MX::MENU_STATE currState; MX::MENU_STATE_DATA menuData;
		if (+e.Read(currState, menuData) && currState == MX::MENU_STATE::MAINMENU)
		{
			isPaused = menuData.mainMenu;
		}
		});
	eventPusher.Register(onGameStart);

	// Level Manager - handles events that happen in the game
	// one event - when the player reaches a certain score they progress to the next wave
	struct GameManager {}; // local game tag for the Game Manager
	game->entity("Game-Manager").add<GameManager>();
	game->system<GameManager>().each([this, enemySpawnDelay, scale, enemy1startX, enemy1accmax,
		enemy1accmin, enemyentity, pickupSpawnDelay, playerName, pickUpEntity,
		&_playerSystem, &_enemySystem, &_bulletSystem, &_pickUpSystem](GameManager& GM)
		{
			// for every wave send them into the next wave and increase difficulty
			if (!isPaused)
			{
				if (startSpawningPickUps)
				{
					SpawnPickUps(pickupSpawnDelay, playerName, pickUpEntity, _playerSystem,
						_enemySystem, _bulletSystem, _pickUpSystem);
					startSpawningPickUps = false;
				}
				if (startSpawningEnemies)
				{
					SpawnEnemies(enemySpawnDelay, scale, enemy1startX, enemy1accmax,
						enemy1accmin, enemyentity);
					startSpawningEnemies = false;
				}
			}
			else if (isPaused)
			{
				proceduralPickupSpawingEvent = nullptr;
				randomEnemySpawningEvent = nullptr;
				startSpawningPickUps = true;
				startSpawningEnemies = true;

			}
		});





	// create a system the runs at the end of the frame only once to merge async changes
	struct LevelSystem {}; // local definition so we control iteration counts
	game->entity("Level System").add<LevelSystem>();
	// only happens once per frame at the very start of the frame
	game->system<LevelSystem>().kind(flecs::OnLoad) // first defined phase
		.each([this](flecs::entity e, LevelSystem& s) {
		// merge any waiting changes from the last frame that happened on other threads
		gameLock.LockSyncWrite();
		gameAsync.merge();
		gameLock.UnlockSyncWrite();
			});

	// Load and play level one's music
	currentTrack.Create(music.c_str(), audioEngine, 0.1f);

	currentTrack.Play();
	return true;
}
void MX::LevelLogic::SpawnEnemies(float enemySpawnDelay, float scale, float enemy1startX,
	float enemy1accmax, float enemy1accmin, const flecs::entity& enemyentity)
{
	// spins up a job in a thread pool to invoke a function at a regular interval
	randomEnemySpawningEvent.Create(enemySpawnDelay * 1000, [this, scale, enemy1startX, enemy1accmax, enemy1accmin, enemyentity]() {
		// compute random spawn location
		std::random_device rd;  // Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
		std::uniform_real_distribution<float> y_range(10, +60);
		std::uniform_real_distribution<float> z_range(-1170, +1170);
		std::uniform_real_distribution<float> a_range(enemy1accmin, enemy1accmax);
		float yStart = y_range(gen); // normal rand() doesn't work great multi-threaded
		float zStart = z_range(gen);
		float accel = a_range(gen);
		// grab enemy type 1 prefab
		flecs::entity enemyEt;
		std::vector <char*> enemytypes = {	"Basic Enemy",
											"Light Enemy",
											"Heavy Enemy" };
		srand(time(NULL));
		int randomtype = (rand() % 3);
		if (RetreivePrefab(enemytypes[randomtype], enemyEt)) {
			// you must ensure the async_stage is thread safe as it has no built-in synchronization
			gameLock.LockSyncWrite();
			// this method of using prefabs is pretty conveinent


			listofEnemies.push_back(gameAsync.entity().is_a(enemyEt).set([&, enemyentity](RigidBody::Velocity& v, RigidBody::Acceleration& a,
				Transform::Position& p, Transform::Orientation& o, Transform::Scale& s, BlenderInformation::ModelBoundary& col, BlenderInformation::BoundaryIndex& colIdx) {
					v = { 0, 0, 0 };
					s.value = { scale, scale, scale };
					GW::MATH::GMATRIXF newOrientation;
					GW::MATH::GMatrix::ScaleLocalF(GW::MATH::GIdentityMatrixF, s.value, newOrientation);
					if (zStart < 0)
					{
						a = { 0, 0, accel };
						GW::MATH::GMatrix::RotateYLocalF(newOrientation, G_DEGREE_TO_RADIAN_F(180), newOrientation);
						o = { newOrientation };
					}
					else if (zStart > 0)
					{
						a = { 0, 0, -accel };
						GW::MATH::GMatrix::RotateYLocalF(newOrientation, G_DEGREE_TO_RADIAN_F(360), newOrientation);
						o = { newOrientation };
					}
					p = { enemy1startX, yStart, zStart };



					col = { enemyentity.get<BlenderInformation::ModelBoundary>()->obb };
					colIdx = { enemyentity.get<BlenderInformation::BoundaryIndex>()->colliderIndex };

				}));
			// be sure to unlock when done so the main thread can safely merge the changes
			gameLock.UnlockSyncWrite();
		}

		}, 1000); // wait 5 seconds to start enemy wave
}
void MX::LevelLogic::SpawnPickUps(float pickupSpawnDelay, const char* playerName, const flecs::entity& pickUpEntity,
	MX::PlayerLogic& _playerSystem, MX::EnemyLogic& _enemySystem, MX::BulletLogic& _bulletSystem, MX::PowerUpLogic& _pickUpSystem)
{
	proceduralPickupSpawingEvent.Create(pickupSpawnDelay * 1000, [this, playerName, pickUpEntity, &_playerSystem, &_enemySystem, &_bulletSystem, &_pickUpSystem]() {

		bool playerEtTest = game->lookup(playerName);
		auto& playerEt = game->lookup(playerName);
		float playerPosZ = playerEt.get<Transform::Position>()->value.z;
		float zOffset = 0;
		flecs::entity pickupEt;

		if (_playerSystem.GetFacingRight())
			zOffset = 50;
		else if (_playerSystem.GetFacingLeft())
			zOffset = -50;

		if (RetreivePrefab("PowerUp Pickup", pickupEt)) {

			gameLock.LockSyncWrite();
			gameAsync.entity().is_a(pickupEt).set([&](RigidBody::Velocity& v, RigidBody::Acceleration& a,
				Transform::Position& p, BlenderInformation::ModelBoundary& col, BlenderInformation::BoundaryIndex& colIdx) {
					v = { 0, 0, 0 };
					a = { 0, -5.0f, 0 };
					p = { 0, 60, playerPosZ + zOffset };
					col = { pickUpEntity.get<BlenderInformation::ModelBoundary>()->obb };
					colIdx = { pickUpEntity.get<BlenderInformation::BoundaryIndex>()->colliderIndex };
				});

			gameLock.UnlockSyncWrite();
		}
		}, 10 * 1000);
}
// Free any resources used to run this system
bool MX::LevelLogic::Shutdown()
{
	randomEnemySpawningEvent = nullptr; // stop adding enemies
	proceduralPickupSpawingEvent = nullptr; // stop adding pickups
	pickUpSystemRef.CleanupPickups();
	enemySystemRef.CleanupEnemies();
	pickUpSystemRef.CleanupPickups();
	gameAsync.merge(); // get rid of any remaining commands
	game->entity("Level System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool MX::LevelLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Level System").enable();
	}
	else {
		game->entity("Level System").disable();
	}
	return false;
}

// **** SAMPLE OF MULTI_THREADED USE ****
//flecs::world world; // main world
//flecs::world async_stage = world.async_stage();
//
//// From thread
//lock(async_stage_lock);
//flecs::entity e = async_stage.entity().child_of(parent)...
//unlock(async_stage_lock);
//
//// From main thread, periodic
//lock(async_stage_lock);
//async_stage.merge(); // merge all commands to main world
//unlock(async_stage_lock);