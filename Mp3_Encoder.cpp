// Mp3_Encoder.cpp: implementation of the CMp3_Encoder class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <streams.h>
#include <windows.h>
#include <stdio.h>
#include <dvdmedia.h>
#include <MMReg.h>
#include <ks.h>
#include <ksmedia.h>
#include <initguid.h>
#include <olectl.h>
#include <process.h>
#include "Mp3_Encoder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma warning( disable : 4996 )
CMp3_Encoder::CMp3_Encoder()
{
	m_beInitStream = NULL;
	m_beEncodeChunk = NULL;
	m_beDeinitStream = NULL;
	m_beCloseStream = NULL;
	m_beVersion = NULL;
	m_beWriteVBRHeader = NULL;
	m_beWriteInfoTag = NULL;
	m_hbeStream = 0;
	m_hmp3DLL = NULL;
	m_pMP3Buffer = NULL;
	m_bSaveMp3ToFile = NULL;
	m_psavepath = NULL;
}

CMp3_Encoder::~CMp3_Encoder()
{
	close();
}

BOOL CMp3_Encoder::init(DWORD dwSampleRate, BOOL bSaveMp3toFile, char *psavepath)
{
	close();
	CAutoLock Lock(&m_CritSecLock);
	m_hmp3DLL = LoadLibrary(L"lame_enc.dll");
	if(!m_hmp3DLL)
		return FALSE;
	m_beInitStream	= (BEINITSTREAM) GetProcAddress(m_hmp3DLL, TEXT_BEINITSTREAM);
	m_beEncodeChunk	= (BEENCODECHUNK) GetProcAddress(m_hmp3DLL, TEXT_BEENCODECHUNK);
	m_beDeinitStream	= (BEDEINITSTREAM) GetProcAddress(m_hmp3DLL, TEXT_BEDEINITSTREAM);
	m_beCloseStream	= (BECLOSESTREAM) GetProcAddress(m_hmp3DLL, TEXT_BECLOSESTREAM);
	m_beVersion		= (BEVERSION) GetProcAddress(m_hmp3DLL, TEXT_BEVERSION);
	m_beWriteVBRHeader= (BEWRITEVBRHEADER) GetProcAddress(m_hmp3DLL,TEXT_BEWRITEVBRHEADER);
	m_beWriteInfoTag  = (BEWRITEINFOTAG) GetProcAddress(m_hmp3DLL,TEXT_BEWRITEINFOTAG);
	if(!m_beInitStream || !m_beEncodeChunk || !m_beDeinitStream || !m_beCloseStream || !m_beVersion || !m_beWriteInfoTag || !m_beWriteVBRHeader)
	{
		m_beInitStream = NULL;
		m_beEncodeChunk = NULL;
		m_beDeinitStream = NULL;
		m_beCloseStream = NULL;
		m_beVersion = NULL;
		m_beWriteVBRHeader = NULL;
		m_beWriteInfoTag = NULL;
		FreeLibrary(m_hmp3DLL);
		m_hmp3DLL = NULL;
		return FALSE;
	}
	BE_VERSION	Version			={0,};
	BE_CONFIG	beConfig		={0,};
	memset(&beConfig,0,sizeof(beConfig));					// clear all fields

	// use the LAME config structure
	beConfig.dwConfig = BE_CONFIG_LAME;

	// this are the default settings for testcase.wav
	beConfig.format.LHV1.dwStructVersion	= 1;
	beConfig.format.LHV1.dwStructSize		= sizeof(beConfig);		
	beConfig.format.LHV1.dwSampleRate		= dwSampleRate;				// INPUT FREQUENCY
	beConfig.format.LHV1.dwReSampleRate		= 0;					// DON"T RESAMPLE
	beConfig.format.LHV1.nMode				= BE_MP3_MODE_JSTEREO;	// OUTPUT IN STREO
	beConfig.format.LHV1.dwBitrate			= 128;					// MINIMUM BIT RATE
	beConfig.format.LHV1.nPreset			= LQP_R3MIX;		// QUALITY PRESET SETTING
	beConfig.format.LHV1.dwMpegVersion		= MPEG1;				// MPEG VERSION (I or II)
	beConfig.format.LHV1.dwPsyModel			= 0;					// USE DEFAULT PSYCHOACOUSTIC MODEL 
	beConfig.format.LHV1.dwEmphasis			= 0;					// NO EMPHASIS TURNED ON
	beConfig.format.LHV1.bOriginal			= TRUE;					// SET ORIGINAL FLAG
	beConfig.format.LHV1.bWriteVBRHeader	= TRUE;					// Write INFO tag

//	beConfig.format.LHV1.dwMaxBitrate		= 320;					// MAXIMUM BIT RATE
//	beConfig.format.LHV1.bCRC				= TRUE;					// INSERT CRC
//	beConfig.format.LHV1.bCopyright			= TRUE;					// SET COPYRIGHT FLAG	
//	beConfig.format.LHV1.bPrivate			= TRUE;					// SET PRIVATE FLAG
//	beConfig.format.LHV1.bWriteVBRHeader	= TRUE;					// YES, WRITE THE XING VBR HEADER
//	beConfig.format.LHV1.bEnableVBR			= TRUE;					// USE VBR
//	beConfig.format.LHV1.nVBRQuality		= 5;					// SET VBR QUALITY
	beConfig.format.LHV1.bNoRes				= TRUE;					// No Bit resorvoir

// Init the MP3 Stream
	DWORD dwMP3Buffer = 0;
	DWORD dwSamples = 0;
	BE_ERR err = m_beInitStream(&beConfig, &dwSamples, &dwMP3Buffer, &m_hbeStream);
	if(err != BE_ERR_SUCCESSFUL)
	{
		m_beInitStream = NULL;
		m_beEncodeChunk = NULL;
		m_beDeinitStream = NULL;
		m_beCloseStream = NULL;
		m_beVersion = NULL;
		m_beWriteVBRHeader = NULL;
		m_beWriteInfoTag = NULL;
		FreeLibrary(m_hmp3DLL);
		m_hmp3DLL = NULL;
		return FALSE;
	}
	m_pMP3Buffer = new BYTE[dwMP3Buffer];
	m_bSaveMp3ToFile = bSaveMp3toFile;
	if(m_bSaveMp3ToFile)
	{
		int nlen = strlen(psavepath) + 1;
		m_psavepath = new char[nlen];
		memset(m_psavepath, 0, nlen);
		strcpy(m_psavepath, psavepath);
	}
	return TRUE;
}

