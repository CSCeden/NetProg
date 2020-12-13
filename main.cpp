#include <stdio.h>

#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#include <IcmpAPI.h>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")

#include "net.h"
#include "netadapter.h"
#include "Parser.h"

using namespace std;

std::string Data[256];
int index = 0;

void execute(std::string exePath)
{
	std::cout << "EdenCon Version [1.0 Beta]\n\n";

	NetAdapter netadapter;
	ZeroMemory(&netadapter, sizeof(netadapter));

	while (true)
	{
		std::cout << ">";

		string command = " ";
		cin >> command;

		if (command == "net_dump")
		{
			cout << "IP: ";
			string ip;
			cin >> ip;

			char* destIpStr = NULL;
			u_char slash = 32;
			int c;
			IPAddr destIp = 0;
			u_long loopcount;
			HANDLE* hHandles;
			DWORD ThreadId;
			u_long i = 0;

			if (!init_winsock()) {
				exit(1);
			}

			slash = 24;

			destIp = lookupAddress(ip.c_str());

			if (slash != 32) {
				destIp = getFirstIP(destIp, slash);
			}

			if (destIp == INADDR_NONE) {
				fprintf(stderr, "Error: Could not find IP %s\n", destIpStr);
				exit(1);
			}

			SetConsoleCtrlHandler((PHANDLER_ROUTINE)&controlc, TRUE);

			loopcount = 1 << (32 - slash);
			hHandles = (HANDLE*)malloc(sizeof(HANDLE) * loopcount);

			for (i = 0; i < loopcount; i++) {
				struct in_addr ip_addr;
				ip_addr.s_addr = destIp;

				hHandles[i] = CreateThread(NULL, 0, sendAnARP, &destIp, 0, &ThreadId);

				if (hHandles[i] == NULL) {
					fprintf(stderr, "No thread created!\n");
					exit(1);
				}

				Sleep(10);
				u_long temp = ntohl(destIp);
				destIp = ntohl(++temp);
			}

			for (i = 0; i < loopcount; i++) {
				WaitForSingleObject(hHandles[i], INFINITE);
			}

			WSACleanup();

			cout << "Do you want to save the data? - yes : no" << "\n";
			cout << ">";
			string selectionSave;
			cin >> selectionSave;

			if (selectionSave == "yes")
			{
				string dumpFilename = "mac_dump.txt";
				std::ofstream macaddressesFile(dumpFilename);
				if (macaddressesFile.is_open())
				{
					for (int i = 0; i <= index; i++)
					{
						if (i == index)
							break;

						macaddressesFile << Data[i] << "\n";
					}
					macaddressesFile.close();
				}

				std::string cur_dir(exePath);
				int pos = cur_dir.find_last_of("/\\");

				std::cout << "Dumped at: " << cur_dir.substr(0, pos) << std::endl;
			}
			else if (selectionSave == "no")
			{
				cout << "Did not save data!" << endl;
			}
			else {
				cout << "Invalid input!" << endl;
			}
			cout << "\n";
		}
		if (command == "mac_change")
		{
			vector <string> list;
			unordered_map<string, string> AdapterDetails = netadapter.getAdapters();
			for (auto& itm : AdapterDetails)
			{
				list.push_back(itm.first);
			}

			cout << "Available Adapters: " << endl;
			int range = 0;
			for (auto itm = list.begin(); itm != list.end(); itm++)
			{
				cout << '\t' << range + 1 << ")" << *itm << endl;
				range++;
			}

			cout << "Adapter> ";

			int selection = 0;
			cin >> selection;

			if (cin.fail() || (selection < 1) || (selection > range))
			{
				cin.clear();
				cerr << "[!]Invalid Selection Input!" << endl;
				return;
			}

			string oldMAC = AdapterDetails.at(list.at(selection - 1));

			cout << "Mac Address>";
			string temp;
			cin >> temp;

			wstring wstr(list.at(selection - 1).begin(), list.at(selection - 1).end());
			const wchar_t* wAdapterName = wstr.c_str();

			bool bRet = false;
			HKEY hKey = NULL;
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				_T("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002bE10318}"),
				0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
			{
				DWORD dwIndex = 0;
				TCHAR Name[1024];
				DWORD cName = 1024;
				while (RegEnumKeyEx(hKey, dwIndex, Name, &cName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
				{
					HKEY hSubKey = NULL;
					if (RegOpenKeyEx(hKey, Name, 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
					{
						BYTE Data[1204];
						DWORD cbData = 1024;
						if (RegQueryValueEx(hSubKey, _T("DriverDesc"), NULL, NULL, Data, &cbData) == ERROR_SUCCESS)
						{
							if (_tcscmp((TCHAR*)Data, wAdapterName) == 0)
							{
								string newMAC = temp;
								temp.erase(std::remove(temp.begin(), temp.end(), '-'), temp.end());

								wstring wstr_newMAC(temp.begin(), temp.end());
								const wchar_t* newMACAddr = wstr_newMAC.c_str();

								if (RegSetValueEx(hSubKey, _T("NetworkAddress"), 0, REG_SZ,
									(const BYTE*)newMACAddr, sizeof(TCHAR) * ((DWORD)_tcslen(newMACAddr) + 1)) == ERROR_SUCCESS)
								{
									cout << "MAC Address " << "[" << oldMAC << "] " << "successfully changed to: " << newMAC << endl;
									netadapter.DisableEnableConnections(false, wAdapterName);
									netadapter.DisableEnableConnections(true, wAdapterName);
								}
							}
						}
						RegCloseKey(hSubKey);
					}
					cName = 1024;
					dwIndex++;
				}
				RegCloseKey(hKey);
			}
			else
			{
				cerr << "Could not change MAC Address - Try running the program with higher privlages" << endl;
			}
			cout << "\n";
		}
		if (command == "clear")
		{
			system("cls");
			std::cout << "LittleMac Version [1.0 Beta]\n\n";
		}
		if (command == "net_local_dg")
		{
			Parser parser;
			cout << parser.GetParsedIPConfigData("Default Gateway") << "\n\n";
		}
		if (command == "net_local_ipv4")
		{
			Parser parser;
			cout << parser.GetParsedIPConfigData("IPv4 Address") << "\n\n";
		}
		if (command == "net_local_mac")
		{		
			std::string APPDATA = getenv("APPDATA");
			std::string path = APPDATA + "\\getmacdata.txt";
			std::string cmd = "getmac > " + path;

			Parser parser;

			system(cmd.c_str());

			//TODO: Get actuall line count
			for (int i = 0; i < 5; i++)
			{
				std::string data = parser.GetParsedGetMacData(i, path);

				if (!data.empty() && data != " ")
				{
					cout << data << "\n";
				}
			}
			cout << "\n";
		}
		if (command == "help")
		{
			cout << "net_dump -- Dump mac addresses and ips on local network\n";
			cout << "mac_change -- Change mac address to 12 hex decimal\n";
			cout << "net_local_dg -- Get network default gateway\n";
			cout << "net_local_ipv4 -- Get local ipv4 address\n";
			cout << "net_local_mac -- Get current mac address of selected adapter\n";
			cout << "\n";
		}
	}
}

int main(int argc, char* argv[])
{
	execute(argv[0]);
	std::cin.get();

	return EXIT_SUCCESS;
}



