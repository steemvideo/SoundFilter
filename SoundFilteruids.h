//------------------------------------------------------------------------------
// File: ContUIDs.h
//
// Desc: DirectShow sample code - CLSID's used by the contrast filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------



DEFINE_GUID(CLSID_SoundFilter, 
mmioFOURCC('s','o','u','n'),0x15a3, 0x4ece, 0x98, 0xc0, 0xd9, 0xcd, 0x27, 0x44, 0xf5, 0xa8);
// {89B2C28D-779F-4704-AD29-113B0977E8A5}
DEFINE_GUID(CLSID_SoundProperty,	 0x89b2c28d, 0x779f, 0x4704, 0xad, 0x29, 0x11, 0x3b, 0x9, 0x77,  0xe8, 0xa5);



#if (_ATL_VER < 0x0700)
  #pragma warning(disable: 4100) // unreferenced formal parameter
  #pragma warning(disable: 4189) // local variable is initialized but not referenced
#endif