BOOL CMp3_Encoder::encodermp3(long nInLen, BYTE *pInBuf, unsigned long &nOutLen, BYTE **OutBuf)
{
	if(m_hbeStream == 0)
		return FALSE;
	CAutoLock Lock(&m_CritSecLock);
	unsigned long nhavelen = 0;
	BE_ERR err = m_beEncodeChunk(m_hbeStream, nInLen/2, (short *)pInBuf, m_pMP3Buffer, &nhavelen);
	if(err != BE_ERR_SUCCESSFUL)
		return FALSE;
	*OutBuf = m_pMP3Buffer;
	nOutLen = nhavelen;
	return TRUE;
}	

BOOL CMp3_Encoder::deinitstream(BYTE **pOutBuf, DWORD *dwWrite)
{
	CAutoLock Lock(&m_CritSecLock);
	if(m_hbeStream && m_beDeinitStream)
	{
		BE_ERR err = m_beDeinitStream(m_hbeStream, m_pMP3Buffer, dwWrite);
		if(err == BE_ERR_SUCCESSFUL)
		{
			*pOutBuf = m_pMP3Buffer;
			return TRUE;
		}
	}
	return FALSE;
}


void CMp3_Encoder::close()
{
	CAutoLock Lock(&m_CritSecLock);
	if(m_hbeStream && m_beCloseStream)
	{
		m_beCloseStream( m_hbeStream );
	}
	if (m_bSaveMp3ToFile && m_beWriteInfoTag && m_hbeStream && m_psavepath)
	{
		m_beWriteInfoTag(m_hbeStream, m_psavepath);
	}
	m_hbeStream = 0;
	m_beInitStream = NULL;
	m_beEncodeChunk = NULL;
	m_beDeinitStream = NULL;
	m_beCloseStream = NULL;
	m_beVersion = NULL;
	m_beWriteVBRHeader = NULL;
	m_beWriteInfoTag = NULL;
	if(m_hmp3DLL)
		::FreeLibrary(m_hmp3DLL);
	m_hmp3DLL = NULL;
	if(m_psavepath)
		delete []m_psavepath;
	m_psavepath = NULL;
	if(m_pMP3Buffer)
		delete []m_pMP3Buffer;
	m_pMP3Buffer = NULL;
}