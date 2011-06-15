#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <bthsdpdef.h>
#include <bthdef.h>
#include <BluetoothAPIs.h>

#include <strsafe.h>

#pragma comment(lib, "Bthprops.lib")

// MSAutoConnectWiimote.cpp
// Adapted from code by Richard L. Lynch, retrieved 2011-01-18 from http://www.richlynch.com/code/wiipair
// 
// Called without input arguments, this executable will use all local Bluetooth radios to look for any
// Bluetooth devices that identify according to certain common Wiimote device names.  It will pair with
// any that it finds that are not already connected.  It will keep repeating the scan until it has
// performed a new pairing with at least one device.
//
// If called with Bluetooth devices' MAC addresses as input arguments, e.g.:
//          MSAutoConnectWiimote   00:21:bd:6e:35:cd    00:23:31:0e:7a:4c
// then this program will keep scanning until it finds the first specified device, pair with it if
// it is not already connected, and then repeat this procedure, going through the list in order until
// it has ensured that all specified devices are connected.
//
// Press (do not hold) 1 & 2 simultaneously on all Wiimotes, breathe for two seconds, then launch this
// program. After exiting, go to "Show Bluetooth Devices" and verify that each Wiimote is described
// using *three* lines: "Nintendo RVL-CNT-01",  "No passkey", and "Connected".   Without that last line
// saying "Connected" (which is flakily sometimes absent) the Wiimote will forget the pairing within a
// few seconds.  If so, retry this program.

DWORD ShowErrorCode(LPTSTR msg, DWORD dw)
{
   // Retrieve the system error message for the last-error code

   LPVOID lpMsgBuf;
   if(dw == ERROR_SUCCESS) return dw;
   FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dw,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &lpMsgBuf,
      0,
      NULL
      );

   _tprintf(_T("%s error %d: %s"), msg, dw, lpMsgBuf);

   LocalFree(lpMsgBuf);

   return dw;
}

_TCHAR * FormatBTAddress(BLUETOOTH_ADDRESS address)
{
   static _TCHAR ret[20];
   _stprintf(ret, _T("%02x:%02x:%02x:%02x:%02x:%02x"),
      address.rgBytes[5],
      address.rgBytes[4],
      address.rgBytes[3],
      address.rgBytes[2],
      address.rgBytes[1],
      address.rgBytes[0]
      );
   return ret;
}

int MatchBTAddress(BLUETOOTH_ADDRESS * addr, const char *str)
{
	int i, nscanned, b[6];
	if(str == NULL || addr == NULL) return 0;
	nscanned = sscanf(str, "%x:%x:%x:%x:%x:%x", b+5, b+4, b+3, b+2, b+1, b+0);
	if(nscanned != 6) return 0;
	for(i = 0; i < 6; i++) if(addr->rgBytes[i] != (BYTE)(b[i])) return 0;
	return 1;
}

