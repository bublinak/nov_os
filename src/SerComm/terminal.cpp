#include "terminal.h"

/*

*/
String terminal::processMessage(String message)
{
	String args[10];
	int argsCount = 0;
	String response = "";
	argsCount = 0;
	while (message.length() > 0)
	{
		int index = message.indexOf(' ');
		if (index == -1) // No space found
		{
			args[argsCount] = message;
			args[argsCount].trim();
			argsCount++;
			break;
		}
		else
		{
			args[argsCount] = message.substring(0, index);
			message = message.substring(index + 1);
			args[argsCount].trim();
			argsCount++;
		}
	}
	if (argsCount > 0)
	{
		if (args[0] == "preferences")
		{
			if (args[1] == "get")
			{
				response = "Preferences getting on the go...";
			} else if (args[1] == "set")
			{
				response = "Preferences setting on the go...";
			} else if (args[1] == "open")
			{
				response = "Preferences opening on the go...";
			} else if (args[1] == "close")
			{
				response = "Preferences closing on the go...";
			} else if (args[1] == "list")
			{
				response = "Preferences listing on the go...";
			} else {
				response = "Preferences help on the go...";
			}
		} else if (args[0] == "reboot")
		{
			response = "Reboot on the go...";
			vTaskDelay(1000/portTICK_PERIOD_MS);
			ESP.restart();
		} else if (args[0] == "reset")
		{
			response = "Reset on the go...";
		} else if (args[0] == "file")
		{
			response = "File on the go...";
		} else if (args[0] == "lua")
		{
			response = "Lua on the go...";
		} else	// Help
		{
			response = "Help on the go...";
		}
	} 
	return response;
}