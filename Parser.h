#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "Singelton.h"

class Parser
{
public:
	std::string GetParsedIPConfigData(std::string Columb)
	{
		//Set up command file path and command line command
		std::string APPDATA = getenv("APPDATA");
		std::string path = APPDATA + "\\localipdata.txt";
		std::string cmd =  "ipconfig > " + path;

		//execute ipconfig command and save file to path
		system(cmd.c_str());

		//current line
		std::string line;

		//Line array : Here is all lines saved
		std::string lineArray[500];
		int arrayCount = 0;

		std::ifstream file(path);
		if (file.is_open())
		{
			//Get all lines
			while (std::getline(file, line))
			{
				//Save each line into a element in an array
				lineArray[arrayCount] = line;
				arrayCount++;
			}

			for (int arrayindex = 0; arrayindex <= arrayCount; arrayindex++)
			{
				std::string s = Columb;
				std::string s2 = ":";

				//Search all lines and get pos
				std::size_t i = lineArray[arrayindex].find(s);
				std::size_t i2 = lineArray[arrayindex].find(s2);

				//Found a match for Columb
				if (lineArray[arrayindex].find(s) != std::string::npos)
				{
					//Validate
					if (i != std::string::npos)
					{
						//Earse Columb name
						lineArray[arrayindex].erase(i, s.length());

						//Erase all blanks
						lineArray[arrayindex].erase(remove_if(lineArray[arrayindex].begin(), lineArray[arrayindex].end(), isspace), lineArray[arrayindex].end());

						//Found match for ':'
						if (lineArray[arrayindex].find(s2) != std::string::npos)
						{
							//Validate
							if (i2 != std::string::npos)
							{
								//Delete all characters prior to ':'
								lineArray[arrayindex].erase(0, lineArray[arrayindex].find(":"));
								lineArray[arrayindex].erase(std::remove(lineArray[arrayindex].begin(), lineArray[arrayindex].end(), ':'), lineArray[arrayindex].end());
							}
						}
						//Return our data
						return lineArray[arrayindex];
					}
				} 

				//Only go through all lines once
				if (arrayindex == arrayCount)
					break; 
			} 

			//Close file
			file.close();
		}
		//Something went wrong
		return "Invalid";
	}

	std::string lineArrayRead[5];

	std::string GetParsedGetMacData(int index, std::string path)
	{
		//current line
		std::string line;

		int arrayCount = 0;

		lineArrayRead[arrayCount] = "Invalid";

		std::ifstream file(path);
		if (file.is_open())
		{
			//Get all lines
			while (std::getline(file, line))
			{
				//Save each line into a element in an array
				lineArrayRead[arrayCount] = line;
				arrayCount++;
			}

			//What to search for
			std::string s = "Physical Address";
			std::string s2 = "-";
			std::string s3 = "=";
			std::string s4 = "Media disconnected";

			//Search all lines and get pos
			std::size_t i = lineArrayRead[index].find(s);
			std::size_t i2 = lineArrayRead[index].find(s2);
			std::size_t i3 = lineArrayRead[index].find(s3);
			std::size_t i4 = lineArrayRead[index].find(s4);

			//Find and delete Physical Address Transport Name line
			if (lineArrayRead[index].find(s) != std::string::npos)
			{
				if (i != std::string::npos)
				{
					lineArrayRead[index].erase(i, lineArrayRead[index].length());
				}
			}

			//Delete spearator line [=]
			if (lineArrayRead[index].find(s3) != std::string::npos)
			{
				lineArrayRead[index].erase(std::remove(lineArrayRead[index].begin(), lineArrayRead[index].end(), '='), lineArrayRead[index].end());
			}

			//Delete everything after the mac addresses : should be in THIS FORMAT XX-XX-XX-XX-XX-XX, 17 chars long
			if (lineArrayRead[index].find(s2) != std::string::npos)
			{
				if (i2 != std::string::npos)
				{
					if (lineArrayRead[index].find(s4) != std::string::npos)
					{
						//Check for Media disconnected : if found then dont parse
						if (i4 != std::string::npos)
						{
							lineArrayRead[index] = lineArrayRead[index];
						}
					} else {
						lineArrayRead[index].erase(18, lineArrayRead[index].length());

						//Erase all blanks
						lineArrayRead[index].erase(remove_if(lineArrayRead[index].begin(), lineArrayRead[index].end(), isspace), lineArrayRead[index].end());
					}
				}
			}

			//Close file
			file.close();

			//Return parsed line parsed data
			return lineArrayRead[index];
		}
		//Something went wrong
		return "Invalid";
	}
}; 