int main(int argc, char* argv[])
{
   HANDLE hRadios[256];
   int nRadios;
   int nPaired = 0;
   int nNewlyPaired = 0;
   int nToPair = argc-1;
   int istarget = 0;

   ///////////////////////////////////////////////////////////////////////
   // Enumerate BT radios
   ///////////////////////////////////////////////////////////////////////
   {
      HBLUETOOTH_RADIO_FIND hFindRadio;
      BLUETOOTH_FIND_RADIO_PARAMS radioParam;
     
      _tprintf(_T("Enumerating radios...\n"));

      radioParam.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);

      nRadios = 0;
      hFindRadio = BluetoothFindFirstRadio(&radioParam, &hRadios[nRadios++]);
      if (hFindRadio)
      {
         while (BluetoothFindNextRadio(&radioParam, &hRadios[nRadios++]));
         BluetoothFindRadioClose(hFindRadio);
      }
      else
      {
         ShowErrorCode(_T("BluetoothFindFirstRadio"), GetLastError());
         return (1);
      }
      nRadios--;
      _tprintf(_T("Found %d radios\n"), nRadios);
   }

   ///////////////////////////////////////////////////////////////////////
   // Keep looping until we pair with a Wii device
   ///////////////////////////////////////////////////////////////////////
   while ((nToPair == 0 && nNewlyPaired == 0) || nPaired < nToPair)
   {
      int radio;
     
      for (radio = 0; radio < nRadios; radio++)
      {
         BLUETOOTH_RADIO_INFO radioInfo;
         HBLUETOOTH_DEVICE_FIND hFind;
         BLUETOOTH_DEVICE_INFO btdi;
         BLUETOOTH_DEVICE_SEARCH_PARAMS srch;
         
         radioInfo.dwSize = sizeof(radioInfo);
         btdi.dwSize = sizeof(btdi);
         srch.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
         
         ShowErrorCode(_T("BluetoothGetRadioInfo"), BluetoothGetRadioInfo(hRadios[radio], &radioInfo));

         _tprintf(_T("\nRadio %d: %ls %s\n"),
               radio,
               radioInfo.szName,
               FormatBTAddress(radioInfo.address)
               );
         
         srch.fReturnAuthenticated = TRUE;
         srch.fReturnRemembered = TRUE;
         srch.fReturnConnected = TRUE;
         srch.fReturnUnknown = TRUE;
         srch.fIssueInquiry = TRUE;
         srch.cTimeoutMultiplier = 4;
         srch.hRadio = hRadios[radio];

         _tprintf(_T("    Scanning...\n"));
         
         hFind = BluetoothFindFirstDevice(&srch, &btdi);

         if (hFind == NULL)
         {
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
            {
               _tprintf(_T("    No bluetooth devices found.\n"));
            }
            else
            {
               ShowErrorCode(_T("    BluetoothFindFirstDevice"), GetLastError());  
               return (1);
            }
         }
         else
         {
            do
            {
			   istarget = nToPair && MatchBTAddress(&btdi.Address, argv[nPaired+1]);
			   if(nToPair && !istarget) continue;

               _tprintf(_T("    Found: %s \"%s\"\n"), FormatBTAddress(btdi.Address), btdi.szName);
               
               if (istarget || !wcscmp(btdi.szName, L"Nintendo RVL-WBC-01") || !wcscmp(btdi.szName, L"Nintendo RVL-CNT-01"))
               {
                  DWORD pcServices = 16;
                  GUID guids[16];
                  DWORD error = ERROR_SUCCESS;

				  if (btdi.fConnected) {
                     _tprintf(_T("    Device is already connected\n"));
					 nPaired++;
					 continue;
				  }
				  if (btdi.fRemembered) { // Make Windows forget pairing
                     _tprintf(_T("    Device is \"remembered\" but not connected. Removing...\n"));
					 error = ShowErrorCode(_T("    BluetoothRemoveDevice"), BluetoothRemoveDevice(&btdi.Address));
					 if(error != ERROR_SUCCESS) continue;
				  }

                  _tprintf(_T("    Pairing...\n"));
				  if(0) {  // Contrary to Richard Lynch's original code, this authentication step seems to be unnecessary (and fails, if attempted)
                     WCHAR pass[6];
                     // MAC address is passphrase
                     pass[0] = (WCHAR)(radioInfo.address.rgBytes[0]);
                     pass[1] = (WCHAR)(radioInfo.address.rgBytes[1]);
                     pass[2] = (WCHAR)(radioInfo.address.rgBytes[2]);
                     pass[3] = (WCHAR)(radioInfo.address.rgBytes[3]);
                     pass[4] = (WCHAR)(radioInfo.address.rgBytes[4]);
                     pass[5] = (WCHAR)(radioInfo.address.rgBytes[5]);
                     // Pair with Wii device.
                     error = ShowErrorCode(_T("    BluetoothAuthenticateDevice"), BluetoothAuthenticateDevice(NULL, hRadios[radio], &btdi, pass, 6));
			         if(error != ERROR_SUCCESS) continue;
				  }

                  // If this is not done, the Wii device will not remember the pairing
                  error = ShowErrorCode(_T("    BluetoothEnumerateInstalledServices"), BluetoothEnumerateInstalledServices(hRadios[radio], &btdi, &pcServices, guids));
			      if(error != ERROR_SUCCESS) continue;

                  // Activate service
                  error = ShowErrorCode(_T("    BluetoothSetServiceState"), BluetoothSetServiceState (hRadios[radio], &btdi, &HumanInterfaceDeviceServiceClass_UUID, BLUETOOTH_SERVICE_ENABLE ));
			      if(error != ERROR_SUCCESS) continue;
                 
                  _tprintf(_T("    Paired.\n"));
                  nPaired++;
				  nNewlyPaired++;

               } // if (!wcscmp(btdi.szName, L"Nintendo RVL-WBC-01") || !wcscmp(btdi.szName, L"Nintendo RVL-CNT-01"))
            } while (BluetoothFindNextDevice(hFind, &btdi));

			BluetoothFindDeviceClose(hFind);

         } // if (hFind == NULL)
      } // for (radio = 0; radio < nRadios; radio++)
      Sleep(1000);
   }

   ///////////////////////////////////////////////////////////////////////
   // Clean up
   ///////////////////////////////////////////////////////////////////////

   {
      int radio;
     
      for (radio = 0; radio < nRadios; radio++)
      {
         CloseHandle(hRadios[radio]);
      }
   }
   
   _tprintf(_T("=============================================\n"), nPaired);
   _tprintf(_T("%d Wii devices paired\n"), nPaired);

	char strTemp[256]; printf("press return"); scanf("blah %s\n", strTemp);
   return 0;
}