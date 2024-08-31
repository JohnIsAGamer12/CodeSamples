#include "PlayerLogic.h"
#include "../Components/Visuals.h"
#include "../Entities/Prefabs.h"
#include "../Events/Playevents.h"
#include "../Events/GameStates.h"

using namespace MX; // Example Space Game
using namespace GW::INPUT; // input libs
using namespace GW::AUDIO; // audio libs

// Connects logic to traverse any players and allow a controller to manipulate them
bool MX::PlayerLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	MX::MenuController& _menuController,
	GW::AUDIO::GAudio _audioEngine,
	GW::INPUT::GInput _immediateInput,
	GW::INPUT::GController _controllerInput,
	GW::INPUT::GBufferedInput _bufferedInput,
	GW::CORE::GEventGenerator _eventPusher,
	MX::PlayerData _player)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	immediateInput = _immediateInput;
	bufferedInput = _bufferedInput;
	controllerInput = _controllerInput;
	audioEngine = _audioEngine;
	player = _player;
	// setting default current Health
	currentHealth = 0;
	currentScore = 0;

	eventPusher = _eventPusher;
	menuController = _menuController;

	game->set<PlayerScore>({ 0 });
	game->set<PlayerHealth>({ 0 });
	game->set<PlayerEmpowered>({ false });
	// Init any helper systems required for this task
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	// Grab Highscore from ini
	/*std::ifstream file("Highscore.txt");
	if (file)
	{

	}
	else
	{*/
	game->set<HighScore>({ (*readCfg).at("Scores").at("highscore").as<int>() });
	/*}*/
	int width = (*readCfg).at("Window").at("width").as<int>();
	float speed = (*readCfg).at("Player").at("speed").as<float>();
	const char* playerName = (*readCfg).at("Player").at("model").as<const char*>();
	float _bulletSize = (*readCfg).at("Player-Lazers").at("lazScale").as<float>();
	// TODO: Make it Data Driven
	float rotSpeed = (*readCfg).at("Player").at("rotSpeed").as<float>();
	// END TODO:
	float scale = (*readCfg).at("Player").at("scale").as<float>();

	// Bullet Damage Multipler Powerup Stats
	const char* lazerModelName = (*readCfg).at("Player-Lazers").at("model").as<const char*>();
	const char* modelName = "";
	float DmgMult = (*readCfg).at("PowerUp-Missiles").at("damageMultiplier").as<float>();
	int maxNumberOfMultMissiles = (*readCfg).at("PowerUp-Missiles").at("maxnumofmultmissles").as<int>();

	std::string newFireFx = "";


	bool isTakingDamage = false;

	onPaused.Create([&](const GW::GEvent e) {
		MX::PAUSE_STATE currState; MX::PAUSE_STATE_DATA menuData;
		if (+e.Read(currState, menuData) && currState == PAUSE_STATE::UNPAUSED || currState == PAUSE_STATE::PAUSED)
		{
			isPaused = menuData.isPaused;

			if (!isPaused)
				menuController.Activate(false);
			else if (isPaused)
				menuController.Activate(true);
		}
		});
	eventPusher.Register(onPaused);

	onGameOver.Create([&](const GW::GEvent e) {
		MX::MENU_STATE currState; MX::MENU_STATE_DATA menuData;
		if (+e.Read(currState, menuData) && currState == MENU_STATE::GAMEOVERMENU)
		{
			isPaused = menuData.gameOverMenu;

			if (menuData.startGamePressed)
			{
				player.Reset(game, gameConfig);
				facingRight = false;
				facingLeft = true;
				movingRight = false;
				movingLeft = true;
				currentAngle = 0;
				isDead = false;
				menuController.Activate(false);

				GW::GEvent resetScore;
				resetScore.Write(MX::PLAY_EVENT::PLAYER_DAMAGED, nullptr);
				eventPusher.Push(resetScore);

				GW::GEvent unPaused;
				MX::PAUSE_STATE_DATA pauseData;
				pauseData.isPaused = false;
				unPaused.Write(MX::PAUSE_STATE::UNPAUSED, menuData);
				eventPusher.Push(unPaused);
			}

		}
		});
	eventPusher.Register(onGameOver);

	onGameStart.Create([&](const GW::GEvent e) {
		MX::MENU_STATE currState; MX::MENU_STATE_DATA menuData;
		if (+e.Read(currState, menuData) && currState == MX::MENU_STATE::MAINMENU)
		{
			if (!isPaused)
			{
				menuController.Activate(false);
				player.Reset(game, gameConfig);
				facingRight = false;
				facingLeft = true;
				movingRight = false;
				movingLeft = true;
				currentAngle = 0;
				isDead = false;

				currentScore = 0;
				game->set<PlayerScore>({ 0 });
				game->set<PlayerHealth>({ 100 });
				GW::GEvent resetScore;
				PLAY_EVENT_DATA _resetScore;
				_resetScore.Score = 0;
				_resetScore.Health = game->get<PlayerHealth>()->health;
				resetScore.Write(MX::PLAY_EVENT::PLAYER_DAMAGED, _resetScore);
				eventPusher.Push(resetScore);

				GW::GEvent unPaused;
				MX::PAUSE_STATE_DATA pauseData;
				pauseData.isPaused = false;
				unPaused.Write(MX::PAUSE_STATE::UNPAUSED, menuData);
				eventPusher.Push(unPaused);

			}
			else
				menuController.Activate(true);
		}
		});
	eventPusher.Register(onGameStart);

	menuController.Activate(true);

	// add logic for updating playersw
	playerSystem = game->system<Player, Transform::Position, Transform::Orientation, ControllerID, Health>("Player System")
		.iter([this, speed, DmgMult, rotSpeed, playerName, _bulletSize, lazerModelName, maxNumberOfMultMissiles](flecs::iter it, Player*, Transform::Position* p,
			Transform::Orientation* o, ControllerID* c, Health* h) {
				float kbm_states[256] = { 0 };
				for (auto i : it) {
					currentHealth = h[i].value;

					MX::PLAY_EVENT_DATA x;
					//x.Score = currentscore;
					float horizontalInput = 0;
					float verticalInput = 0;
					game->set<PlayerScore>({ currentScore });
					game->set<PlayerHealth>({ currentHealth });
					//game->set<PlayerEmpowered>({ false });
					// left-right movement
					float zaxis = 0, yaxis = 0, input = 0;
					// Use the controller/keyboard to move the player around the screen	
					if (c[i].index == 0) { // enable keyboard controls for player 1
						for (size_t i = 0; i < ARRAYSIZE(kbm_states); i++)
						{
							immediateInput.GetState(G_KEY_UNKNOWN + i, kbm_states[i]);
						}
					}

					onTakingDamage.Create([this, c, i](const GW::GEvent& e) {

						MX::PLAY_EVENT event; MX::PLAY_EVENT_DATA eventData;
						if (+e.Read(event, eventData) && event == PLAY_EVENT::PLAYER_DAMAGED) {

							// only in here if event matches
							currentHealth = currentHealth - eventData.DmgValue;
							std::cout << "The Player has " << currentHealth << " HP !\n";
							game->set<Health>({ currentHealth });

							bool isConnected;
							controllerInput.IsConnected(c[i].index, isConnected);
							if (isConnected)
								controllerInput.StartVibration(c[i].index, 0, 0.3f, 0.5f);

							game->set<PlayerHealth>({ currentHealth });

							if ((currentHealth) <= 0)
							{

								std::cout << "The Player has Died!\n";
								int score = game->get<PlayerScore>()->score;
								int high = game->get<HighScore>()->highscore;
								if (score > high)
								{
									high = score;
									std::cout << "MarsX's NEW HERO! NEW HIGH SCORE!" << "\n";
									game->set<HighScore>({ score }); // New High Score Set in Entity for now
									std::cout << "SCORE: " << game->get<HighScore>()->highscore << "\n";
								}
								if (score <= high)
								{
									std::cout << "HIGH SCORE: " << game->get<HighScore>()->highscore << "\n";
									std::cout << "FINAL SCORE: " << game->get<PlayerScore>()->score << "\n";
								}
								currentScore = 0;
								game->set<PlayerScore>({ 0 });
							}
						}
						});
					eventPusher.Register(onTakingDamage);


					if (!isPaused)
					{
						if (kbm_states[G_KEY_F7] == 1)
							currentHealth = 0;


						bool controllerIsConnected;
						controllerInput.IsConnected(i, controllerIsConnected);
						float _controllerDpadLeftInput = 0, _controllerDpadRightInput = 0;
						float _controllerDpadUpInput = 0, _controllerDpadDownInput = 0;
						float _controllerLeftStickXInput = 0, _controllerLeftStickYInput = 0;


						if (controllerIsConnected)
						{
							controllerInput.GetState(c[i].index, G_LX_AXIS, _controllerLeftStickXInput);
							controllerInput.GetState(c[i].index, G_LY_AXIS, _controllerLeftStickYInput);
							if (_controllerLeftStickXInput > 0.5f)
							{
								_controllerLeftStickXInput = 1;
#if _DEBUG
								std::cout << "Left Stick X-Axis is Positve" << '\n';
#endif
							}
							else if (_controllerLeftStickXInput == -1)
							{
								_controllerLeftStickXInput = -1;
#if _DEBUG
								std::cout << "Left Stick X-Axis is Negative" << '\n';
#endif
							}

							if (_controllerLeftStickYInput > 0)
							{
								_controllerLeftStickYInput = 1;
#if _DEBUG
								std::cout << "Right Stick Y-Axis is Positve" << '\n';
#endif
							}
							else if (_controllerLeftStickYInput < 0)
							{
								_controllerLeftStickYInput = -1;
#if _DEBUG
								std::cout << "Right Stick Y-Axis is Negative" << '\n';
#endif // _DEBUG

							}



							controllerInput.GetState(c[i].index, G_DPAD_LEFT_BTN, _controllerDpadLeftInput);
							if (_controllerDpadLeftInput == 1)
							{
#if _DEBUG
								std::cout << "DPAD LEFT IS PRESSED" << '\n';
#endif // _DEBUG
								_controllerDpadLeftInput = -1;
							}

							controllerInput.GetState(c[i].index, G_DPAD_RIGHT_BTN, _controllerDpadRightInput);
#if _DEBUG
							if (_controllerDpadRightInput == 1)
								std::cout << "DPAD RIGHT IS PRESSED" << '\n';
#endif // _DEBUG
							controllerInput.GetState(c[i].index, G_DPAD_UP_BTN, _controllerDpadUpInput);


							controllerInput.GetState(c[i].index, G_DPAD_DOWN_BTN, _controllerDpadDownInput);
							if (_controllerDpadDownInput == 1)
								_controllerDpadDownInput = -1;

						}
						zaxis = G_LARGER(zaxis, -1);// cap right motion
						zaxis = G_SMALLER(zaxis, 1);// cap left motion

						// apply movement
						verticalInput = (kbm_states[G_KEY_W] - kbm_states[G_KEY_S]) + (_controllerDpadUpInput + _controllerDpadDownInput) + (_controllerLeftStickYInput);
						horizontalInput = (kbm_states[G_KEY_D] - kbm_states[G_KEY_A]) + (_controllerDpadRightInput + _controllerDpadLeftInput) + (_controllerLeftStickXInput);


						if (!(verticalInput == 0 && horizontalInput == 0))
						{
							if (!movingRight || !movingLeft)
							{
								if (kbm_states[G_KEY_A] == 1 || _controllerDpadLeftInput == -1 || _controllerLeftStickXInput == -1)
								{
									movingLeft = true;
									facingLeft = true;
									facingRight = false;

									GW::GEvent movingLeft;
									MX::GAMEPLAY_STATE_DATA gameData;
									gameData.facingLeft = true;
									gameData.facingRight = false;
									movingLeft.Write(GAMEPLAY_STATE::PLAYERMOVINGLEFT, gameData);
									eventPusher.Push(movingLeft);
								}
								else if (kbm_states[G_KEY_D] == 1 || _controllerDpadRightInput == 1 || _controllerLeftStickXInput == 1)
								{
									movingRight = true;
									facingRight = true;
									facingLeft = false;
									GW::GEvent movingRight;
									MX::GAMEPLAY_STATE_DATA gameData;
									gameData.facingLeft = false;
									gameData.facingRight = true;
									movingRight.Write(GAMEPLAY_STATE::PLAYERMOVINGRIGHT, gameData);
									eventPusher.Push(movingRight);

								}
							}
						}

						if (movingRight)
						{
							if (currentAngle < standardRightOrient)
							{
								GW::MATH::GMATRIXF newOrientation;
								GW::MATH::GMatrix::RotateYLocalF(it.entity(i).get<Transform::Orientation>()->value, rotSpeed * it.delta_time(), newOrientation);
								it.entity(i).set<Transform::Orientation>({ newOrientation });
								currentAngle += rotSpeed * it.delta_time();
							}
							else
								movingRight = false;
						}
						else if (movingLeft)
						{
							if (currentAngle > standardLeftOrient)
							{
								GW::MATH::GMATRIXF newOrientation;
								GW::MATH::GMatrix::RotateYLocalF(it.entity(i).get<Transform::Orientation>()->value, -rotSpeed * it.delta_time(), newOrientation);
								it.entity(i).set<Transform::Orientation>({ newOrientation });
								currentAngle -= rotSpeed * it.delta_time();
							}
							else
								movingLeft = false;
						}


						p[i].value.y += verticalInput * it.delta_time() * speed;
						p[i].value.z += horizontalInput * it.delta_time() * speed;
						// limit the player to stay within the Playable Area
						p[i].value.z = G_LARGER(p[i].value.z, -1179.0f);
						p[i].value.z = G_SMALLER(p[i].value.z, +1179.0f);
						p[i].value.y = G_LARGER(p[i].value.y, 6.0f);
						p[i].value.y = G_SMALLER(p[i].value.y, +70.0f);

						// fire weapon if they are in a firing state
						if (it.entity(i).has<Firing>()) {
							Transform::Position offset = p[i];

							auto lazerLookup = game->lookup(lazerModelName);

							FireLasers(playerName, lazerLookup.get<BlenderInformation::ModelBoundary>()->obb, _bulletSize, it.world(), i, offset);
							if (it.entity(i).has<PoweredUp>())
							{
								if (curNumberOfMultMissiles < maxNumberOfMultMissiles)
								{
									curNumberOfMultMissiles += 1;
									std::cout << curNumberOfMultMissiles << '\n';
								}
								else if (curNumberOfMultMissiles >= maxNumberOfMultMissiles)
								{
									curNumberOfMultMissiles = 0;
									it.entity(i).remove<PoweredUp>();

									game->set<PlayerEmpowered>({ false });
									MX::PLAY_EVENT_DATA x;
									GW::GEvent OnPlayerPowerDown;
									OnPlayerPowerDown.Write(MX::PLAY_EVENT::PLAYER_POWEREDUP, x);
									eventPusher.Push(OnPlayerPowerDown);
									std::cout << "POWERED DOWN" << "\n";
								}
							}
							it.entity(i).remove<Firing>();
						}

						//DED
						if (currentHealth <= 0)
						{
							// Game Over Reinitalize the Player
							isDead = true;
							player.Reset(game, gameConfig);
							facingRight = false;
							facingLeft = true;
							movingRight = false;
							movingLeft = true;
							currentAngle = 0;

							MX::PLAY_EVENT_DATA x;
							GW::GEvent playerHasDied;
							x.playerIsDead = isDead;
							x.Score = currentScore;
							x.ScrValue = 0;
							playerHasDied.Write(MX::PLAY_EVENT::PLAYER_DESTROYED, x);
							eventPusher.Push(playerHasDied);
							playerHasDied.Write(MX::PLAY_EVENT::PLAYER_RESET, x);
							eventPusher.Push(playerHasDied);
							GW::GEvent gameOver;
							MX::MENU_STATE_DATA menuData;
							menuData.gameOverMenu = true;
							menuData.startGameHighlighted = true;
							menuData.startGamePressed = false;
							menuData.exitGameHighlighted = false;
							menuData.exitGamePressed = false;
							gameOver.Write(MX::MENU_STATE::GAMEOVERMENU, menuData);
							eventPusher.Push(gameOver);
							menuController.Activate(true);

							GW::GEvent unPaused;
							MX::PAUSE_STATE_DATA pauseData;
							pauseData.isPaused = true;
							unPaused.Write(MX::PAUSE_STATE::PAUSED, pauseData);
							eventPusher.Push(unPaused);
						}
					}

					if (!isPaused)
					{
						// process any cached button events after the loop (happens multiple times per frame)
						ProcessInputEvents(playerName, it.world(), kbm_states);
					}
				}
			});


	// Create an event cache for when the spacebar/'A' button is pressed
	pressEvents.Create(Max_Frame_Events); // even 32 is probably overkill for one frame

	// register for keyboard and controller events
	bufferedInput.Register(pressEvents);
	controllerInput.Register(pressEvents);

	onDestroyEnemy.Create([this](const GW::GEvent& e) {
		MX::PLAY_EVENT event; MX::PLAY_EVENT_DATA eventData;
		if (+e.Read(event, eventData) && (event == MX::PLAY_EVENT::ENEMY_DESTROYED)) {
			// only in here if event matches
			if (currentHealth > 0)
			{
				currentScore += eventData.ScrValue;
				game->set<PlayerScore>({ currentScore });
				std::cout << "SCORE: " << game->get<PlayerScore>()->score << "\n";
			}
		}
		});
	_eventPusher.Register(onDestroyEnemy);

	return true;
}

