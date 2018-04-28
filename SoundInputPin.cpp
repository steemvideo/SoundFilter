#include "StdAfx.h"
#include "SoundInputPin.h"
#include "SoundTrans.h"
CSoundInputPin::CSoundInputPin(TCHAR* pObjectName, CSoundTrans* pFilter, HRESULT* phr, LPCWSTR pName) 
: CTransformInputPin(pObjectName, pFilter, phr, pName)
{
	m_bCanReconnectWhenActive = true;
	m_pSoundTrans = pFilter;
}

CSoundInputPin::~CSoundInputPin(void)
{
}

STDMETHODIMP CSoundInputPin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv,E_POINTER);

	if (riid == IID_IPinConnection) 
	{
		return GetInterface((IPinConnection *) this, ppv);
	} 
	else
	{
		return __super::NonDelegatingQueryInterface(riid, ppv);
	}
}



STDMETHODIMP CSoundInputPin::DynamicQueryAccept( 
	/* [in] */ const AM_MEDIA_TYPE *pmt)
{
	CAutoLock cObjectLock(m_pLock);
	HRESULT hr = __super::QueryAccept(pmt);
	if (FAILED(hr))
	{
		return hr;
	}
	hr =  m_pSoundTrans->QueryAcceptDownstream(pmt);
 
	m_pSoundTrans->SetReconnect();
	return hr;
}

STDMETHODIMP CSoundInputPin::NotifyEndOfStream( 
	/* [in] */ HANDLE hNotifyEvent)
{
	return S_OK;
}

STDMETHODIMP CSoundInputPin::IsEndPin( void) 
{
	return S_OK;
}

STDMETHODIMP CSoundInputPin::DynamicDisconnect( void)
{
	CAutoLock cObjectLock(m_pLock);
	Disconnect();
	return S_OK;
}

STDMETHODIMP CSoundInputPin::ReceiveConnection(
	IPin * pConnector,      // this is the initiating connecting pin
	const AM_MEDIA_TYPE *pmt   // this is the media type we will exchange
	)
{
	CheckPointer(pConnector,E_POINTER);
	CheckPointer(pmt,E_POINTER);
	ValidateReadPtr(pConnector,sizeof(IPin));
	ValidateReadPtr(pmt,sizeof(AM_MEDIA_TYPE));
	CAutoLock cObjectLock(m_pLock);

	/* Are we already connected */

	if(m_Connected) 
	{
		m_Connected->Release();
		m_Connected = NULL;
	}

	/* See if the filter is active */
	if (!IsStopped() && !m_bCanReconnectWhenActive) {
		return VFW_E_NOT_STOPPED;
	}

	HRESULT hr = CheckConnect(pConnector);
	if (FAILED(hr)) {
		// Since the procedure is already returning an error code, there
		// is nothing else this function can do to report the error.
		EXECUTE_ASSERT( SUCCEEDED( BreakConnect() ) );

#ifdef DXMPERF
		PERFLOG_RXCONNECT( pConnector, (IPin *) this, hr, pmt );
#endif // DXMPERF

		return hr;
	}

	/* Ask derived class if this media type is ok */

	CMediaType * pcmt = (CMediaType*) pmt;
	hr = CheckMediaType(pcmt);
	if (hr != NOERROR) {
		// no -we don't support this media type

		// Since the procedure is already returning an error code, there
		// is nothing else this function can do to report the error.
		EXECUTE_ASSERT( SUCCEEDED( BreakConnect() ) );

		// return a specific media type error if there is one
		// or map a general failure code to something more helpful
		// (in particular S_FALSE gets changed to an error code)
		if (SUCCEEDED(hr) ||
			(hr == E_FAIL) ||
			(hr == E_INVALIDARG)) {
				hr = VFW_E_TYPE_NOT_ACCEPTED;
		}

#ifdef DXMPERF
		PERFLOG_RXCONNECT( pConnector, (IPin *) this, hr, pmt );
#endif // DXMPERF

		return hr;
	}

	/* Complete the connection */

	m_Connected = pConnector;
	m_Connected->AddRef();
	hr = SetMediaType(pcmt);
	if (SUCCEEDED(hr)) {
		hr = CompleteConnect(pConnector);
		if (SUCCEEDED(hr)) {

#ifdef DXMPERF
			PERFLOG_RXCONNECT( pConnector, (IPin *) this, NOERROR, pmt );
#endif // DXMPERF

			return NOERROR;
		}
	}

	m_Connected->Release();
	m_Connected = NULL;

	// Since the procedure is already returning an error code, there
	// is nothing else this function can do to report the error.
	EXECUTE_ASSERT( SUCCEEDED( BreakConnect() ) );

#ifdef DXMPERF
	PERFLOG_RXCONNECT( pConnector, (IPin *) this, hr, pmt );
#endif // DXMPERF

	return hr;
}

HRESULT CSoundInputPin::SetMediaType(const CMediaType* mt)
{
	HRESULT hr=CTransformInputPin::SetMediaType(mt);
	return hr!=S_OK?hr:(init(*mt)?S_OK:E_FAIL);
}

bool CSoundInputPin::init(const CMediaType &mt)
{
	m_pSoundTrans->m_insf = mt;
	return true;

}
