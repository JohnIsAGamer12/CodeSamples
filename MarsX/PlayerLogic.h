// The player system is responsible for allowing control over the main ship(s)
#ifndef PLAYERLOGIC_H
#define PLAYERLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Systems/MenuController.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Entities/PlayerData.h"

using namespace std::chrono;

// example space game (avoid name collisions)
namespace MX 
{
	class PlayerLogic 
	{
		// keeping track of the player's currentHealth
		int currentHealth;
		int currentScore;
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to our running ECS system
		flecs::system playerSystem;
		// permananent handles input systems
		GW::INPUT::GInput immediateInput;
		GW::INPUT::GBufferedInput bufferedInput;
		GW::INPUT::GController controllerInput;
		// permananent handle to audio system
		GW::AUDIO::GAudio audioEngine;
		// key press event cache (saves input events)
		// we choose cache over responder here for better ECS compatibility
		GW::CORE::GEventCache pressEvents;
		GW::CORE::GEventGenerator eventPusher;

		bool debugMode = false;
		bool isPaused = true;
		// variables use for rotating the player's current direction
		float standardRightOrient = G_DEGREE_TO_RADIAN_F(180);
		float standardLeftOrient = 0;
		float currentAngle = 0;
		bool movingRight = false;
		bool movingLeft = true;
		//variables for determinign the direction the bullets will be facing
		bool facingRight = false;
		bool facingLeft = true;

		int curNumberOfMultMissiles = 0;

		MX::PlayerData player = {};
		MX::MenuController menuController = {};

		// event responder
		GW::CORE::GEventResponder onTakingDamage;
		GW::CORE::GEventResponder onDestroyEnemy;

		GW::CORE::GEventResponder onPaused;
		GW::CORE::GEventResponder onGameOver;
		GW::CORE::GEventResponder onGameStart;
	public:
		bool isDead = false;

		//GAME STATE LOGIC

		// attach the required logic to the ECS 
		bool Init(	std::shared_ptr<flecs::world> _game,
					std::weak_ptr<const GameConfig> _gameConfig,
					MX::MenuController& _menuController,
					GW::AUDIO::GAudio _audioEngine,
					GW::INPUT::GInput _immediateInput,
					GW::INPUT::GController _controllerInput,
					GW::INPUT::GBufferedInput _bufferedInput,
					GW::CORE::GEventGenerator _eventPusher,
					MX::PlayerData _player);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown(); 

		//Getters and Setters
		bool GetFacingRight();
		void SetFacingRight(bool _facingRight);
		bool GetFacingLeft();
		void SetFacingLeft(bool _facingLeft);
		bool GetIsDead();
		void SetIsDead(bool _isDead);
	private:
		// how big the input cache can be each frame
		static constexpr unsigned int Max_Frame_Events = 32;
		// helper routines
		bool ProcessInputEvents(const char* playerName, flecs::world& stage, float kbs_state[256]);
		bool FireLasers(const char* playerName, const GW::MATH::GOBBF bulletCol, const float& bulletSize, flecs::world& stage, size_t _it, MX::Transform::Position origin);
	};
};

#endif