#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

namespace MX
{
	class MenuController
	{
		std::shared_ptr<flecs::world> game;

		GW::CORE::GEventGenerator eventPusher;

		GW::INPUT::GInput immediateInput;
		GW::INPUT::GBufferedInput bufferedInput;
		GW::INPUT::GController controllerInput;

		GW::CORE::GEventCache pressEvents;


		GW::CORE::GEventResponder gameOverHandler;
		GW::CORE::GEventResponder pauseMenuHandler;
		GW::CORE::GEventResponder mainMenuHandler;
		GW::CORE::GEventResponder creditsMenuHandler;

		bool pressed = false;
		bool exitCredits = false;

		bool mainMenu = true;
		bool startSelected = false;
		bool startPressed = false;
		bool fullScreenHighlighted = false;
		bool fullScreenPressed = true;
		bool creditsHighlighted = false;
		bool creditsPressed = false;

		bool gameOverMenu = false;
		bool retrySelected = false;
		bool retryPressed = false;
		
		bool pauseMenu = false;
		bool resumeSelected = false;
		bool resumePressed = false;

		bool quitSelected = false;
		bool quitPressed = false;

		bool credits = false;
		bool exitCreditsPressed = false;

		int iter = 0;
	public:
		bool Init(std::shared_ptr<flecs::world> _game, 
			GW::INPUT::GInput _immediateInput,
			GW::INPUT::GController _controllerInput,
			GW::INPUT::GBufferedInput _bufferedInput,
			GW::CORE::GEventGenerator _eventPusher);
		bool Activate(bool runSystem);
		bool Shutdown();
	private:
		static constexpr unsigned int Max_Frame_Events = 32;

	};
}

#endif