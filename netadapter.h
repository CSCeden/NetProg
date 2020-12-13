#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <Windows.h>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <Windows.h>
#include <iphlpapi.h>
#include <tchar.h>
#include <netcon.h>

class NetAdapter
{
public:
	void showAdapterList();
	std::unordered_map<std::string, std::string> getAdapters();
	HRESULT DisableEnableConnections(BOOL bEnable, const wchar_t* AdapterName);
};
