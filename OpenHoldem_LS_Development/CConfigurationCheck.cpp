#include "stdafx.h"

#include "CConfigurationCheck.h"
#include "CPreferences.h"


CConfigurationCheck *p_configurationcheck = 0;

// A full list of keyboard layout codes can be found here:
// http://msdn.microsoft.com/en-us/goglobal/bb895996.aspx
const TCHAR k_KeyboardLayout_UK_US_English[KL_NAMELENGTH] = "00000409";

CConfigurationCheck::CConfigurationCheck()
{
	CheckEverything();
}

CConfigurationCheck::~CConfigurationCheck()
{}

void CConfigurationCheck::CheckEverything()
{
	// Check really critical settings in any case.
	// OpenHoldem will never work, if these are not right.
	CheckColourDepth();

	// OpenHoldem may or may not work, if these are not right.
	if (prefs.configurationcheck_keyboard_settings())
	{
		CheckKeyboardSettings();
	}

	if (prefs.configurationcheck_theme_settings())
	{
		CheckForClassicalTheme();
	}

	if (prefs.configurationcheck_font_settings())
	{
		CheckForFontSmoothing();
	}

	if (CheckOfPerlInstallationNecessary())
	{
		CheckForMissingMSVCRT();
		CheckForMissingActivePerl();
		CheckForPerlPath();
	}
}

bool CConfigurationCheck::CheckOfPerlInstallationNecessary()
{
	// We better make that check dependend of the perl-preferences,
	// instead of a separate option that might be turned off
	// or might confuse other users.
	return (prefs.perl_load_default_formula() || prefs.perl_load_interpreter());
}

HKEY CConfigurationCheck::GetHive(CString mhive)
{
	HKEY hive;

	if (mhive == "HKCU")
		hive = HKEY_CURRENT_USER;

	if (mhive == "HKLM")
		hive = HKEY_LOCAL_MACHINE;

	return hive;
}

