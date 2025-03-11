#include "BulletData.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Gameplay.h"
#include "../Components/Physics.h"
#include "Prefabs.h"

bool MX::BulletData::Load(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::AUDIO::GAudio _audioEngine)
{
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();

	float speed = (*readCfg).at("Player-Lazers").at("speed").as<float>();
	float scale = (*readCfg).at("Player-Lazers").at("lazScale").as<float>();
	int pcount = (*readCfg).at("Player-Lazers").at("projectiles").as<int>();
	float frate = (*readCfg).at("Player-Lazers").at("firerate").as<float>();
	std::string fireFX = (*readCfg).at("Player-Lazers").at("fireFX").as<std::string>();
	const char* lazerModelName = (*readCfg).at("Player-Lazers").at("model").as<const char*>();
	int dmg = (*readCfg).at("Player-Missile").at("lazDamage").as<int>();
	// looks up the name of the Entity based on the 'lazerModelName'
	auto lazerEntity = _game->lookup(lazerModelName);

	// set up the transform for the lazer
	GW::MATH::GMATRIXF world = GW::MATH::GIdentityMatrixF;
	GW::MATH::GVECTORF scaleVec = { scale, scale, scale };
	GW::MATH::GMatrix::ScaleLocalF(world, scaleVec, world);
	GW::MATH::GMatrix::RotateYLocalF(world, G_DEGREE_TO_RADIAN_F(180), world);

	// Load sound effect used by this bullet prefab
	GW::AUDIO::GSound shoot;
	shoot.Create(fireFX.c_str(), _audioEngine, 0.2f); // we need a global music & sfx volumes


	// Create prefab for lazer weapon
	auto lazerPrefab = _game->prefab().set([&, dmg, scaleVec](Transform& t, Transform::Orientation& o, Transform::Position& p,
		Transform::Scale& s, Material& m, BlenderInformation::ModelIndex& mIdx, BlenderInformation::TransformInfo& tIdx,
		BlenderInformation::ModelBoundary& col, Damage& d)
		{
			t = { world };
			s = { scaleVec };
			o = { t.worldMatrix.row1.x, t.worldMatrix.row1.y, t.worldMatrix.row1.z, t.worldMatrix.row1.w,
				  t.worldMatrix.row2.x, t.worldMatrix.row2.y, t.worldMatrix.row2.z, t.worldMatrix.row2.w,
				  t.worldMatrix.row3.x, t.worldMatrix.row3.y, t.worldMatrix.row3.z, t.worldMatrix.row3.w,
				  t.worldMatrix.row4.x, t.worldMatrix.row4.y, t.worldMatrix.row4.z, t.worldMatrix.row4.w };
			p = { 0, 0, 0, 1 };
			mIdx = { lazerEntity.get<BlenderInformation::ModelIndex>()->modelIndex };
			tIdx = { lazerEntity.get<BlenderInformation::TransformInfo>()->transformIndex };
		})
		// .set<> in a prefab means components are shared (instanced)
			.set<BlenderInformation::BlenderName>({ lazerModelName })
			.set<RigidBody::Acceleration>({ 0, 0 })
			.set<RigidBody::Velocity>({ 0, 0, -speed })
			.set<GW::AUDIO::GSound>(shoot.Relinquish())
			// .override<> ensures a component is unique to each entity created from a prefab 
			.set_override<Damage>({ dmg, 1 })
			.override<Transform>()
			.override<Transform::Position>()
			.override<Transform::Orientation>()
			.override<BlenderInformation::ModelBoundary>()
			.override<BlenderInformation::BoundaryIndex>()
			.override<Bullet>() // Tag this prefab as a bullet (for queries/systems)
			.override<Collidable>() // can be collided with
			.override<Renderable>();

		// register this prefab by name so other systems can use it
		RegisterPrefab("Lazer Bullet", lazerPrefab);

		return true;
}

bool MX::BulletData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all bullets and their prefabs
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, Bullet&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

	// unregister this prefab by name
	UnregisterPrefab("Lazer Bullet");

	return true;
}
