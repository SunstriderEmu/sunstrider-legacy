#include "TestMgr.h"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "Log.h"

namespace Testing
{
	int Testing::RunCatch(int argc, char const* const* const argv)
	{
		// Prepare Catch. CatchSession may be initialized only once, so we store it as static
		static Catch::Session* catch_session = nullptr;
		if(catch_session == nullptr)
			catch_session = new Catch::Session(); //will never be deleted but whatever, this is not meant to be use in production
         
		catch_session->resetConfigData(); //custom added function
		int result = catch_session->run( argc, argv );
		return result;
	}

	void Testing::ClearAllTests()
	{
		TC_LOG_INFO("scripts.hotswap", "Clearing all tests");
		Catch::getRegistryHub().clearAllTestsFromRegistry();
	}
}