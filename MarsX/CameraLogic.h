// Camera System Responsible for Translating the Renderer's View Matrix Relative to the Player
#ifndef CAMERALOGIC_H
#define CAMERALOGIC_H

#include "../GameConfig.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "DirectX11RendererLogic.h"

namespace MX
{
	class CameraLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		DirectX11RendererLogic* renderingSystem;

		GW::CORE::GEventGenerator eventPusher;
		// get player(s)
		flecs::query<Player, Camera, Transform::Position> playerCache;
		GW::MATH::GMatrix matrix;
		GW::MATH::GMATRIXF camera;

		GW::CORE::GEventResponder onPlayerMove;
		GW::CORE::GEventResponder onGameStart;

		float movementSpeed = 100;
		float offSet = 0;
		float maxoffSetRight = -30.0f;
		float maxoffSetLeft = +30.0f;
		float offSetIdle = 0;

		bool isFacingLeft = true;
		bool isFacingRight = false;

	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig, GW::CORE::GEventGenerator& _eventPusher, DirectX11RendererLogic* _renderingSystem);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();

	};
};

#endif