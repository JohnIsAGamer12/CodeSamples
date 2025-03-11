#include "PowerUpData.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Gameplay.h"
#include "../Components/Physics.h"
#include "Prefabs.h"

bool MX::PowerUpData::Load(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::AUDIO::GAudio _audioEngine)
{
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();

	// Create prefab for lazer weapon
	// color
	float red = (*readCfg).at("PowerUp-Pickups").at("red").as<float>();
	float blue = (*readCfg).at("PowerUp-Pickups").at("blue").as<float>();
	float green = (*readCfg).at("PowerUp-Pickups").at("green").as<float>();
	// Model Name
	const char* pickUpModelName = (*readCfg).at("PowerUp-Pickups").at("model").as<const char*>();
	float scale = 8;
	std::string pickupFX = (*readCfg).at("PowerUp-Pickups").at("pickupFX").as<std::string>();

	//bool pickUpEntityTest = _game->lookup(pickUpModelName);

	// default world matrix for Pick ups
	GW::MATH::GMATRIXF pickUpTransform = GW::MATH::GIdentityMatrixF;
	GW::MATH::GVECTORF scalePickUp = { scale, scale, scale };
	GW::MATH::GMatrix::ScaleLocalF(pickUpTransform, scalePickUp, pickUpTransform);

	GW::AUDIO::GSound pickup;
	pickup.Create(pickupFX.c_str(), _audioEngine, 0.2f);


	// add prefab to ECS
	auto& pickUpEntity = _game->lookup(pickUpModelName);
	auto PickupPrefab = _game->prefab()
		// .set<> in a prefab means components are shared (instanced)
		.set<Transform>({ pickUpTransform })
		.set<Transform::Scale>({ scalePickUp })
		.set<Transform::Orientation>({ pickUpTransform })
		.set<Transform::Position>({ 0, 60, 0, 1 })
		.set<Material>({ red, green, blue })
		.set<BlenderInformation::BlenderName>({ pickUpModelName })
		.set<BlenderInformation::TransformInfo>({ pickUpEntity.get<BlenderInformation::TransformInfo>()->transformIndex })
		.set<BlenderInformation::ModelIndex>({ pickUpEntity.get<BlenderInformation::ModelIndex>()->modelIndex })
		.set<GW::AUDIO::GSound>(pickup.Relinquish())
		// .override<> ensures a component is unique to each entity created from a prefab 
		.set_override<BlenderInformation::ModelBoundary>({ pickUpEntity.get<BlenderInformation::ModelBoundary>()->obb })
		.override<Transform::Position>()
		.override<Transform::Orientation>()
		.override<RigidBody::Velocity>()
		.override<RigidBody::Acceleration>()
		.override<Pickup>() // Tag this prefab as a pickup (for queries/systems)
		.override<Collidable>(); // can be collided with

	// register this prefab by name so other systems can use it
	RegisterPrefab("PowerUp Pickup", PickupPrefab);

	return true;
}

bool MX::PowerUpData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all bullets and their prefabs
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, Pickup&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

	// unregister this prefab by name
	UnregisterPrefab("Powerup Pickup");

	return true;
}
