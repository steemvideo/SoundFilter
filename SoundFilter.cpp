// SoundFilter.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif
#include <streams.h>
#include "SoundFilteruids.h"
#include "SoundTrans.h"
#include "SoundProperty.h"


// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance


const AMOVIESETUP_MEDIATYPE sudPinTypes_sound =
{
	&MEDIATYPE_Audio,       // Major type
	&MEDIASUBTYPE_PCM       // Minor type
};


const AMOVIESETUP_PIN psoundPins[] =
{
	{
		L"Input",           // String pin name
			FALSE,              // Is it rendered
			FALSE,              // Is it an output
			FALSE,              // Allowed none
			FALSE,              // Allowed many
			&CLSID_NULL,        // Connects to filter
			L"Output",          // Connects to pin
			1,                  // Number of types
			&sudPinTypes_sound },     // The pin details
		{ L"Output",          // String pin name
		FALSE,              // Is it rendered
		TRUE,               // Is it an output
		FALSE,              // Allowed none
		FALSE,              // Allowed many
		&CLSID_NULL,        // Connects to filter
		L"Input",           // Connects to pin
		1,                  // Number of types
		&sudPinTypes_sound        // The pin details
		}
};

const AMOVIESETUP_FILTER sudsoundtrans =
{
	&CLSID_SoundFilter,			// Filter CLSID
	L"SoundFilter",				// Filter name
	MERIT_DO_NOT_USE,			// Its merit
	2,							// Number of pins
	psoundPins					// Pin details
};

CFactoryTemplate g_Templates[] = 
{

	{ 
		L"SoundFilter", 
			&CLSID_SoundFilter,
			CSoundTrans::CreateInstance, 
			NULL, 
			&sudsoundtrans 
	}
	,
	{ 
		L"Sound Contrast Property Page", 
			&CLSID_SoundProperty, 
			CSoundProperty::CreateInstance 
	}
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);



STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);

}


extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  dwReason, 
					  LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
#ifdef _MANAGED
#pragma managed(pop)
#endif

