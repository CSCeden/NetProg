#include "netadapter.h"

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

using namespace std;

void NetAdapter::showAdapterList()
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	UINT i;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		cerr << "Error allocating memory needed to call GetAdaptersinfo." << endl;
	}
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			cerr << "Error allocating memory needed to call GetAdaptersinfo" << endl;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			cout << "\n\tComboIndex: \t" << pAdapter->ComboIndex << endl;
			cout << "\tAdapter Name: \t" << pAdapter->AdapterName << endl;
			cout << "\tAdapter Desc: \t" << pAdapter->Description << endl;
			cout << "\tAdapter Addr: \t";
			for (i = 0; i < pAdapter->AddressLength; i++) {
				if (i == (pAdapter->AddressLength - 1))
					printf("%.2X\n", (int)pAdapter->Address[i]);
				else
					printf("%.2X-", (int)pAdapter->Address[i]);
			}
			cout << "\tIP Address: \t" << pAdapter->IpAddressList.IpAddress.String << endl;
			cout << "\tIP Mask: \t" << pAdapter->IpAddressList.IpMask.String << endl;
			cout << "\tGateway: \t" << pAdapter->GatewayList.IpAddress.String << endl;
			pAdapter = pAdapter->Next;
		}
	}
	else {
		cerr << "GetAdaptersInfo failed with error: " << dwRetVal << endl;
	}
	if (pAdapterInfo)
		FREE(pAdapterInfo);
}

unordered_map<string, string> NetAdapter::getAdapters()
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	unordered_map<string, string> result;
	stringstream temp;
	string str_mac;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		cerr << "Error allocating memory needed to call GetAdaptersinfo" << endl;
	}
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			cerr << "Error allocating memory needed to call GetAdaptersinfo\n" << endl;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			for (UINT i = 0; i < pAdapter->AddressLength; i++) {
				temp << setfill('0') << setw(2) << hex << (int)pAdapter->Address[i];
				if (i != pAdapter->AddressLength - 1)
				{
					temp << "-";
				}
			}
			str_mac = temp.str();
			temp.str("");
			temp.rdbuf();
			for (auto& c : str_mac)
			{
				c = toupper(c);
			}

			result.insert({ pAdapter->Description, str_mac });
			pAdapter = pAdapter->Next;
		}
	}
	else {
		cerr << "GetAdaptersInfo failed with error: " << dwRetVal << endl;
	}
	if (pAdapterInfo)
		FREE(pAdapterInfo);

	return result;
}

HRESULT NetAdapter::DisableEnableConnections(BOOL bEnable, const wchar_t* AdapterName)
{
	HRESULT hr = E_FAIL;

	CoInitialize(0);

	INetConnectionManager* pNetConnectionManager = NULL;
	hr = CoCreateInstance(CLSID_ConnectionManager,
		NULL,
		CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
		IID_INetConnectionManager,
		reinterpret_cast<LPVOID*>(&pNetConnectionManager)
	);
	if (SUCCEEDED(hr))
	{
		IEnumNetConnection* pEnumNetConnection;
		pNetConnectionManager->EnumConnections(NCME_DEFAULT, &pEnumNetConnection);

		ULONG ulCount = 0;
		BOOL fFound = FALSE;
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

		HRESULT hrT = S_OK;

		do
		{
			NETCON_PROPERTIES* pProps = NULL;
			INetConnection* pConn;

			hrT = pEnumNetConnection->Next(1, &pConn, &ulCount);

			if (SUCCEEDED(hrT) && 1 == ulCount)
			{
				hrT = pConn->GetProperties(&pProps);

				if (S_OK == hrT)
				{
					if (bEnable && (_tcscmp((TCHAR*)pProps->pszwDeviceName, AdapterName) == 0))
					{
						printf("Enabling adapter: %S...\n", pProps->pszwDeviceName);
						hr = pConn->Connect();
					}
					else if (_tcscmp((TCHAR*)pProps->pszwDeviceName, AdapterName) == 0)
					{
						printf("Disabling adapter: %S...\n", pProps->pszwDeviceName);
						hr = pConn->Disconnect();
					}

					CoTaskMemFree(pProps->pszwName);
					CoTaskMemFree(pProps->pszwDeviceName);
					CoTaskMemFree(pProps);
				}
				pConn->Release();
				pConn = NULL;
			}

		} while (SUCCEEDED(hrT) && 1 == ulCount && !fFound);

		if (FAILED(hrT))
		{
			hr = hrT;
		}
		pEnumNetConnection->Release();
	}

	if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_RETRY))
	{
		printf("Could not enable or disable connection (0x%08x)\r\n", hr);
	}

	pNetConnectionManager->Release();
	CoUninitialize();

	return hr;
}