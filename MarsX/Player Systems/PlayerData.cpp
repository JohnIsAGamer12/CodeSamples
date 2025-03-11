#include "PlayerData.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "Prefabs.h"


bool MX::PlayerData::Load(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig)
{
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	// health
	int health = (*readCfg).at("Player").at("health").as<int>();
	// color
	float red = (*readCfg).at("Player").at("red").as<float>();
	float green = (*readCfg).at("Player").at("green").as<float>();
	float blue = (*readCfg).at("Player").at("blue").as<float>();
	// transform 
	float xstart = (*readCfg).at("Player").at("xstart").as<float>();
	float ystart = (*readCfg).at("Player").at("ystart").as<float>();
	float zstart = (*readCfg).at("Player").at("zstart").as<float>();
	float xangle = (*readCfg).at("Player").at("xangle").as<float>();
	float yangle = (*readCfg).at("Player").at("yangle").as<float>();
	float zangle = (*readCfg).at("Player").at("zangle").as<float>();
	float scale = (*readCfg).at("Player").at("scale").as<float>();

	// Model Name
	const char* PlayerModelName = (*readCfg).at("Player").at("model").as<const char*>();
	// player's Rocket Ambience sound
	/* std::string engineFX = (*readCfg).at("Player1").at("engineSound").as<std::string>(); */

	// Texture Map
	std::string playerTexture = (*readCfg).at("Player").at("texture").as<std::string>();

	player = new flecs::entity();
	(*player) = _game->lookup(PlayerModelName);

	// Create Player One
	player->set([&](Transform& mat, Transform::Position& p, Transform::Orientation& o, Transform::Scale& s,
		Material& m, Material::TextureColor& texel, ControllerID& c,
		Damage& d, Health& h) {
			c = { 0 };
			d = { 0 };
			h = { health };
			p = { mat.worldMatrix.row4.x, 30, mat.worldMatrix.row4.z, 1 };
			s = { scale, scale, scale };
			m = { {red, green, blue} };
			texel = { playerTexture };
			o = { GW::MATH::GIdentityMatrixF };
			GW::MATH::GMatrix::ScaleLocalF(o.value, s.value, o.value);
		})
		.set<BlenderInformation::BlenderName>({ PlayerModelName })
		.add<Player>() // Tag this entity as a Player
		.add<Camera>()
		.add<Collidable>()
		.add<Renderable>();


	return true;
}

bool MX::PlayerData::Reset(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig)
{
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	// health
	int health = (*readCfg).at("Player").at("health").as<int>();
	// color
	float red = (*readCfg).at("Player").at("red").as<float>();
	float green = (*readCfg).at("Player").at("green").as<float>();
	float blue = (*readCfg).at("Player").at("blue").as<float>();
	// transform 
	float xstart = (*readCfg).at("Player").at("xstart").as<float>();
	float ystart = (*readCfg).at("Player").at("ystart").as<float>();
	float zstart = (*readCfg).at("Player").at("zstart").as<float>();
	float xangle = (*readCfg).at("Player").at("xangle").as<float>();
	float yangle = (*readCfg).at("Player").at("yangle").as<float>();
	float zangle = (*readCfg).at("Player").at("zangle").as<float>();
	float scale = (*readCfg).at("Player").at("scale").as<float>();

	// Model Name
	const char* playerModelName = (*readCfg).at("Player").at("model").as<const char*>();
	// player's Rocket Ambience sound
	/* std::string engineFX = (*readCfg).at("Player1").at("engineSound").as<std::string>(); */

	// Texture Map
	std::string playerTexture = (*readCfg).at("Player").at("texture").as<std::string>();

	if (player != nullptr)
	{
		player->set([&](flecs::entity e, Transform& mat, Transform::Position& p, Transform::Orientation& o, Transform::Scale& s,
			Material& m, Material::TextureColor& texel, Health& h) {
				h = { health };
				p = { mat.worldMatrix.row4.x, 30, mat.worldMatrix.row4.z, 1 };
				s = { scale, scale, scale };
				m = { {red, green, blue} };
				texel = { playerTexture };
				o = { GW::MATH::GIdentityMatrixF };
				GW::MATH::GMatrix::ScaleLocalF(o.value, s.value, o.value);
			});
		_game->set<PlayerHealth>({ health });
		_game->set<PlayerScore>({ 0 });
		_game->set<PlayerEmpowered>({ false });
	}
	else
		std::cout << "Player wasnt reloaded properly" << '\n';

	return true;
}

bool MX::PlayerData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all players
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, Player&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

	return true;
}