// Free any resources used to run this system
bool MX::PlayerLogic::Shutdown()
{
	playerSystem.destruct();
	game.reset();
	gameConfig.reset();

	return true;
}

bool MX::PlayerLogic::GetFacingRight()
{
	return facingRight;
}

void MX::PlayerLogic::SetFacingRight(bool _facingRight)
{
	facingRight = _facingRight;
}

bool MX::PlayerLogic::GetFacingLeft()
{
	return facingLeft;
}

void MX::PlayerLogic::SetFacingLeft(bool _facingLeft)
{
	facingLeft = _facingLeft;
}

bool MX::PlayerLogic::GetIsDead()
{
	return isDead;
}

void MX::PlayerLogic::SetIsDead(bool _isDead)
{
	isDead = _isDead;
}

// Toggle if a system's Logic is actively running
bool MX::PlayerLogic::Activate(bool runSystem)
{
	if (playerSystem.is_alive()) {
		(runSystem) ?
			playerSystem.enable()
			: playerSystem.disable();
		return true;
	}
	return false;
}

bool MX::PlayerLogic::ProcessInputEvents(const char* playerName, flecs::world& stage, float kbs_state[256])
{
	// pull any waiting events from the event cache and process them
	GW::GEvent event;
	while (+pressEvents.Pop(event)) {
		bool fire = false;
		GController::Events controller;
		GController::EVENT_DATA c_data;
		GBufferedInput::Events keyboard;
		GBufferedInput::EVENT_DATA k_data;
		// these will only happen when needed
		if (+event.Read(keyboard, k_data)) {
			if (keyboard == GBufferedInput::Events::KEYPRESSED)
			{
				if (k_data.data == G_KEY_SPACE) {
					fire = true;
				}
			}
			if (keyboard == GBufferedInput::Events::KEYRELEASED) {

				if (k_data.data == G_KEY_F8)
				{
					GW::GEvent debugMde;
					MX::GAMEPLAY_STATE_DATA gameData;
					gameData.debugMode = !debugMode;
					debugMode = gameData.debugMode;
					debugMde.Write(MX::GAMEPLAY_STATE::DEBUGMODE, gameData);
					eventPusher.Push(debugMde);
				}


				if (k_data.data == G_KEY_ESCAPE)
				{
					isPaused = true;
					GW::GEvent paused;
					MX::PAUSE_STATE_DATA pauseData;
					pauseData.isPaused = isPaused;
					paused.Write(MX::PAUSE_STATE::PAUSED, pauseData);
					eventPusher.Push(paused);
					menuController.Activate(true);

					MX::MENU_STATE_DATA menuData;
					menuData.pauseMenu = isPaused;
					menuData.startGameHighlighted = true;
					menuData.startGamePressed = false;
					menuData.exitGameHighlighted = false;
					menuData.exitGamePressed = false;
					paused.Write(MX::MENU_STATE::PAUSEMENU, menuData);
					eventPusher.Push(paused);

				}

				if (k_data.data == G_KEY_SPACE) {
					fire = false;
				}
			}
		}
		else if (+event.Read(controller, c_data)) {
			if (controller == GController::Events::CONTROLLERBUTTONVALUECHANGED) {

				if (c_data.inputValue > 0 && c_data.inputCode == G_START_BTN)
				{
					isPaused = true;
					menuController.Activate(true);
					GW::GEvent paused;
					MX::PAUSE_STATE_DATA pauseData;
					pauseData.isPaused = isPaused;
					paused.Write(MX::PAUSE_STATE::PAUSED, pauseData);
					eventPusher.Push(paused);

					MX::MENU_STATE_DATA menuData;
					menuData.pauseMenu = isPaused;
					menuData.startGameHighlighted = true;
					menuData.startGamePressed = false;
					menuData.exitGameHighlighted = false;
					menuData.exitGamePressed = false;
					paused.Write(MX::MENU_STATE::PAUSEMENU, menuData);
					eventPusher.Push(paused);

				}

				if (c_data.inputValue > 0 && c_data.inputCode == G_SOUTH_BTN)
				{
					controllerInput.StartVibration(stage.entity(playerName).get<ControllerID>()->index, 0, 0.5f, 0.3f);
					fire = true;
				}
				else if (c_data.inputValue <= 0 && c_data.inputCode == G_SOUTH_BTN)
					fire = false;
			}
		}

		if (fire) {
			if (!stage.entity(playerName).has<Firing>())
				stage.entity(playerName).add<Firing>();
		}
	}

	return true;
}