bool CConfigurationCheck::OpenKey(CString mhive, CString registry_path)
{
	HKEY hive, hKey;
	hive = GetHive(mhive);

	if (RegOpenKeyEx(hive, registry_path, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return true;
	}

	RegCloseKey(hKey);
	return false;
}


CString CConfigurationCheck::GetValue(CString mhive, int type, CString registry_path, CString key_name)
{
	DWORD dwData, nBytes, dwType;
	CString value;
	HKEY hive, pKey;
	hive = GetHive(mhive);

	// Open the Key
	if(RegOpenKeyEx(hive,registry_path,0,KEY_QUERY_VALUE,&pKey)==ERROR_SUCCESS)
	{
		switch(type)
		{
			//REG_DWORD
			case 0 :

				if(RegQueryValueEx(pKey,key_name,NULL,&dwType,(LPBYTE)&dwData,&nBytes)==ERROR_SUCCESS)
				{
					value.Format("%d", dwData);
					RegCloseKey(pKey);
					return value;
				}

			//REG_SZ, REG_EXPAND_SZ
			case 1 :

				if(RegQueryValueEx(pKey,key_name,NULL,&dwType,0,&nBytes)==ERROR_SUCCESS)
				{
					// Allocate string buffer
					LPTSTR buffer = value.GetBuffer(nBytes/sizeof(TCHAR));
					// Read string
					VERIFY(RegQueryValueEx(pKey,key_name,NULL,0,(LPBYTE)buffer,&nBytes)==ERROR_SUCCESS);
					// Release string buffer
					value.ReleaseBuffer();
					//printf("Get SZ Value OK\n");
					RegCloseKey(pKey);
					return value;
				}

			default :
				return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

void CConfigurationCheck::CheckColourDepth()
{
	CWindowDC dc(NULL);
	int nBitsPerPixel = dc.GetDeviceCaps(PLANES) * dc.GetDeviceCaps(BITSPIXEL);
	if (nBitsPerPixel < 24 && !prefs.disable_msgbox())
		MessageBox(0, "It appears that your Display settings are not configured according to OpenHoldem specifications.\n"
				"24 bit color or higher is needed to reliably extract information from the poker client.\n\n"
				"For more info, look at the manual and the user forums",
				"Caution: Color Depth Too Low", MB_OK|MB_ICONWARNING);
}

void CConfigurationCheck::CheckKeyboardSettings()
{
	TCHAR KeyboardLayout[KL_NAMELENGTH];
	bool Success = GetKeyboardLayoutName((LPSTR)&KeyboardLayout);

	if (Success && (_tcscmp(KeyboardLayout, k_KeyboardLayout_UK_US_English) != 0))
	{
		MessageBox(0, "You seem to have non-english keyboard settings.\n"
				"Keyboard settings affect especially the decimal point in numbers\n"
				"and therefore the scraper-engine and the auto-player.\n"
				"If you continue, OpenHoldem may or may not work as expected.\n"
				"If you are an experienced user with a working setup\n"
				"you may turn this warning off.\n"
				"If you are new to OpenHoldem or encounter problems\n"
				"you should fix your keyboard settings\n",
				"Caution: Improper keyboard settings", MB_OK|MB_ICONWARNING);
	}
}

void CConfigurationCheck::CheckForClassicalTheme()
{
	CString p_szKeyThemeActive = "Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager\\";
	CString p_szKeyWinVersion = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\";

	CString p_szNameThemeActive = "ThemeActive";
	CString p_szNameWinVersion = "CurrentVersion";

	bool classic_theme = false;

	CString win_Vs = GetValue("HKLM", 1, p_szKeyWinVersion, p_szNameWinVersion);

	CString WIN_2000 = "5.0";
	CString SRV_2003 = "5.2";

	if(win_Vs == WIN_2000 || win_Vs == SRV_2003)
	{
		classic_theme = true;
	}

	else if (atoi(GetValue("HKCU", 1, p_szKeyThemeActive, p_szNameThemeActive)) == 0)
	{
		classic_theme = true;
	}

	if (classic_theme == false)
	{
		MessageBox(0, "Classical Theme Not Found\n"
			"Settings deviate from recommended defaults.\n"
			"\n"
			"Please enable windows classical theme.\n"
			"for optimum compatibility \n",
			"Caution: Classical Theme Disabled", MB_OK|MB_ICONWARNING);
	}
}


void CConfigurationCheck::CheckForFontSmoothing()
{
	CString p_szKeyFontSmoothing = "Control Panel\\Desktop\\";
	CString p_szNameFontSmooth = "FontSmoothing";
	CString p_szNameFontSmoothType = "FontSmoothingType";

	bool font_smoothing = true;

	if (atoi(GetValue("HKCU", 1, p_szKeyFontSmoothing, p_szNameFontSmooth)) == 0)
	{
		if (atoi(GetValue("HKCU", 0, p_szKeyFontSmoothing, p_szNameFontSmoothType)) == 0)
		{
			font_smoothing = false;
		}
	}

	if (font_smoothing == true)
	{
		if (atoi(GetValue("HKCU", 1, p_szKeyFontSmoothing, p_szNameFontSmooth)) == 2)
		{
			int FontSmoothType = atoi(GetValue("HKCU", 0, p_szKeyFontSmoothing, p_szNameFontSmoothType));

			if (FontSmoothType == 2)
			{
				MessageBox(0, "It appears that Cleartype font smoothing is enabled.\n"
							"\n"
							"In order for OpenHoldem to reliably\n"
							"extract information from the poker client\n"
							"you should disable Font Smoothing.",
							"Caution: Font smoothing is enabled", MB_OK|MB_ICONWARNING);
			}

			else if (FontSmoothType == 1)
			{
				MessageBox(0, "It appears that Standard font smoothing is enabled.\n"
							"\n"
							"In order for OpenHoldem to reliably\n"
							"extract information from the poker client\n"
							"you should disable Font Smoothing.",
							"Caution: Font smoothing is enabled", MB_OK|MB_ICONWARNING);
			}
		}
	}
}

void CConfigurationCheck::CheckForMissingMSVCRT()
{
	CString p_szKeyCRT = "SOFTWARE\\Microsoft\\DevDiv\\VC\\Servicing\\8.0\\RED\\1033\\";
	CString p_szNameCRT = "Install";

	bool installed = false;

	if (atoi(GetValue("HKLM", 0, p_szKeyCRT, p_szNameCRT)) == 1)
	{
		installed = true;
	}

	if (installed == false)
	{
		MessageBox(0, "Unable to detect\n"
			"Microsoft Visual C++ 2005 redistributable runtime library.\n"
			"\n"
			"This library is necessary for Perl users.\n"
			"if you don't use Perl you may turn this warning off\n"
			"by not loading the perl interpreter by default.",
			"Caution: MSVCRT 8.0 missing", MB_OK|MB_ICONWARNING);
	}
}

void CConfigurationCheck::CheckForMissingActivePerl()
{
	CString p_szKeyAP = "SOFTWARE\\Activestate\\ActivePerl\\";

	if (!OpenKey("HKLM", p_szKeyAP))
	{
		MessageBox(0, "Unable to detect\n"
			"ActiveState ActivePerl.\n"
			"\n"
			"This version is required for Perl users.\n"
			"if you don't use Perl you may turn this warning off\n"
			"by not loading the perl interpreter by default.",
			"Caution: ActivePerl missing", MB_OK|MB_ICONWARNING);
	}
}

void CConfigurationCheck::CheckForPerlPath()
{
	CString p_szKeyP = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\";
	CString p_szNameP = "Path";

	CString path = GetValue("HKLM", 1, p_szKeyP, p_szNameP);

	if (path.Find("Perl") == -1)
	{
		MessageBox(0, "Path\n"
			"Perl was not correctly detected in your Path.\n"
			"\n"
			"If you don't use Perl you may turn this warning off\n"
			"by not loading the perl interpreter by default.",
			"Caution: Perl not found in %PATH%", MB_OK|MB_ICONWARNING);
	}
}
