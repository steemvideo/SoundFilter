#pragma once
#include "streams.h"
class CSoundTrans;

class CSoundInputPin: public CTransformInputPin,
	public IPinConnection
{
	friend class CSoundTrans;

	CSoundTrans *m_pSoundTrans;
public:
	CSoundInputPin(TCHAR* pObjectName, CSoundTrans* pFilter, HRESULT* phr, LPCWSTR pName);
	~CSoundInputPin(void);
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	STDMETHODIMP DynamicQueryAccept( 
		/* [in] */ const AM_MEDIA_TYPE *pmt);

	STDMETHODIMP NotifyEndOfStream( 
		/* [in] */ HANDLE hNotifyEvent);

	STDMETHODIMP IsEndPin( void) ;

	STDMETHODIMP DynamicDisconnect( void);
	
	STDMETHODIMP ReceiveConnection(IPin * pConnector,      // this is the initiating connecting pin
		const AM_MEDIA_TYPE *pmt   // this is the media type we will exchange
	);

	HRESULT SetMediaType(const CMediaType* mt);
	bool init(const CMediaType &mt);
		//////////////////////////
};