// play sound and launch two laser rounds
bool MX::PlayerLogic::FireLasers(const char* playerName, const GW::MATH::GOBBF bulletCol, const float& bulletSize, flecs::world& stage, size_t _it, Transform::Position origin)
{
	float bulletSpeed = 0;

	// Grab the prefab for a laser round
	flecs::entity bullet;
	bool res = RetreivePrefab("Lazer Bullet", bullet);
	GW::MATH::GVECTORF scaleVec = { bulletSize, bulletSize, bulletSize };
	GW::MATH::GMATRIXF newOrientation;
	GW::MATH::GMatrix::ScaleLocalF(GW::MATH::GIdentityMatrixF, scaleVec, newOrientation);

	// bullet travels depending on the player's direction it's facing
	if (facingLeft)
	{
		bulletSpeed = 1 * bullet.get<RigidBody::Velocity>()->value.z;
		GW::MATH::GMatrix::RotateYLocalF(newOrientation, G_DEGREE_TO_RADIAN_F(180), newOrientation);
		bullet.set<Transform::Orientation>({ newOrientation });
		origin.value.z += -10;
	}
	else if (facingRight)
	{
		bulletSpeed = -1 * bullet.get<RigidBody::Velocity>()->value.z;
		GW::MATH::GMatrix::RotateYLocalF(newOrientation, G_DEGREE_TO_RADIAN_F(0), newOrientation);
		bullet.set<Transform::Orientation>({ newOrientation });
		origin.value.z += 10;
	}

	auto laserInstant = stage.entity().is_a(bullet).set([&](Transform::Position& p, RigidBody::Velocity& v, Damage& d, BlenderInformation::ModelBoundary& col) {
		p = { origin };
		v = { 0, 0, bulletSpeed };
		if (stage.entity(playerName).has<PoweredUp>())
		{
			if (bullet.get<Damage>()->dmgValue != d.dmgValue * 2)
				d = { bullet.get<Damage>()->dmgValue * 2, 2 };
		}
		else if (!stage.entity(playerName).has<PoweredUp>())
			d = { bullet.get<Damage>()->dmgValue , 0 };

		col = { bulletCol };

		});
	laserInstant.add<FiredBy>(stage.entity(playerName));


	// play the sound of the Lazer prefab
	GW::AUDIO::GSound shoot = *bullet.get<GW::AUDIO::GSound>();
	shoot.Play();

	return true;
}

