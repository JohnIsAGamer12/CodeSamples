#include "MenuController.h"
#include "../Events/GameStates.h"
#include "../Components/Identification.h"

bool MX::MenuController::Init(std::shared_ptr<flecs::world> _game,
	GW::INPUT::GInput _immediateInput,
	GW::INPUT::GController _controllerInput,
	GW::INPUT::GBufferedInput _bufferedInput,
	GW::CORE::GEventGenerator _eventPusher)
{
	game = _game;
	eventPusher = _eventPusher;

	immediateInput = _immediateInput;
	bufferedInput = _bufferedInput;
	controllerInput = _controllerInput;

	std::string pauseMenuBtn[2]
	{
		"Resume",
		"Quit"
	};

	std::string mainMenuBtn[2]
	{
		"Play",
		"Credits"
	};

	std::string gameOverBtn[2]
	{
		"Retry",
		"Quit"
	};

	// event responders from the user controller
	gameOverHandler.Create([&](const GW::GEvent& e)
		{
			MX::MENU_STATE currMenuState; MX::MENU_STATE_DATA menuData;
			if (+e.Read(currMenuState, menuData) && currMenuState == MENU_STATE::GAMEOVERMENU)
			{
				gameOverMenu = menuData.gameOverMenu;

				retrySelected = menuData.startGameHighlighted;
				quitSelected = menuData.exitGameHighlighted;
				if (retrySelected == true)
				{
					// get the button that enters into the new state
					retryPressed = menuData.startGamePressed;
					if (retryPressed)
					{
						gameOverMenu = false;
						GW::GEvent unPaused;
						MX::PAUSE_STATE_DATA pauseData;
						pauseData.isPaused = false;
						unPaused.Write(MX::PAUSE_STATE::UNPAUSED, pauseData);
						eventPusher.Push(unPaused);
					}
				}
				else if (quitSelected == true)
				{
					quitPressed = menuData.exitGamePressed;
					if (quitPressed)
					{
						mainMenu = true;
						gameOverMenu = false;
						GW::GEvent playGame;
						MX::MENU_STATE_DATA menuData;
						menuData.mainMenu = true;
						menuData.startGameHighlighted = true;
						menuData.startGamePressed = false;
						menuData.creditsHighlighted = false;
						menuData.creditsPressed = false;
						playGame.Write(MX::MENU_STATE::MAINMENU, menuData);
						eventPusher.Push(playGame);

						GW::GEvent unPaused;
						MX::PAUSE_STATE_DATA pauseData;
						pauseData.isPaused = true;
						unPaused.Write(MX::PAUSE_STATE::PAUSED, pauseData);
						eventPusher.Push(unPaused);
					}
				}
			}
		});
	eventPusher.Register(gameOverHandler);

	mainMenuHandler.Create([&](const GW::GEvent& e)
		{
			MX::MENU_STATE currMenuState; MX::MENU_STATE_DATA menuData;
			if (+e.Read(currMenuState, menuData) && currMenuState == MENU_STATE::MAINMENU)
			{
				mainMenu = menuData.mainMenu;
				credits = menuData.credits;

				startSelected = menuData.startGameHighlighted;
				creditsHighlighted = menuData.creditsHighlighted;
				if (startSelected == true)
				{
					startPressed = menuData.startGamePressed;
					if (startPressed)
					{
						GW::GEvent unPaused;
						MX::PAUSE_STATE_DATA pauseData;
						pauseData.isPaused = false;
						unPaused.Write(MX::PAUSE_STATE::UNPAUSED, pauseData);
						eventPusher.Push(unPaused);
					}
				}
				else if (creditsHighlighted == true)
				{
					creditsPressed = menuData.creditsPressed;
					if (creditsPressed)
					{
						credits = true;
					}
				}
			}
		});
	eventPusher.Register(mainMenuHandler);

	pauseMenuHandler.Create([&](const GW::GEvent& e)
		{
			MX::MENU_STATE currMenuState; MX::MENU_STATE_DATA menuData;
			if (+e.Read(currMenuState, menuData) && currMenuState == MENU_STATE::PAUSEMENU)
			{
				pauseMenu = menuData.pauseMenu;

				resumeSelected = menuData.startGameHighlighted;
				quitSelected = menuData.exitGameHighlighted;
				if (resumeSelected == true)
				{
					// get the button that enters into the new state
					resumePressed = menuData.startGamePressed;
					if (resumePressed)
					{
						GW::GEvent unPaused;
						MX::PAUSE_STATE_DATA pauseData;
						pauseData.isPaused = false;
						unPaused.Write(MX::PAUSE_STATE::UNPAUSED, pauseData);
						eventPusher.Push(unPaused);
					}

				}
				else if (quitSelected == true)
				{
					quitPressed = menuData.exitGamePressed;
					if (quitPressed)
					{
						mainMenu = true;
						pauseMenu = false;
						GW::GEvent playGame;
						MX::MENU_STATE_DATA menuData;
						menuData.mainMenu = true;
						menuData.startGameHighlighted = true;
						menuData.startGamePressed = false;
						menuData.creditsHighlighted = false;
						menuData.creditsPressed = false;
						playGame.Write(MX::MENU_STATE::MAINMENU, menuData);
						eventPusher.Push(playGame);

						GW::GEvent unPaused;
						MX::PAUSE_STATE_DATA pauseData;
						pauseData.isPaused = true;
						unPaused.Write(MX::PAUSE_STATE::PAUSED, pauseData);
						eventPusher.Push(unPaused);
					}
				}
			}
		});
	eventPusher.Register(pauseMenuHandler);

	pressEvents.Create(Max_Frame_Events);

	bufferedInput.Register(pressEvents);
	controllerInput.Register(pressEvents);

	struct MainMenuController {}; // local definition for main menu 
	struct GameOverMenuController {};
	struct PauseMenuController {};

	game->entity("MainMenu-Controller").add<MainMenuController>().set<ControllerID>({ 0 });
	game->entity("PauseMenu-Controller").add<PauseMenuController>().set<ControllerID>({ 0 });
	game->entity("GameOverMenu-Controller").add<GameOverMenuController>().set<ControllerID>({ 0 });

	game->system<PauseMenuController, ControllerID>().each([this, pauseMenuBtn](PauseMenuController& pM, ControllerID& c)
		{
			if (pauseMenu)
			{
				GW::GEvent event;
				while (+pressEvents.Pop(event))
				{
					GW::INPUT::GController::Events controller;
					GW::INPUT::GController::EVENT_DATA c_data;
					GW::INPUT::GBufferedInput::Events keyboard;
					GW::INPUT::GBufferedInput::EVENT_DATA k_data;

					if (+event.Read(keyboard, k_data))
					{
						if (keyboard == GW::INPUT::GBufferedInput::Events::KEYPRESSED)
						{
							if (k_data.data == G_KEY_W)
							{
								iter -= 1;
								if (iter < 0)
									iter = 0;
							}
							else if (k_data.data == G_KEY_S)
							{
								iter += 1;
								if (iter > ARRAYSIZE(pauseMenuBtn) - 1)
									iter = ARRAYSIZE(pauseMenuBtn) - 1;
							}

							if (k_data.data == G_KEY_SPACE)
							{
								controllerInput.StartVibration(c.index, -1, 0.1f, 0.3f);
								pressed = true;
							}
						}
					}
					else if (+event.Read(controller, c_data))
					{
						if (controller == GW::INPUT::GController::Events::CONTROLLERBUTTONVALUECHANGED)
						{
							if (c_data.inputValue > 0 && c_data.inputCode == G_DPAD_UP_BTN)
							{
								iter -= 1;
								if (iter < 0)
								{
									iter = 0;
									controllerInput.StartVibration(c.index, 0, 0.5f, 0.1f);
								}
								else
									controllerInput.StartVibration(c.index, +1, 0.1f, 0.1f);
							}
							else if (c_data.inputValue > 0 && c_data.inputCode == G_DPAD_DOWN_BTN)
							{
								iter += 1;
								if (iter > ARRAYSIZE(pauseMenuBtn) - 1)
								{
									iter = ARRAYSIZE(pauseMenuBtn) - 1;
									controllerInput.StartVibration(c.index, 0, 0.5f, 0.1f);
								}
								else
									controllerInput.StartVibration(c.index, -1, 0.1f, 0.1f);
							}

							if (c_data.inputValue > 0 && c_data.inputCode == G_SOUTH_BTN)
							{
								controllerInput.StartVibration(c.index, -1, 0.1f, 0.3f);
								pressed = true;
							}
						}
					}
				}

				if (pauseMenuBtn[iter] == "Resume")
				{
					// set playHighlight to true and send the event over
					// check if we have click Space to enter the new Play state
					GW::GEvent resume;
					MX::MENU_STATE_DATA menuData;
					menuData.pauseMenu = pauseMenu;
					menuData.startGameHighlighted = true;
					menuData.startGamePressed = false;
					menuData.exitGameHighlighted = false;
					menuData.exitGamePressed = false;
					resume.Write(MX::MENU_STATE::PAUSEMENU, menuData);
					eventPusher.Push(resume);
					if (pressed == true)
					{
						pauseMenu = false;
						GW::GEvent playGame;
						MX::MENU_STATE_DATA menuData;
						menuData.pauseMenu = pauseMenu;
						menuData.startGameHighlighted = true;
						menuData.startGamePressed = pressed;
						menuData.exitGameHighlighted = false;
						menuData.exitGamePressed = false;
						playGame.Write(MX::MENU_STATE::PAUSEMENU, menuData);
						eventPusher.Push(playGame);
						pressed = false;
					}
				}
				else if (pauseMenuBtn[iter] == "Quit")
				{
					// set quitHighlight to true and send the event over
					// check if we have click Space to enter the new Play state
					GW::GEvent quitSelected;
					MX::MENU_STATE_DATA menuData;
					menuData.pauseMenu = pauseMenu;
					menuData.mainMenu = mainMenu;
					menuData.startGameHighlighted = false;
					menuData.startGamePressed = false;
					menuData.exitGameHighlighted = true;
					menuData.exitGamePressed = false;
					quitSelected.Write(MX::MENU_STATE::PAUSEMENU, menuData);
					eventPusher.Push(quitSelected);
					if (pressed == true)
					{
						pauseMenu = false;
						mainMenu = true;
						GW::GEvent returnToMainMenu;
						MX::MENU_STATE_DATA menuData;
						menuData.mainMenu = mainMenu;
						menuData.startGameHighlighted = false;
						menuData.startGamePressed = false;
						menuData.exitGameHighlighted = true;
						menuData.exitGamePressed = pressed;
						iter = -1;
						returnToMainMenu.Write(MX::MENU_STATE::PAUSEMENU, menuData);
						eventPusher.Push(returnToMainMenu);
						pressed = false;
					}
				}
			}
		});

	game->system<GameOverMenuController, ControllerID>().each([this, gameOverBtn](GameOverMenuController& gm, ControllerID& c) {

		if (gameOverMenu)
		{
			GW::GEvent event;
			while (+pressEvents.Pop(event))
			{
				GW::INPUT::GController::Events controller;
				GW::INPUT::GController::EVENT_DATA c_data;
				GW::INPUT::GBufferedInput::Events keyboard;
				GW::INPUT::GBufferedInput::EVENT_DATA k_data;

				if (+event.Read(keyboard, k_data))
				{
					if (keyboard == GW::INPUT::GBufferedInput::Events::KEYPRESSED)
					{
						if (k_data.data == G_KEY_W)
						{
							iter -= 1;
							if (iter < 0)
								iter = 0;
						}
						else if (k_data.data == G_KEY_S)
						{
							iter += 1;
							if (iter > ARRAYSIZE(gameOverBtn) - 1)
								iter = ARRAYSIZE(gameOverBtn) - 1;
						}

						if (k_data.data == G_KEY_SPACE)
						{
							pressed = true;
							controllerInput.StartVibration(c.index, -1, 0.1f, 0.3f);
						}
					}
				}
				else if (+event.Read(controller, c_data))
				{
					if (controller == GW::INPUT::GController::Events::CONTROLLERBUTTONVALUECHANGED)
					{
						if (c_data.inputValue > 0 && c_data.inputCode == G_DPAD_UP_BTN)
						{
							iter -= 1;
							if (iter < 0)
							{
								iter = 0;
								controllerInput.StartVibration(c.index, 0, 0.5f, 0.1f);
							}
							else
								controllerInput.StartVibration(c.index, +1, 0.1f, 0.1f);
						}
						else if (c_data.inputValue > 0 && c_data.inputCode == G_DPAD_DOWN_BTN)
						{
							iter += 1;
							if (iter > ARRAYSIZE(gameOverBtn) - 1)
							{
								iter = ARRAYSIZE(gameOverBtn) - 1;
								controllerInput.StartVibration(c.index, 0, 0.5f, 0.1f);
							}
							else
								controllerInput.StartVibration(c.index, -1, 0.1f, 0.1f);

							if (c_data.inputValue > 0 && c_data.inputCode == G_SOUTH_BTN)
							{
								pressed = true;
								controllerInput.StartVibration(c.index, -1, 0.1f, 0.3f);

							}
						}
					}
				}
			}

			if (gameOverBtn[iter] == "Retry")
			{
				// set playHighlight to true and send the event over
				// check if we have click Space to enter the new Play state
				GW::GEvent resume;
				MX::MENU_STATE_DATA menuData;
				menuData.gameOverMenu = gameOverMenu;
				menuData.startGameHighlighted = true;
				menuData.startGamePressed = false;
				menuData.exitGameHighlighted = false;
				menuData.exitGamePressed = false;
				resume.Write(MX::MENU_STATE::GAMEOVERMENU, menuData);
				eventPusher.Push(resume);
				if (pressed == true)
				{
					gameOverMenu = false;
					GW::GEvent playGame;
					MX::MENU_STATE_DATA menuData;
					menuData.gameOverMenu = gameOverMenu;
					menuData.startGameHighlighted = true;
					menuData.startGamePressed = pressed;
					menuData.exitGameHighlighted = false;
					menuData.exitGamePressed = false;
					playGame.Write(MX::MENU_STATE::GAMEOVERMENU, menuData);
					eventPusher.Push(playGame);

					GW::GEvent unPaused;
					MX::PAUSE_STATE_DATA pauseData;
					pauseData.isPaused = false;
					unPaused.Write(MX::PAUSE_STATE::UNPAUSED, pauseData);
					eventPusher.Push(unPaused);
					pressed = false;
				}
			}
			else if (gameOverBtn[iter] == "Quit")
			{
				// set quitHighlight to true and send the event over
				// check if we have click Space to enter the new Play state
				GW::GEvent quitSelected;
				MX::MENU_STATE_DATA menuData;
				menuData.gameOverMenu = gameOverMenu;
				menuData.exitGameHighlighted = true;
				menuData.exitGamePressed = false;
				menuData.mainMenu = mainMenu;
				quitSelected.Write(MX::MENU_STATE::GAMEOVERMENU, menuData);
				eventPusher.Push(quitSelected);
				if (pressed == true)
				{
					gameOverMenu = false;
					mainMenu = true;
					GW::GEvent returnToMainMenu;
					MX::MENU_STATE_DATA menuData;
					menuData.gameOverMenu = gameOverMenu;
					menuData.exitGameHighlighted = true;
					menuData.exitGamePressed = pressed;
					menuData.mainMenu = mainMenu;
					iter = -1;
					returnToMainMenu.Write(MX::MENU_STATE::GAMEOVERMENU, menuData);
					eventPusher.Push(returnToMainMenu);
					pressed = false;
				}
			}
		}
		});


	game->system<MainMenuController, ControllerID>().each([this, mainMenuBtn, pauseMenuBtn, gameOverBtn](MainMenuController& mM, ControllerID& c)
		{
			GW::GEvent event;
			while (+pressEvents.Pop(event))
			{
				GW::INPUT::GController::Events controller;
				GW::INPUT::GController::EVENT_DATA c_data;
				GW::INPUT::GBufferedInput::Events keyboard;
				GW::INPUT::GBufferedInput::EVENT_DATA k_data;

				if (+event.Read(keyboard, k_data))
				{
					if (keyboard == GW::INPUT::GBufferedInput::Events::KEYPRESSED)
					{
						std::cout << k_data.data << '\n';
						if (k_data.data == G_KEY_W)
						{
							iter -= 1;
							if (iter < 0)
								iter = 0;
						}
						else if (k_data.data == G_KEY_S)
						{
							iter += 1;
							if (iter > ARRAYSIZE(mainMenuBtn) - 1)
								iter = ARRAYSIZE(mainMenuBtn) - 1;
						}

						if (k_data.data == G_KEY_SPACE)
						{
							pressed = true;
						}
						if (credits)
						{
							if (k_data.data == G_KEY_ESCAPE)
							{
								exitCredits = true;
							}
						}
					}
				}
				else if (+event.Read(controller, c_data))
				{
					if (controller == GW::INPUT::GController::Events::CONTROLLERBUTTONVALUECHANGED)
					{
						if (c_data.inputValue > 0 && c_data.inputCode == G_DPAD_UP_BTN)
						{
							iter -= 1;
							if (iter < 0)
							{
								iter = 0;
								controllerInput.StartVibration(c.index, 0, 0.5f, 0.1f);
							}
							else
								controllerInput.StartVibration(c.index, +1, 0.1f, 0.1f);
						}
						else if (c_data.inputValue > 0 && c_data.inputCode == G_DPAD_DOWN_BTN)
						{
							iter += 1;
							if (iter > ARRAYSIZE(mainMenuBtn) - 1)
							{
								iter = ARRAYSIZE(mainMenuBtn) - 1;
								controllerInput.StartVibration(c.index, 0, 0.5f, 0.1f);
							}
							else
								controllerInput.StartVibration(c.index, -1, 0.1f, 0.1f);
						}

						if (c_data.inputValue > 0 && c_data.inputCode == G_SOUTH_BTN)
						{
							pressed = true;
						}

						if (credits)
						{
							if (c_data.inputValue > 0 && c_data.inputCode == G_START_BTN)
							{
								exitCredits = true;
							}
						}
					}
				}
			}

			if (mainMenu)
			{
				if (mainMenuBtn[iter] == "Play")
				{
					// set playHighlight to true and send the event over
					// check if we have click Space to enter the new Play state
					GW::GEvent playSelected;
					MX::MENU_STATE_DATA menuData;
					menuData.mainMenu = mainMenu;
					menuData.credits = false;
					menuData.startGameHighlighted = true;
					menuData.startGamePressed = false;
					menuData.creditsHighlighted = false;
					menuData.creditsPressed = false;
					playSelected.Write(MX::MENU_STATE::MAINMENU, menuData);
					eventPusher.Push(playSelected);
					if (pressed == true)
					{
						std::cout << "We've Hit Play!" << '\n';
						mainMenu = false;
						GW::GEvent playGame;
						MX::MENU_STATE_DATA menuData;
						menuData.mainMenu = mainMenu;
						menuData.credits = false;
						menuData.startGameHighlighted = true;
						menuData.startGamePressed = pressed;
						menuData.creditsHighlighted = false;
						menuData.creditsPressed = false;
						playGame.Write(MX::MENU_STATE::MAINMENU, menuData);
						eventPusher.Push(playGame);
						pressed = false;
					}
				}
				else if (mainMenuBtn[iter] == "Credits")
				{
					// set quitHighlight to true and send the event over
					// check if we have click Space to enter the new Play state
					GW::GEvent creditsSelected;
					MX::MENU_STATE_DATA menuData;
					menuData.mainMenu = mainMenu;
					menuData.credits = false;
					menuData.startGameHighlighted = false;
					menuData.startGamePressed = false;
					menuData.creditsHighlighted = true;
					menuData.creditsPressed = false;
					creditsSelected.Write(MX::MENU_STATE::MAINMENU, menuData);
					eventPusher.Push(creditsSelected);
					if (pressed)
					{
						std::cout << "We've Hit Credits!" << '\n';
						mainMenu = false;
						credits = true;
						GW::GEvent playCredits;
						MX::MENU_STATE_DATA menuData;
						menuData.mainMenu = mainMenu;
						menuData.creditsHighlighted = true;
						menuData.creditsPressed = credits;
						playCredits.Write(MX::MENU_STATE::MAINMENU, menuData);
						eventPusher.Push(playCredits);
						playCredits.Write(MX::MENU_STATE::CREDITS, menuData);
						eventPusher.Push(playCredits);
						pressed = false;
					}
				}
			}
			else if (credits)
			{
				if (exitCredits)
				{
					credits = false;
					mainMenu = true;
					GW::GEvent returnToMainMenu;
					MX::MENU_STATE_DATA menuData;
					menuData.mainMenu = mainMenu;
					menuData.credits = credits;
					menuData.startGameHighlighted = true;
					menuData.startGamePressed = false;
					menuData.creditsHighlighted = false;
					menuData.creditsPressed = pressed;
					iter = -1;
					returnToMainMenu.Write(MX::MENU_STATE::CREDITS, menuData);
					eventPusher.Push(returnToMainMenu);
					exitCredits = false;
				}
			}
		});


	return true;
}

bool MX::MenuController::Activate(bool runSystem)
{
	if (runSystem)
	{
		game->entity("MainMenu-Controller").enable();
		game->entity("PauseMenu-Controller").enable();
		game->entity("GameOverMenu-Controller").enable();
	}
	else if (!runSystem)
	{
		game->entity("MainMenu-Controller").disable();
		game->entity("PauseMenu-Controller").disable();
		game->entity("GameOverMenu-Controller").disable();
	}


	return true;
}

bool MX::MenuController::Shutdown()
{
	game->defer_begin();
	game->entity("Menu-Controller").destruct();
	game->defer_end();

	return true;
}
