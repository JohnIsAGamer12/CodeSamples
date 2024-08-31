#include "CameraLogic.h"
#include "../Events/GameStates.h"

using namespace MX; // Example Space Game

bool MX::CameraLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig, GW::CORE::GEventGenerator& _eventPusher, DirectX11RendererLogic* _renderingSystem)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	renderingSystem = _renderingSystem;
	eventPusher = _eventPusher;
	// create view matrix and invert for calculations
	matrix.Create();
	GW::MATH::GVECTORF eye = { 90, 30, 0 };
	GW::MATH::GVECTORF at = { 0, 40, 0 };
	GW::MATH::GVECTORF up = { 0, 1, 0 };
	matrix.LookAtLHF(eye, at, up, camera);
	GW::MATH::GMatrix::RotateYLocalF(camera, G_RADIAN_TO_DEGREE_F(90), camera);

	onGameStart.Create([&](const GW::GEvent& e)
		{
			MX::MENU_STATE menuState; MX::MENU_STATE_DATA menuData;
			if (+e.Read(menuState, menuData) && menuState == MX::MENU_STATE::MAINMENU || 
												menuState == MX::MENU_STATE::GAMEOVERMENU)
			{
				isFacingLeft = true;
				isFacingRight = false;
			}
		});
	eventPusher.Register(onGameStart);

	onPlayerMove.Create([&](const GW::GEvent& e) 
		{
			MX::GAMEPLAY_STATE gameState; MX::GAMEPLAY_STATE_DATA gameData;
			if (+e.Read(gameState, gameData) && gameState == GAMEPLAY_STATE::PLAYERMOVINGLEFT)
			{
				isFacingLeft = gameData.facingLeft;
			}
			else if (+e.Read(gameState, gameData) && gameState == GAMEPLAY_STATE::PLAYERMOVINGRIGHT)
			{
				isFacingRight = gameData.facingRight;
			}
		});
	eventPusher.Register(onPlayerMove);

	// add camera entity to world for updating
	game->entity("Camera")
		.add<Camera>()
		.set_override<Transform::Orientation>({ camera })
		.set_override<Transform::Position>({ camera.row4.x,camera.row4.y,camera.row4.z,camera.row4.w });

	// query for players
	playerCache = game->query<Player, Camera, Transform::Position>();

	game->system<Player, Camera, Transform::Orientation, Transform::Position>("Camera System")
		.each([this](flecs::entity e, Player, Camera, Transform::Orientation& o, Transform::Position& t)
			{
				// set camera entity's transform z = player z position
				playerCache.each([this, &o](flecs::entity e, Player, Camera, Transform::Position& pt) {
					// Hardcoded the Y-Position so the player doesn't see the void beneath the floor
					camera.row4.y = 30;
					
					if (isFacingLeft)
					{
						if (offSet < maxoffSetLeft)
						{
							offSet += movementSpeed * game->delta_time();
						}
						else
							isFacingLeft = false;
					}
					else if (isFacingRight)
					{
						if (offSet > maxoffSetRight)
						{
							offSet -= movementSpeed * game->delta_time();
						}
						else
							isFacingRight = false;
					}


					camera.row4.z = pt.value.z - offSet;
					});

				// pass translated camera matrix into Renderer
				renderingSystem->UpdateCamera(camera.row4);
			});

	return true;
}

// Free any resources used to run this system
bool MX::CameraLogic::Shutdown()
{
	game->entity("Camera System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool MX::CameraLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Camera System").enable();
	}
	else {
		game->entity("Camera System").disable();
	}
	return false;
}