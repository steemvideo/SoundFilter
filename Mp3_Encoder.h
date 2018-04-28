// Mp3_Encoder.h: interface for the CMp3_Encoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MP3_ENCODER_H__EA55565C_E7CE_479F_B054_4E9E9AF8D2FA__INCLUDED_)
#define AFX_MP3_ENCODER_H__EA55565C_E7CE_479F_B054_4E9E9AF8D2FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "BladeMP3EncDLL.h"
#include "streams.h"
class CMp3_Encoder  
{
public:
	CMp3_Encoder();
	virtual ~CMp3_Encoder();
	BOOL init(DWORD dwSampleRate, BOOL bSaveMp3toFile, char *psavepath);
	BOOL encodermp3(long nInLen, BYTE *pInBuf, unsigned long &nOutLen, BYTE **OutBuf);
	void close();
	BOOL deinitstream(BYTE **pOutBuf, DWORD *dwWrite);
	
private:
	BEINITSTREAM			m_beInitStream;
	BEENCODECHUNK			m_beEncodeChunk;
	BEDEINITSTREAM			m_beDeinitStream;
	BECLOSESTREAM			m_beCloseStream;
	BEVERSION				m_beVersion;
	BEWRITEVBRHEADER		m_beWriteVBRHeader;
	BEWRITEINFOTAG			m_beWriteInfoTag;
	HBE_STREAM				m_hbeStream;
	HINSTANCE				m_hmp3DLL;
	PBYTE					m_pMP3Buffer;
//
	BOOL					m_bSaveMp3ToFile;
	char					*m_psavepath;
	CCritSec m_CritSecLock;

};

#endif // !defined(AFX_MP3_ENCODER_H__EA55565C_E7CE_479F_B054_4E9E9AF8D2FA__INCLUDED_)
