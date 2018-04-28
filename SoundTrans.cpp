// SoundTrans.cpp: implementation of the CSoundTrans class.

#include "stdafx.h"
//#include <streams.h>
//#include <windows.h>
//#include <stdio.h>
//#include <dvdmedia.h>
//#include <MMReg.h>
//#include <ks.h>
//#include <ksmedia.h>
//#include <initguid.h>
//#include <olectl.h>
//#include <process.h>
//#include <math.h>

#include "SoundFilteruids.h"
#include "iSound.h"


#include "Mp3_Encoder.h"
#include "SoundTrans.h"

#include "SoundInputPin.h"
#include <atlbase.h>




#if (NTDDI_VERSION < NTDDI_WINXP)
#define KSAUDIO_SPEAKER_DIRECTOUT       0
#endif
#pragma warning(disable:4238)  // nonstd extension used: class rvalue used as lvalue
#pragma warning(disable:4244)
#pragma warning( disable : 4996)

const	double		Q = 1.2247449;
#define	M_PI		3.14159265358979323846
#define	INT_MAX		2147483647
#define WRITE_MAX	1024*1024*2

#define CHAR_MIN_NUM	0
#define CHAR_MAX_NUM	255
//#define SHORT_MIN	-32768
//#define SHORT_MAX	32767 
#define SHORT_MIN	-32760
#define SHORT_MAX	32760 
#define INT24_MAX	0x7FFFFF
#define INT24_MIN	(1<<23)-1
#define INT32_MAX 0x7fffffff	
#define INT32_MIN (-0x7fffffff-1)	



#define CHAR_100 100
CEqFilter::CEqFilter()
{
	 neq[0] = neq[1] = neq[2] = neq[3] = neq[4] = neq[5] = neq[6] = neq[7] = neq[8] = neq[9] = CHAR_100;
	 nf[0] = 3125;		nf[1] = 6250;		nf[2] = 12500;
	 nf[3] = 25000;		nf[4] = 50000;		nf[5] = 100000;
	 nf[6] = 200000;	nf[7] = 400000;		nf[8] = 800000;
	 nf[9] = 1600000;
}

CEqFilter::~CEqFilter()
{
	
}

inline float db2value(float db,int mul) 
{
	return powf(10.0f,db/(mul*20.0f));
}
void CEqFilter::bp2(float* a, float* b, double fc, double q)
{
	double th=2*M_PI*fc;
	double C=(1-tan(th*q/2))/(1+tan(th*q/2));
	static const double bp2mult=1;

	a[0]=float(bp2mult*(1+C)*cos(th));
	a[1]=float(bp2mult*-1*C);

	b[0]=float(bp2mult*(1.0-C)/2.0);
	b[1]=float(bp2mult*-1.0050);
}

void CEqFilter::reset()
{
	memset(wq, 0, sizeof(wq));
}

void CEqFilter::init(double dhz, int *ndb)
{
	reset();
	static const double qmult = 1.0;
	if(ndb != NULL)
	{
		int *ndb0 = ndb;
		for (int m = 0; m < KM; m++, ndb0++)
		{
			neq[m] = *ndb0;
		}
	}
	for(int i=0; i<AF_NCH; i++)
	{
		for(int j=0;j<KM;j++)
		{
			float db=(HIGHDB - LOWDB)*neq[j]/200.0f+LOWDB;
			float db21 = db2value(db,100); 
			g[i][j]=(float)(qmult*( db21 -1.0));
		}
	}
	K=KM;
	while((nf[K-1]/100.0)>dhz/2.3)
		K--;
	for(int k=0;k<K;k++)
		bp2(a[k],b[k],(nf[k]/100.0)/dhz,Q);
}

void CEqFilter::geteq(int *ndb)
{
	if(ndb != NULL)
	{
		int *ndb0 = ndb;
		for (int m = 0; m < KM; m++, ndb0++)
		{
			*ndb0 = neq[m];
		}
	}
}
void CEqFilter::process(int ntag, int nchannels, BYTE *samples, long numsamples)
{
	switch (ntag)
	{
		case BYTESAMPLE16:process_16v((short*)samples,numsamples,nchannels);break;
		case BYTESAMPLE24:process_24v(samples,numsamples,nchannels);break;
		case BYTESAMPLE32:process_32v((int *)samples,numsamples,nchannels);break;
	}
}

void CEqFilter::process_16v(short *samples, size_t numsamples, int nchannels)
{
	unsigned int ci=nchannels;
	while (ci--)
	{
		const float *g=this->g[ci];
		short *inout=samples+ci;
		short *end=inout+nchannels*numsamples;
		while (inout<end)
		{
			float yt=eq_16v(g,*inout,ci);
			*inout=limit_16v(yt);
			inout+=nchannels;
		}
	}
}

float CEqFilter::eq_16v(const float *g, short in, int ci)
{
	float yt=(float)in;
	for (int k=0;k<K;k++)
	{
		float *wq = this->wq[ci][k];
		float w=yt*b[k][0] + wq[0]*a[k][0] + wq[1]*a[k][1];
		yt+=(w + wq[1]*b[k][1])*g[k];
		wq[1] = wq[0];
		wq[0] = w;
	}
	return yt;
}

float CEqFilter::limit_16v(float yt)
{
   if(yt < SHORT_MIN)
    return SHORT_MIN;
   else if(yt>SHORT_MAX)
    return SHORT_MAX;
   else
    return (short)yt;
}

void CEqFilter::process_24v(BYTE *samples, size_t numsamples, int nchannels)
{
	unsigned int ci=nchannels;
	while (ci--)
	{
		const float *g=this->g[ci];
		BYTE *inout=samples+ci*3;
		BYTE *end=inout+nchannels*numsamples*3;
		while (inout<end)
		{
			int i_inout = ((int)inout[0]<<16) + ((int)inout[1]<<8) + inout[2];
			float yt=eq_24v(g, i_inout, ci);
			i_inout = (int)limit_24v(yt);
			BYTE *pb = (BYTE*)&i_inout;
			inout[0] = pb[1] ; inout[1] = pb[2] ; 	inout[2] = pb[3];
			inout+=nchannels*3;
		}
	}
}

float CEqFilter::eq_24v(const float *g, int in, int ci)
{
	float yt=(float)in;
	for (int k=0;k<K;k++)
	{
		float *wq = this->wq[ci][k];
		float w=yt*b[k][0] + wq[0]*a[k][0] + wq[1]*a[k][1];
		yt+=(w + wq[1]*b[k][1])*g[k];
		wq[1] = wq[0];
		wq[0] = w;
	}
	return yt;
}

float CEqFilter::limit_24v(float yt)
{
   if(yt < INT24_MIN)
    return INT24_MIN;
   else if(yt > INT24_MAX)
    return INT24_MAX;
   else
    return yt;
}

void CEqFilter::process_32v(int *samples, size_t numsamples, int nchannels)
{
	unsigned int ci=nchannels;
	while (ci--)
	{
		const float *g=this->g[ci];
		int *inout=samples+ci;
		int *end=inout+nchannels*numsamples;
		while (inout<end)
		{
			float yt=eq_32v(g,*inout,ci);
			*inout=limit_32v(yt);
			inout+=nchannels;
		}
	}
}

float CEqFilter::eq_32v(const float *g, int in, int ci)
{
	float yt=(float)in;
	for (int k=0;k<K;k++)
	{
		float *wq = this->wq[ci][k];
		float w=yt*b[k][0] + wq[0]*a[k][0] + wq[1]*a[k][1];
		yt+=(w + wq[1]*b[k][1])*g[k];
		wq[1] = wq[0];
		wq[0] = w;
	}
	return yt;
}

int CEqFilter::limit_32v(float yt)
{
   if(yt < INT32_MIN)
    return INT32_MIN;
   else if( yt> INT32_MAX)
    return INT32_MAX;
   else
    return yt;
}


bool CEqFilter::ifeq()
{
	bool bifeq = false;
	for (int i = 0; i < 10; i++) 
	{
		if(neq[i] != CHAR_100)
		{
			bifeq = true;
			break;
		}
	}
	return bifeq;
}

ISoundtrans *g_ISoundtrans = NULL;

CSoundTrans::CSoundTrans(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr) :
			CTransformFilter(tszName, punk, CLSID_SoundFilter),
			m_schannel(S_DEFAULT),
			m_rtAudioTimeShift(0),
			m_rtNextStart(0),
			m_rtNextStop(1),
			m_bmute(false),
			m_laddsound(10),
			m_lbalance(0),
			m_bseq(false),
			m_nastreamcur(0),
			m_nastreamcount(0),
			m_wavfile(NULL),
			m_wavlen(0),
			m_bReconnect(false),
			m_bConvert(false),
			m_bmp3stero(false)
			//m_newSegment(0)
{
	memset(m_pwavpath, 0, MAX_PATH);
	memcpy(m_wavriff.head.szid, "RIFF", 4);
	memcpy(m_wavriff.sz, "WAVE", 4);
	m_wavriff.head.szsize = sizeof(m_wavriff.sz);
	memcpy(m_wavface.head.szid, "fact", 4);
	memcpy(m_wavface.szface, "QVOD", 4);
	m_wavface.head.szsize = sizeof(m_wavface.szface);
	m_pMp3Encoder = NULL;
	m_eAudioType = F_NO;
	g_ISoundtrans = this;
	//////////add by zhaokui /////////////
	memset(&m_insf,0,sizeof(m_insf));
	memset(&m_outsf,0,sizeof(m_outsf));
	memset(&m_Cursf,0,sizeof(m_Cursf));
	m_bMixer = false;
	m_DstChannels = 0;
	m_SrcChannels = 0;
	m_inmask = 0;
	m_outmask = 0;
	m_outConfigOption = 13;
	mixerPCM16 = NULL;
	m_rtStartProc = 0;
	///////////////////////////

}
CBasePin * CSoundTrans::GetPin(int n)
{
	HRESULT hr = S_OK;

	if (m_pInput == NULL) {

		m_pInput = new CSoundInputPin(NAME("Transform input pin"),
			this,              // Owner filter
			&hr,               // Result code
			L"XForm In");      // Pin name


		//  Can't fail
		ASSERT(SUCCEEDED(hr));
		if (m_pInput == NULL) {
			return NULL;
		}
		m_pOutput = (CTransformOutputPin *)
			new CTransformOutputPin(NAME("Transform output pin"),
			this,            // Owner filter
			&hr,             // Result code
			L"XForm Out");   // Pin name


		// Can't fail
		ASSERT(SUCCEEDED(hr));
		if (m_pOutput == NULL) {
			delete m_pInput;
			m_pInput = NULL;
		}
	}

	// Return the appropriate pin

	if (n == 0) {
		return m_pInput;
	} else
		if (n == 1) {
			return m_pOutput;
		} else {
			return NULL;
		}
}
#define BeginEnumPins(pBaseFilter, pEnumPins, pPin) \
{CComPtr<IEnumPins> pEnumPins; \
	if(pBaseFilter && SUCCEEDED(pBaseFilter->EnumPins(&pEnumPins))) \
{ \
	for(CComPtr<IPin> pPin; S_OK == pEnumPins->Next(1, &pPin, 0); pPin = NULL) \
{ \

#define EndEnumPins }}}
class CPinInfo : public PIN_INFO
{
public:
	CPinInfo() {
		pFilter = NULL;
	}
	~CPinInfo() {
		if(pFilter) pFilter->Release();
	}
};
IBaseFilter* GetFilterFromPin(IPin* pPin)
{
	if(!pPin) return NULL;
	IBaseFilter* pBF = NULL;
	CPinInfo pi;
	if(pPin && SUCCEEDED(pPin->QueryPinInfo(&pi)))
		pBF = pi.pFilter;
	return(pBF);
}
int CountPins(IBaseFilter* pBF, int& nIn, int& nOut, int& nInC, int& nOutC)
{
	nIn = nOut = 0;
	nInC = nOutC = 0;

	BeginEnumPins(pBF, pEP, pPin)
	{
		PIN_DIRECTION dir;
		if(SUCCEEDED(pPin->QueryDirection(&dir)))
		{
			CComPtr<IPin> pPinConnectedTo;
			pPin->ConnectedTo(&pPinConnectedTo);

			if(dir == PINDIR_INPUT) {
				nIn++;
				if(pPinConnectedTo) nInC++;
			}
			else if(dir == PINDIR_OUTPUT) {
				nOut++;
				if(pPinConnectedTo) nOutC++;
			}
		}
	}
	EndEnumPins

		return(nIn+nOut);
}
DEFINE_GUID(CLSID_ReClock,
			0x9dc15360, 0x914c, 0x46b8, 0xb9, 0xdf, 0xbf, 0xe6, 0x7f, 0xd3, 0x6c, 0x6a);

bool IsAudioWaveRenderer(IBaseFilter* pBF)
{
	int nIn, nOut, nInC, nOutC;
	CountPins(pBF, nIn, nOut, nInC, nOutC);

	if(nInC > 0 && nOut == 0 && CComQIPtr<IBasicAudio>(pBF))
	{
		BeginEnumPins(pBF, pEP, pPin)
		{
			AM_MEDIA_TYPE mt;
			if(S_OK != pPin->ConnectionMediaType(&mt))
				continue;

			FreeMediaType(mt);

			return(!!(mt.majortype == MEDIATYPE_Audio)
				/*&& mt.formattype == FORMAT_WaveFormatEx*/);
		}
		EndEnumPins
	}

	CLSID clsid;
	memcpy(&clsid, &GUID_NULL, sizeof(clsid));
	pBF->GetClassID(&clsid);

	return(clsid == CLSID_DSoundRender || clsid == CLSID_AudioRender || clsid == CLSID_ReClock
		/*|| clsid == __uuidof(CNullAudioRenderer) || clsid == __uuidof(CNullUAudioRenderer)*/);
}
CLSID GetCLSID(IBaseFilter* pBF)
{
	CLSID clsid = GUID_NULL;
	if(pBF) pBF->GetClassID(&clsid);
	return(clsid);
}
class __declspec(uuid("AEFA5024-215A-4FC7-97A4-1043C86FD0B8"))
	MatrixMixer {};

bool CSoundTrans::CanReconnect()
{
	IPin *pPin = m_pOutput->GetConnected();
	CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPin);

	if(IsAudioWaveRenderer(pBF) || GetCLSID(pBF) == __uuidof(MatrixMixer))
	{
		return false;
	}
	return true;
}

HRESULT CSoundTrans::QueryAcceptDownstream(const AM_MEDIA_TYPE* pmt)
{
	HRESULT hr;


	if (m_pOutput && m_pOutput->IsConnected())
	{
		if (!CanReconnect())
		{
			return S_OK;
		}

		CComPtr<IPinConnection> pPinConnect ;
		hr = m_pOutput->GetConnected()->QueryInterface(IID_IPinConnection,(void**)&pPinConnect);

		if (FAILED(hr))
		{
			return hr;
		}
		hr =pPinConnect->DynamicQueryAccept(pmt);

		if (FAILED(hr))
		{
			return hr;
		} 
	}
	return S_OK;
}
/*
HRESULT CSoundTrans::ReconnectOutput(size_t numsamples, CMediaType& mt)
{
	HRESULT hr;

	CBaseInputPin * inputPin = m_pInput;

	CComPtr<IMediaSample>pOut;
	CComPtr<IPinConnection> pPinConnect ;
	if (m_pOutput->IsConnected())
	{
		hr = m_pOutput->GetConnected()->QueryInterface(IID_IPinConnection,(void**)&pPinConnect);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	hr =pPinConnect->DynamicQueryAccept(&mtNew);
	//hr = m_pOutput->GetConnected()->QueryAccept(&mtNew);

	if (FAILED(hr))
		return hr;

	hr = m_pOutput->GetConnected()->ReceiveConnection(m_pOutput, &mtNew);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pOutput->GetDeliveryBuffer(&pOut, NULL, NULL, 0);

	if (FAILED(hr))
	{
		return hr;
	}


	//以下目的在于，取出已重连好的下级inpin的媒体类型 和当前要设置的mtNew作比较
	CComQIPtr<IMemInputPin> pPin = m_pOutput->GetConnected();
	if(!pPin) 
		return E_NOINTERFACE;

	CComPtr<IMemAllocator> pAllocator;
	if(FAILED(hr = pPin->GetAllocator(&pAllocator)) || !pAllocator) 
		return hr;
	ALLOCATOR_PROPERTIES props, actual;
	if(FAILED(hr = pAllocator->GetProperties(&props)))
		return hr;

	CComPtr<IMemAllocator> pAllocatorSrc;
	if(FAILED(hr = inputPin->GetAllocator(&pAllocatorSrc)) || !pAllocatorSrc) 
		return hr;

	ALLOCATOR_PROPERTIES propsSrc;
	if(FAILED(hr = pAllocatorSrc->GetProperties(&propsSrc)))
		return hr;

	long cbBuffer = propsSrc.cbBuffer;


	if(propsSrc.cbBuffer > props.cbBuffer && propsSrc.cBuffers > props.cBuffers)
	{
		props.cBuffers = propsSrc.cBuffers;
		props.cbBuffer = propsSrc.cbBuffer;

		if(FAILED(hr = m_pOutput->DeliverBeginFlush())
			|| FAILED(hr = m_pOutput->DeliverEndFlush())
			|| FAILED(hr = pAllocator->Decommit())
			|| FAILED(hr = pAllocator->SetProperties(&props, &actual))
			|| FAILED(hr = pAllocator->Commit()))
			return hr;

		if(props.cBuffers > actual.cBuffers || props.cbBuffer > actual.cbBuffer)
		{
			NotifyEvent(EC_ERRORABORT, hr, 0);
			return E_FAIL;
		}
	}

	return S_OK;
}
*/

HRESULT CSoundTrans::ReconnectOutput(size_t numsamples,CMediaType& outmt)
{
	IPin *cpin=m_pOutput->GetConnected();
	if (!cpin) return E_NOINTERFACE;

	WAVEFORMATEX *outwfe=(WAVEFORMATEX*)outmt.Format();
	//WAVEFORMATEX *inwfe=(WAVEFORMATEX*)inmt.Format();
	long cbBuffer=long(numsamples*outwfe->nBlockAlign) ;
	//AM_MEDIA_TYPE *ppMediaType = NULL;
	//pOut->GetMediaType(&ppMediaType);
	//WAVEFORMATEX *outwfe=(WAVEFORMATEX*)ppMediaType->pbFormat;

	if (outmt!=m_pOutput->CurrentMediaType()|| cbBuffer>actual.cbBuffer)
	{
		///////////////
		if (cbBuffer>actual.cbBuffer)
		{
			CComPtr<IMemInputPin> pPin;
			if (FAILED(cpin->QueryInterface(IID_IMemInputPin,(void**)&pPin)))
				return E_NOINTERFACE;

			HRESULT hr;
			CComPtr<IMemAllocator> pAllocator;
			if (FAILED(hr=pPin->GetAllocator(&pAllocator)) || !pAllocator)
				return S_OK;//hr; Avisynth GetSample filter can't handle format changes

			ALLOCATOR_PROPERTIES props;
			if (FAILED(hr=pAllocator->GetProperties(&props)))
				return hr;

			props.cBuffers=4;
			props.cbBuffer=cbBuffer*3/2;

			hr=m_pOutput->DeliverBeginFlush();
			hr=m_pOutput->DeliverEndFlush();
			hr=pAllocator->Decommit();//VFW_E_ALREADY_COMMITTED
			hr=pAllocator->SetProperties(&props,&actual);
			hr=pAllocator->Commit();

			//if (FAILED(hr=m_pOutput->DeliverBeginFlush()) ||
			//	FAILED(hr=m_pOutput->DeliverEndFlush()) ||
			//	FAILED(hr=pAllocator->Decommit()) ||
			//	FAILED(hr=pAllocator->SetProperties(&props,&actual)) ||
			//	FAILED(hr=pAllocator->Commit()))
			//	return hr;

			if (props.cBuffers>actual.cBuffers || props.cbBuffer>actual.cbBuffer)
			{
				NotifyEvent(EC_ERRORABORT,hr,0);
				return E_FAIL;
			}
		}
		////////////////////////

		m_bReconnect = true;
		return S_OK;
	}
	m_bReconnect = false;
	return S_FALSE;
}
#define REF_SECOND_MULT 10000000LL
bool CSoundTrans::Mix(IMediaSample *insample,IMediaSample *outsample, size_t nSamples,bool bCalMatirx)
{
	BYTE* pDataIn;
	BYTE* pDataOut;

	HRESULT hr;
	//copy data
	if(FAILED(hr = insample->GetPointer(&pDataIn)))return hr;
	if(FAILED(hr = outsample->GetPointer(&pDataOut)))return hr;

	long nlen = insample->GetActualDataLength();
	long nlen1 = outsample->GetActualDataLength();

	 outsample->SetMediaTime(NULL,NULL); 
	 if(nSamples == 0)
	 {
		 return false;
	 }

	 //REFERENCE_TIME offset=0;

	 //REFERENCE_TIME rtDurProc=REF_SECOND_MULT*nSamples/44100;

	 //REFERENCE_TIME rtStart=m_rtStartProc+offset; 
	 //REFERENCE_TIME rtStop=m_rtStartProc+offset+rtDurProc;
	 //m_rtStartProc+=rtDurProc;
 
	//hr = outsample->SetTime(&rtStart, &rtStop);
	outsample->SetPreroll(FALSE);
	outsample->SetDiscontinuity(m_fDiscontinuity); m_fDiscontinuity = false;
	outsample->SetSyncPoint(TRUE);

	

	

	if(m_outConfigOption == 16 || m_SrcChannels == m_DstChannels)
	{
		return true;
	}
	//if(m_Cursf.nchannels != m_insf.nchannels  && m_bConvert)
	//{
	//	m_bConvert = false;
	//	return true;
	//}

	size_t dstlen=nSamples*m_Cursf.nchannels*m_Cursf.bitsPerSample()/8;
	//size_t dstlen=nSamples*m_DstChannels*2;

	long Datalen = outsample->GetSize();

	

	memset(pDataOut,0,Datalen);

	hr = outsample->SetActualDataLength((long)dstlen);
	
	if(!mixerPCM16)
	{
		mixerPCM16 = new Mixer<int32_t,1<<16,4>;
	}
	mixerPCM16->process(m_insf,m_outsf,( int16_t*&)pDataIn,( int16_t*&)pDataOut,nSamples,m_outConfigOption,&matrixPtr,&m_inmask,&m_outmask,bCalMatirx);

	return true;
}
CSoundTrans::~CSoundTrans()
{
	CloseWavfile();
	if(mixerPCM16 != NULL)
	{
		delete mixerPCM16 ;
		mixerPCM16 = NULL;
	}
	m_eAudioType = F_NO;
	g_ISoundtrans = NULL;
}

template<class T>
T clamp(double s, T smin, T smax)
{
	if(s < -1) s = -1;
	else if(s > 1) s = 1;
	T t = (T)(s * smax);
	if(t < smin) t = smin;
	else if(t > smax) t = smax;
	return t;
}

CUnknown * WINAPI CSoundTrans::CreateInstance(LPUNKNOWN punk, HRESULT *phr) 
{
    CSoundTrans *pNewObject = new CSoundTrans(NAME("SoundTrans"), punk, phr);
    if (pNewObject == NULL)
	{
        if (phr)
            *phr = E_OUTOFMEMORY;
    }
    return pNewObject;
} 

STDMETHODIMP CSoundTrans::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    if (riid == IID_ISoundtrans) 
	{
        return GetInterface((ISoundtrans *) this, ppv);

    } 
	else if (riid == IID_ISpecifyPropertyPages)
	{
		HRESULT hr = GetInterface((ISpecifyPropertyPages *) this, ppv);
        return hr;

    }
	else 
	{
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

}
#define  WAVE_FORMAT_DOLBY_AC3_SPDIF            0x0092
HRESULT CSoundTrans::CheckInputType(const CMediaType *mtIn)
{
	
    if (*mtIn->Type() != MEDIATYPE_Audio) 
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
	if(*mtIn->Subtype() != MEDIASUBTYPE_PCM)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	WAVEFORMATEX *pformat = (WAVEFORMATEX *)mtIn->pbFormat;
	if(mtIn->formattype == FORMAT_WaveFormatEx && (pformat->nChannels > 2) && (pformat->wFormatTag != WAVE_FORMAT_EXTENSIBLE))
		return VFW_E_INVALIDMEDIATYPE;
	if(mtIn->formattype == FORMAT_WaveFormatEx
		&& (pformat->wBitsPerSample == 8
		|| pformat->wBitsPerSample == 16
		|| pformat->wBitsPerSample == 24
		|| pformat->wBitsPerSample == 32)
		&& (   pformat->wFormatTag == WAVE_FORMAT_PCM
		|| pformat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT
		|| pformat->wFormatTag == WAVE_FORMAT_DOLBY_AC3_SPDIF
		|| pformat->wFormatTag == WAVE_FORMAT_EXTENSIBLE))
	{
		memcpy(&m_wavformat, pformat, min(sizeof(WAVEFORMATEX)+pformat->cbSize, sizeof(m_wavformat)));
		m_eqfilter.init(m_wavformat.Format.nSamplesPerSec, NULL);
		
	
		return S_OK;
	}
	else
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
}


HRESULT CSoundTrans::CheckConnect(PIN_DIRECTION dir,IPin *pPin)
{
	HRESULT res=E_UNEXPECTED;
	switch (dir)
	{
	case PINDIR_INPUT:
		res=S_OK;
		break;
	case PINDIR_OUTPUT:
		TsampleFormat outsf=getOutsf();
		res=S_OK;
		break;
	};
	return res==S_OK?CTransformFilter::CheckConnect(dir,pPin):res;
}

TsampleFormat CSoundTrans::getOutsf(void)
{
	m_outsf = m_insf;
	m_outsf.nchannels = m_DstChannels;
	return m_outsf;
}



HRESULT CSoundTrans::GetMediaType(int iPosition, CMediaType *mtOut)
{
    if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;
	WAVEFORMATEX* pwfe =NULL;
	if(m_bmp3stero)
	{
		*mtOut=m_insf.toCMediaType(true);
		pwfe = (WAVEFORMATEX*)mtOut->ReallocFormatBuffer(sizeof(WAVEFORMATEX));
		pwfe->nSamplesPerSec = 44100;
		pwfe->nChannels = 2;
		pwfe->wBitsPerSample = 16;

		//pwfe->cbSize = 0;
		//pwfe->wFormatTag = WAVE_FORMAT_PCM;
		//pwfe->nBlockAlign = pwfe->nChannels*pwfe->wBitsPerSample>>3;
		//pwfe->nAvgBytesPerSec = pwfe->nSamplesPerSec*pwfe->nBlockAlign;
	}	
	else
	{
		*mtOut = m_pInput->CurrentMediaType();
		mtOut->subtype = MEDIASUBTYPE_PCM;
		pwfe = (WAVEFORMATEX*)mtOut->ReallocFormatBuffer(sizeof(WAVEFORMATEX));
	}

	

	if(iPosition < 0) return E_INVALIDARG;
	if(iPosition > (pwfe->nChannels > 2 && pwfe->nChannels <= 6 ? 1 : 0)) return VFW_S_NO_MORE_ITEMS;

	if(!pwfe->wBitsPerSample) pwfe->wBitsPerSample = 16;

	pwfe->cbSize = 0;
	pwfe->wFormatTag = WAVE_FORMAT_PCM;
	pwfe->nBlockAlign = pwfe->nChannels*pwfe->wBitsPerSample>>3;
	pwfe->nAvgBytesPerSec = pwfe->nSamplesPerSec*pwfe->nBlockAlign;

	if(iPosition == 0 && pwfe->nChannels > 2 && pwfe->nChannels <= 6)
	{
		static DWORD chmask[] = 
		{
			KSAUDIO_SPEAKER_DIRECTOUT,
			KSAUDIO_SPEAKER_MONO,
			KSAUDIO_SPEAKER_STEREO,
			KSAUDIO_SPEAKER_STEREO|SPEAKER_FRONT_CENTER,
			KSAUDIO_SPEAKER_QUAD,
			KSAUDIO_SPEAKER_QUAD|SPEAKER_FRONT_CENTER,
			KSAUDIO_SPEAKER_5POINT1
		};
		
		WAVEFORMATEXTENSIBLE* pwfex = (WAVEFORMATEXTENSIBLE*)mtOut->ReallocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE));
		pwfex->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
		pwfex->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		pwfex->dwChannelMask = chmask[pwfex->Format.nChannels];
		pwfex->Samples.wValidBitsPerSample = pwfex->Format.wBitsPerSample;
		pwfex->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	}

	return S_OK;
}

HRESULT CSoundTrans::CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut)
{
    if (*mtOut->Type() != MEDIATYPE_Audio) 
    {
        return VFW_E_TYPE_NOT_ACCEPTED ;
    }
	if(*mtOut->Subtype() != MEDIASUBTYPE_PCM)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	//if (!SUCCEEDED(CheckInputType(&m_pInput->CurrentMediaType()))) 
	//{
	//	return VFW_E_TYPE_NOT_ACCEPTED;
	//}

	

    return S_OK;
}

//HRESULT CSoundTrans::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
//{}

HRESULT CSoundTrans::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    if(m_pInput->IsConnected() == FALSE)
		return E_UNEXPECTED;

    ASSERT(pAlloc);
    ASSERT(pProperties);

	HRESULT hr = NOERROR;

    pProperties->cBuffers = 4;
    pProperties->cbAlign = 1;

	
    IMemAllocator *pInAlloc = NULL;
    ALLOCATOR_PROPERTIES InProps;
    if(SUCCEEDED(hr = m_pInput->GetAllocator(&pInAlloc))
	&& SUCCEEDED(hr = pInAlloc->GetProperties(&InProps)))
    {
		//pProperties->cbBuffer = InProps.cbBuffer;
		pProperties->cbBuffer=48000*8*4/5;
		pInAlloc->Release();
    }
	else
	{
		return hr;
	}  
    ASSERT(pProperties->cbBuffer);

   // ALLOCATOR_PROPERTIES Actual;
    if(FAILED(hr = pAlloc->SetProperties(pProperties,&actual)))
		return hr;

    //ASSERT(Actual.cBuffers == 1);

    if(pProperties->cBuffers > actual.cBuffers
	|| pProperties->cbBuffer > actual.cbBuffer)
	{
		return E_FAIL;
    }
    return NOERROR;

}

CMediaType CSoundTrans::CreateMediaType(int sf, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
{
	CMediaType mt;

	mt.majortype = MEDIATYPE_Audio;
	mt.subtype = sf == 8 ? MEDIASUBTYPE_IEEE_FLOAT : MEDIASUBTYPE_PCM;
	mt.formattype = FORMAT_WaveFormatEx;

	WAVEFORMATEXTENSIBLE wfex;
	memset(&wfex, 0, sizeof(wfex));
	WAVEFORMATEX* wfe = &wfex.Format;
	wfe->wFormatTag = (WORD)mt.subtype.Data1;
	wfe->nChannels = nChannels;
	wfe->nSamplesPerSec = nSamplesPerSec;
	switch(sf)
	{
	default:
	case 1: wfe->wBitsPerSample = 16; break;
	}
	wfe->nBlockAlign = wfe->nChannels*wfe->wBitsPerSample/8;
	wfe->nAvgBytesPerSec = wfe->nSamplesPerSec*wfe->nBlockAlign;
	mt.SetSampleSize (wfe->wBitsPerSample*wfe->nChannels/8);

	// FIXME: 24/32 bit only seems to work with WAVE_FORMAT_EXTENSIBLE

	if(dwChannelMask)
	{
		wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wfex.Format.cbSize = sizeof(wfex) - sizeof(wfex.Format);
		wfex.dwChannelMask = dwChannelMask;
		wfex.Samples.wValidBitsPerSample = wfex.Format.wBitsPerSample;
		wfex.SubFormat = mt.subtype;
	}

	mt.SetFormat((BYTE*)&wfex, sizeof(wfex.Format) + wfex.Format.cbSize);

	return mt;
}

bool CSoundTrans::IsDiffrentMediatype(IMediaSample *pSample)
{
	HRESULT hr;
	//pSample->AddRef();
	
	AM_MEDIA_TYPE *mt ;
	hr = pSample->GetMediaType(&mt);
	if (hr != NOERROR)
	{ 
		return false;
	}


	DeleteMediaType(mt);
	//pSample->Release();

	return false;
}
HRESULT CSoundTrans::Receive(IMediaSample *pSample)
{
	HRESULT hr = E_FAIL;
	

	if(S_OK !=m_pInput->CheckStreaming())
		return S_FALSE;

	AM_SAMPLE2_PROPERTIES* const pProps = m_pInput->SampleProps();
	if(pProps->dwStreamId != AM_STREAM_MEDIA)
		return m_pOutput->Deliver(pSample);
	
	CMediaType mtIn = m_pInput->CurrentMediaType();
	long nInSize = pSample->GetSize();
	long nInLen = pSample->GetActualDataLength();
	

	m_SrcChannels = m_insf.nchannels;

	
	
	CComPtr<IMediaSample>pOut;
	if(FAILED(hr = m_pOutput->GetDeliveryBuffer(&pOut, NULL, NULL, 0)))
		return hr;

	long nOutSize = pSample->GetSize();
	long nOutLen = pSample->GetActualDataLength();

	AM_MEDIA_TYPE *infmt = NULL;
	hr = pSample->GetMediaType(&infmt);
	if(hr == S_OK)
	{
		m_bConvert = true;
		WAVEFORMATEX *pWfe = (WAVEFORMATEX *)infmt->pbFormat;
		m_SrcChannels = pWfe->nChannels;
		m_insf = *infmt;
		DeleteMediaType(infmt);	
	}

	hr = Transform(pSample,pOut);
	if (FAILED(hr))
	{
		return hr;
	}

	if(m_bMixer)
	{
		hr=S_OK;	
		if(nInLen == 0)
		{
			return S_OK;
		}
		size_t numsamples = nInLen / m_insf.blockAlign();
		
		//CMediaType outmt=m_Cursf.toCMediaType(true);
		//CMediaType inmt=m_insf.toCMediaType(true);

		m_Cursf = getOutsf();
		CMediaType mt=m_Cursf.toCMediaType(true);

		//m_pOutput->SetMediaType(&inmt);
		//pOut->SetMediaType(&inmt);
		//m_Cursf = m_insf;

		if (FAILED(hr=ReconnectOutput(numsamples,mt)))
			return hr;

		if ( hr==S_OK)
		{
			m_pOutput->SetMediaType(&mt);
			pOut->SetMediaType(&mt);
		}
		if(!Mix(pSample,pOut,numsamples,m_bReconnect))
		{
			return S_OK;
		}
	}
	return m_pOutput->Deliver(pOut);
}

HRESULT CSoundTrans::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
	BYTE* pDataIn;
    BYTE* pDataOut;
	HRESULT hr;
	//copy data
    if(FAILED(hr = pIn->GetPointer(&pDataIn)))return hr;
	if(FAILED(hr = pOut->GetPointer(&pDataOut)))return hr;
	long nlen = pIn->GetActualDataLength();
	long nlen1 = pOut->GetActualDataLength();


	WAVEFORMATEX *wfe = (WAVEFORMATEX *)m_pInput->CurrentMediaType().pbFormat;
	WAVEFORMATEX *wfeout = (WAVEFORMATEX *)m_pOutput->CurrentMediaType().pbFormat;
	int bps = wfe->wBitsPerSample>>3;											//每个帧有多少字节
	int inputsamplenums = pIn->GetActualDataLength() / (bps*wfe->nChannels);	//该IN-MEDIASAMPLE有多少个帧
 

	REFERENCE_TIME TimeStart, TimeEnd;
	hr = pIn->GetTime(&TimeStart, &TimeEnd);

	if(pIn->IsDiscontinuity() == S_OK)
	{
		m_fDiscontinuity = true;
		if(FAILED(hr)) { OutputDebugString(L"mpa: disc. w/o timestamp\n");return S_OK;} // lets wait then...
		m_rtStartProc = TimeStart;
	}

	if(hr == S_OK)
	if(SUCCEEDED(hr))
	{
		//end of  modify
		TimeStart += m_rtAudioTimeShift;
		TimeEnd   += m_rtAudioTimeShift;
		pOut->SetTime(&TimeStart, &TimeEnd);
		m_rtNextStart = TimeStart;
		m_rtNextStop  = TimeEnd;
	}
	else
	{
		pOut->SetTime(&m_rtNextStart, &m_rtNextStop);
		wchar_t str1[100];
		swprintf(str1, L"start: %lld,  stop:%lld\n",m_rtNextStart,m_rtNextStop);
		OutputDebugString(str1); 
	}


	REFERENCE_TIME rtDur = 10000000i64*inputsamplenums/wfe->nSamplesPerSec;
	m_rtNextStart += rtDur;
	m_rtNextStop  += rtDur;



	LONGLONG MediaStart, MediaEnd;
    if(S_OK == pIn->GetMediaTime(&MediaStart, &MediaEnd))
		pOut->SetMediaTime(&MediaStart, &MediaEnd);
	

	if(m_bmute)
		memset(pDataOut, m_wavformat.Format.wBitsPerSample == BYTESAMPLE8 ? 128:0, nlen);
	else
		TransData(pDataOut, pDataIn, nlen);
	//copy the media type
	AM_MEDIA_TYPE* pMediaType;
    pIn->GetMediaType(&pMediaType);
    pOut->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);
	//copy the data length
    pOut->SetActualDataLength(nlen);
	return S_OK;
}

HRESULT CSoundTrans::TransData(BYTE *pOutdata, BYTE *pIndata, long nInlen)
{
	CAutoLock lock(&m_Lock);
	BYTE *pStadata = pIndata;
	BYTE *pEnddata = pIndata + nInlen;
	SHORT *ssdata;
	float *ffdata;
	LONG  *lldata;
	int m = 0;
	int bytecounts = (m_wavformat.Format.wBitsPerSample>>3)*m_wavformat.Format.nChannels;//每个帧有多少位
	WORD wBitsPerSample = m_wavformat.Format.wBitsPerSample;
	
	if (m_wavformat.Format.nChannels == 1)
	{
		if((S_RIGHT == m_schannel)||(S_RTOALL == m_schannel))//缺省单声道为左边
		{	
			memset(pIndata,0,nInlen);
		}
	}
	else
	{
		if(S_LEFT == m_schannel)		//左声道
		{
			for (;pStadata < pEnddata; pStadata+=bytecounts)
			{
				switch(wBitsPerSample)
				{
				case BYTESAMPLE8:
					pStadata[1] = 0;
					break;
				case BYTESAMPLE16:
					ssdata=(SHORT*)pStadata;
					ssdata[1] = 0;
					break;
				case BYTESAMPLE24:
					pStadata[3] = pStadata[4] = pStadata[5] = 0;
					break;
				case BYTESAMPLE32:
					if(m_wavformat.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT || 
						m_wavformat.Format.wFormatTag==WAVE_FORMAT_EXTENSIBLE && m_wavformat.SubFormat == MEDIASUBTYPE_IEEE_FLOAT)
					{
						ffdata=(float*)pStadata;
						ffdata[1] = 0;
					}
					else
					{
						lldata=(LONG*)pStadata;
						lldata[1] = 0;
					}
					break;
				}
			}
			//CopyMemory(pOutdata, pIndata, nInlen);
		}
		else if(S_RIGHT == m_schannel)		//右声道
		{
			for (;pStadata < pEnddata; pStadata+=bytecounts)
			{
				switch(wBitsPerSample)
				{
				case BYTESAMPLE8:
					pStadata[0] = 0;
					break;
				case BYTESAMPLE16:
					ssdata=(SHORT*)pStadata;
					ssdata[0] = 0;
					break;
				case BYTESAMPLE24:
					pStadata[0] = pStadata[1] = pStadata[2] = 0;
					break;
				case BYTESAMPLE32:
					if(m_wavformat.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT || 
						m_wavformat.Format.wFormatTag==WAVE_FORMAT_EXTENSIBLE && m_wavformat.SubFormat == MEDIASUBTYPE_IEEE_FLOAT)
					{
						ffdata=(float*)pStadata;
						ffdata[0] = 0;
					}
					else
					{
						lldata=(LONG*)pStadata;
						lldata[0] = 0;
					}
					break;
				}
			}	
		}
		else if(S_LTOALL == m_schannel)		//左声道扩展到全部
		{
			for (;pStadata < pEnddata; pStadata+=bytecounts)
			{
				switch(wBitsPerSample)
				{
				case BYTESAMPLE8:
					pStadata[1] = pStadata[0];
					break;
				case BYTESAMPLE16:
					ssdata=(SHORT*)pStadata;
					ssdata[1] = ssdata[0];
					break;
				case BYTESAMPLE24:
					pStadata[3] = pStadata[0];
					pStadata[4] = pStadata[1];
					pStadata[5] = pStadata[2];
					break;
				case BYTESAMPLE32:
					if(m_wavformat.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT || 
						m_wavformat.Format.wFormatTag==WAVE_FORMAT_EXTENSIBLE && m_wavformat.SubFormat == MEDIASUBTYPE_IEEE_FLOAT)
					{
						ffdata=(float*)pStadata;
						ffdata[1] = ffdata[0];
					}
					else
					{
						lldata=(LONG*)pStadata;
						lldata[1] = lldata[0];
					}
					break;
				}
			}
		}
		else if(S_RTOALL == m_schannel)		//右声道扩展到全部
		{
			for (;pStadata < pEnddata; pStadata+=bytecounts)
			{
				switch(wBitsPerSample)
				{
				case BYTESAMPLE8:
					pStadata[0] = pStadata[1];
					break;
				case BYTESAMPLE16:
					ssdata=(SHORT*)pStadata;
					ssdata[0] = ssdata[1];
					break;
				case BYTESAMPLE24:
					pStadata[0] = pStadata[3];
					pStadata[1] = pStadata[4];
					pStadata[2] = pStadata[5];
					break;
				case BYTESAMPLE32:
					if(m_wavformat.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT || 
						m_wavformat.Format.wFormatTag==WAVE_FORMAT_EXTENSIBLE && m_wavformat.SubFormat == MEDIASUBTYPE_IEEE_FLOAT)
					{
						ffdata=(float*)pStadata;
						ffdata[0] = ffdata[1];
					}
					else
					{
						lldata=(LONG*)pStadata;
						lldata[0] = lldata[1];
					}
					break;
				}
			}
		}
		else if(S_MONO == m_schannel)		//混音
		{
			for (;pStadata < pEnddata; pStadata+=bytecounts)
			{
				switch(wBitsPerSample)
				{
				case BYTESAMPLE8:
					pStadata[0] = pStadata[1] = (BYTE)((pStadata[0]+pStadata[1])>>1);
					break;
				case BYTESAMPLE16:
					ssdata=(SHORT*)pStadata;
					ssdata[0] = ssdata[1] = (SHORT)((ssdata[0]+ssdata[1])>>1);
					break;
				case BYTESAMPLE24:
					m=(((((pStadata[2]+pStadata[5])<<8)+(pStadata[1]+pStadata[4]))<<8)+(pStadata[0]+pStadata[3]))>>1;
					pStadata[0]=pStadata[3]=(BYTE)m;
					pStadata[1]=pStadata[4]=(BYTE)(m>>8);
					pStadata[2]=pStadata[5]=(BYTE)(m>>16);
					break;
				case BYTESAMPLE32:
					if(m_wavformat.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT || 
						m_wavformat.Format.wFormatTag==WAVE_FORMAT_EXTENSIBLE && m_wavformat.SubFormat == MEDIASUBTYPE_IEEE_FLOAT)
					{
						ffdata=(float*)pStadata;
						ffdata[0] = ffdata[1] = (ffdata[0]+ffdata[1]/2);
					}
					else
					{
						lldata=(LONG*)pStadata;
						lldata[0] = lldata[1] = (LONG)((lldata[0]+(LONGLONG)lldata[1])>>1);
					}
					break;
				}
			}
		}
	}

	//增大音量
	if(m_laddsound > 10)
	{
		process(pIndata, nInlen);
	}
	if(m_bseq && m_eqfilter.ifeq())
	{
		int numsamples = nInlen/((m_wavformat.Format.wBitsPerSample>>3)*m_wavformat.Format.nChannels);
		if (m_wavformat.Format.wBitsPerSample == BYTESAMPLE24)
		{
			numsamples = nInlen;
		}
		m_eqfilter.process(wBitsPerSample, m_wavformat.Format.nChannels, pIndata, numsamples);
	}
	//write wav
	if(m_eAudioType > F_NO)
	{
		static LONGLONG flen = 0;
		BYTE *pwdate = pIndata;
		unsigned long nhavelen = nInlen;
		BOOL bWrite = FALSE;
		if((m_eAudioType == F_MP3 || m_eAudioType == F_VIDEOMP3) && m_pMp3Encoder)
		{
			if(m_pMp3Encoder->encodermp3(nInlen, pIndata, nhavelen, &pwdate))
			{
				if(m_eAudioType == F_MP3)
				{
					bWrite = TRUE;
				}
				//else if(g_IColortrans)
				//{
				//	int nInlen1 = nInlen > 0 ? nInlen : 1;
				//	g_IColortrans->put_audiodate((void *)pwdate, nhavelen, nInlen1);
				//	
				//	//g_IColortrans->put_audiodate((void *)pIndata, nInlen, nInlen1);
				//}
			}
		}
		else if(m_eAudioType == F_WAV)
		{
			bWrite = TRUE;
		}
		if(bWrite)
		{
			UINT nwlen = 0;
			while (nwlen < nhavelen)
			{
				int nwritelen = fwrite((void *)pwdate, 1, nhavelen, m_wavfile);
				//char str[100];
				//sprintf(str, "datalen:%d, writeLen:%d\n", nhavelen, nwritelen);
				//OutputDebugString(str);
				pwdate = pwdate + nwritelen;
				nhavelen -= nwlen;
				nwlen += nwritelen;
			}
			flen += nwlen;
			m_wavlen += nwlen;
			if(flen > WRITE_MAX)
			{
				flen = 0;
				fflush(m_wavfile);
			}
		}
	}
	CopyMemory(pOutdata, pIndata, nInlen);
	return S_OK;
}

void CSoundTrans::process(BYTE *pindata, long nlen)
{
	int numsamples = nlen/(m_wavformat.Format.wBitsPerSample>>3);
	if(numsamples > 0)
	{
		switch (m_wavformat.Format.wBitsPerSample)
		{
			case BYTESAMPLE8:
				process_v8(pindata, nlen);
				break;
			case BYTESAMPLE16:
				process_v16((short *)pindata, numsamples);
				break;
			case BYTESAMPLE24:
				process_v24(pindata,  nlen);
				break;
			case BYTESAMPLE32:
				process_v32((int *)pindata,  numsamples);	
				break;
		}
	}
}

void CSoundTrans::process_v8(BYTE *pindata, long nlen)
{
	double lf = (float)(m_laddsound/10);
	for(long i = 0; i < nlen; i++)
	{
		float n = pindata[i];
		n = n*lf;
		if(n < CHAR_MIN_NUM)
			pindata[i] = CHAR_MIN_NUM;
		if(n > CHAR_MAX_NUM)
			pindata[i] = CHAR_MAX_NUM;
	}
}

void CSoundTrans::process_v16(short *pindata, long nlen)
{
	//static unsigned long count = 0;
	//static unsigned long count2 = 0;
	double lf = (float)(m_laddsound/10);
	for(long i = 0; i < nlen; i++)
	{
		double n = pindata[i];
		n = n*lf;
		if(n < SHORT_MIN)
		{
			//count ++;
			pindata[i] = SHORT_MIN;
		}
		else if(n > SHORT_MAX)
		{
			//count ++;
			pindata[i] = SHORT_MAX;
		}
		else
			pindata[i] = short(n);
	}
}

void CSoundTrans::process_v24(BYTE *pindata,  long nlen)
{
	double lf = (float)(m_laddsound/10);
	for(long i = 0; i < nlen; i=i+3)
	{
		double n = ((int)pindata[i] << 16) + ((int)pindata[i+1]<<8) + ((int)pindata[i+2]);
		n = n*lf;
		if(n < INT24_MIN)
			n = INT24_MIN;
		if(n > INT24_MAX)
			n = INT24_MAX;
		BYTE *p = (BYTE *)&n;
		pindata[i] = *(p+1);
		pindata[i+1] = *(p+2);
		pindata[i+2] = *(p+3);
	}
}

void CSoundTrans::process_v32(int  *pindata,  long nlen)
{
	double lf = (float)(m_laddsound/10);
	for(long i = 0; i < nlen; i=i++)
	{
		double n = pindata[i];
		n = n*lf;
		if(n < INT32_MIN)
			pindata[i] = INT32_MIN;
		else if(n > INT32_MAX)
			pindata[i] = INT32_MAX;
		else
			pindata[i] = int(n);
	}
}

//
STDMETHODIMP CSoundTrans::put_channel(char cchannel)
{
	CAutoLock lock(&m_Lock);
	m_schannel = (SCHANNEL)cchannel;

	return S_OK;
}
STDMETHODIMP CSoundTrans::get_channel(char *cchannel)
{
	*cchannel = (char)m_schannel;
	return S_OK;
}
STDMETHODIMP CSoundTrans::put_mute(bool bmute)
{
	CAutoLock lock(&m_Lock);
	m_bmute = bmute;
	return S_OK;
}
STDMETHODIMP CSoundTrans::get_mute(bool *bmute)
{
	*bmute = m_bmute;
	return S_OK;
}
STDMETHODIMP CSoundTrans::put_soundshift(LONGLONG lshift)
{
	CAutoLock lock(&m_Lock);
	m_rtAudioTimeShift = (REFERENCE_TIME)lshift;
	return S_OK;
}
STDMETHODIMP CSoundTrans::get_soundshift(LONGLONG *lshift)
{
	CAutoLock lock(&m_Lock);
	*lshift = (LONGLONG)m_rtAudioTimeShift;
	return S_OK;
}

STDMETHODIMP CSoundTrans::put_addsound(LONG ladd)
{
	CAutoLock lock(&m_Lock);
	m_laddsound = ladd ;
	return S_OK;
}
STDMETHODIMP CSoundTrans::get_addsound(LONG *ladd)
{
	*ladd = m_laddsound;
	return S_OK;
}

STDMETHODIMP CSoundTrans::put_balance(LONG lbal)
{
	CAutoLock lock(&m_Lock);
	m_lbalance = lbal;
	IBasicAudio *iba = NULL;
	HRESULT hr = m_pGraph->QueryInterface(IID_IBasicAudio, (void **)&iba);
	if(SUCCEEDED(hr))
	{
		iba->put_Balance(m_lbalance*100);
		iba->Release();
	}
	iba = NULL;
	return S_OK;
}

STDMETHODIMP CSoundTrans::get_balance(LONG *lbal)
{
	*lbal = m_lbalance;
	return S_OK;
}

STDMETHODIMP CSoundTrans::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if(pPages->pElems == NULL)
    {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_SoundProperty;
    return NOERROR;
}

STDMETHODIMP CSoundTrans::put_eq(int *neq)
{
	CAutoLock lock(&m_Lock);

	if(m_bseq)
	{
		m_eqfilter.init(m_wavformat.Format.nSamplesPerSec, neq);
		return S_OK;
	}
	else
		return E_FAIL;
}

STDMETHODIMP CSoundTrans::get_eq(int *neq)
{
	m_eqfilter.geteq(neq);
	return S_OK;
}

STDMETHODIMP CSoundTrans::put_seq(bool bseq)
{
	CAutoLock lock(&m_Lock);
	
	m_bseq = bseq;
	return S_OK;
}
STDMETHODIMP CSoundTrans::get_seq(bool *bseq)
{
	*bseq = m_bseq;
	return S_OK;
}

bool CSoundTrans::CreateWavfile(char *psavepath)
{
	if(m_eAudioType <= F_NO)
		return false;
	if(m_eAudioType == F_WAV || m_eAudioType == F_MP3)
	{
		m_wavfile = fopen(psavepath, "wb");
		if(!m_wavfile)
		{
			//OutputDebugString("open file failed\n");
			return false;
		}
	}
	if(m_eAudioType == F_WAV)
	{
		fwrite(&m_wavriff, 1, sizeof(WAV_RIFF), m_wavfile);
		WAV_FMT wavfmt;
		memcpy(&wavfmt, &m_wavformat.Format, sizeof(WAV_FMT));
		WAVHEAD fmthead;
		memcpy(fmthead.szid, "fmt ", 4);
		fmthead.szsize = sizeof(WAV_FMT);
		fwrite(&fmthead,	1, sizeof(WAVHEAD), m_wavfile);
		fwrite(&wavfmt,		1, sizeof(WAV_FMT), m_wavfile);
		fwrite(&m_wavface,  1, sizeof(WAV_FAC), m_wavfile);
		memcpy(fmthead.szid, "data", 4);
		fwrite(&fmthead,    1, sizeof(WAVHEAD), m_wavfile);
		m_wavlen = sizeof(WAV_RIFF) + sizeof(WAVHEAD) + sizeof(WAV_FMT) + sizeof(WAV_FAC) + sizeof(WAVHEAD);
		fflush(m_wavfile);
	}	
	else if(m_eAudioType == F_MP3 || m_eAudioType == F_VIDEOMP3)
	{
		m_pMp3Encoder = new CMp3_Encoder();
		BOOL bSaveMp3 = m_eAudioType == F_MP3 ? TRUE : FALSE;
		if(!m_pMp3Encoder->init(m_wavformat.Format.nSamplesPerSec, bSaveMp3, psavepath))
		{
			delete m_pMp3Encoder;
			m_pMp3Encoder = NULL;
			return false;
		}
		//if(m_eAudioType == F_VIDEOMP3 && g_IColortrans)
		//{
		//	g_IColortrans->put_audiodate((void *)&m_wavformat.Format, sizeof(WAVEFORMATEX), 0);
		//}
	}
	return true;
}


void CSoundTrans::EnumFilters(void (*f)(IBaseFilter*,void*,void*), void *param, void *laram)
{
	IEnumFilters *pEnum;
	IBaseFilter *pFilter;
	if(m_pGraph && SUCCEEDED(m_pGraph->EnumFilters(&pEnum)))
	{
		while(pEnum->Next(1, &pFilter, 0) == S_OK)
		{
			f(pFilter,param, laram);
			pFilter->Release();
		}
		pEnum->Release();
	}
}

void CSoundTrans::GetAudioStreamCount(IBaseFilter *pFilter, void *count, void *nouse)
{
	IAMStreamSelect *pSS;
	DWORD *pcount = (DWORD *)count;
	DWORD n = 0 , temp = 0;

	if(SUCCEEDED(pFilter->QueryInterface(IID_IAMStreamSelect, (void **)&pSS)))
	{
		pSS->Count((DWORD*)&temp);
		if(n < temp)
		{
			DWORD nok = 0;
			for (DWORD i = 0; i < temp; i++)
			{
				AM_MEDIA_TYPE  *type;
				if((S_OK == pSS->Info(i,&type,0,0,0,0,0,0)) && (type->majortype ==MEDIATYPE_Audio))
				{
					nok++;		
				}
				DeleteMediaType(type);
			}
			if(n < nok)
			{
				n = nok;
			}
		}
		pSS->Release();
		*pcount = n;
	}
	
}

void CSoundTrans::SelectAudioStream(IBaseFilter *pFilter, void *done, void *ncur) 
{
	IAMStreamSelect *pSS;
	int i,n;
	int *paudioStream = (int *)ncur;
	int pcuraudio = 0;
	if(SUCCEEDED(pFilter->QueryInterface(IID_IAMStreamSelect, (void **)&pSS)))
	{
		pSS->Count((DWORD*)&n);
		for (i = 0; i < n; i++)
		{
			AM_MEDIA_TYPE *type;
			if(pSS->Info(i,&type,0,0,0,0,0,0)==S_OK)
			{
				if(type->majortype == MEDIATYPE_Audio)
				{
					if(pcuraudio == *paudioStream)//select this
					{
						pSS->Enable(i,AMSTREAMSELECTENABLE_ENABLE);
						*(bool*)done=true;
					}
					pcuraudio++;
				}
				DeleteMediaType(type);
			}
		}
		/*if(*paudioStream<n)
		{
			for(i=(*paudioStream-1); !*(bool*)done; i--)
			{
				if(i<0) i=n-1;
				if(i==(*paudioStream)) break;
				AM_MEDIA_TYPE *type;
				if(pSS->Info(i,&type,0,0,0,0,0,0)==S_OK)
				{
					if(type->majortype==MEDIATYPE_Audio)
					{
						pSS->Enable(i,AMSTREAMSELECTENABLE_ENABLE);
						(*paudioStream)=i;
						*(bool*)done=true;
					}
					DeleteMediaType(type);
				}
			}                          
		}*/
		pSS->Release();
	}
}


STDMETHODIMP CSoundTrans::put_astream(DWORD nastream)
{
	CAutoLock lock(&m_Lock);
	if(nastream > m_nastreamcount || nastream == m_nastreamcur)
		return S_FALSE;
	//
	bool done=false;
    EnumFilters(CSoundTrans::SelectAudioStream, &done, (void *)&nastream);
	m_nastreamcur = nastream;
	return S_OK;
}

STDMETHODIMP CSoundTrans::get_astream(DWORD *ncount, DWORD *ncur)
{
	if(m_nastreamcount == 0)
	{	//取音频流
		EnumFilters(CSoundTrans::GetAudioStreamCount, (void *)&m_nastreamcount, NULL);
	}
	*ncount = m_nastreamcount;
	*ncur   = m_nastreamcur;
	return S_OK;
}

STDMETHODIMP CSoundTrans::put_wav(LONG bsaveteyp, char *psavepath)
{
	CAutoLock lock(&m_Lock);
	//OutputDebugString("entry put_wav\n");

	if(psavepath != NULL)
	{
		strcpy(m_pwavpath, psavepath);
	}
	else
	{
		memset(m_pwavpath, 0, MAX_PATH);
	}
	SAVEAUDIOFILE eAudioType = (SAVEAUDIOFILE)bsaveteyp;
	if(eAudioType > F_NO)
	{
		if(m_eAudioType > F_NO)
			CloseWavfile();
		m_eAudioType = eAudioType;
		m_eAudioType = F_MP3;
		if(!CreateWavfile(m_pwavpath))
		{
			m_eAudioType = F_NO;
			return S_FALSE;
		}
	}
	else
	{
		CloseWavfile();
		m_eAudioType = F_NO;
	}
	return S_OK;
}
STDMETHODIMP CSoundTrans::get_wav(LONG *bsaveteyp, char *psavepath)
{
	CAutoLock lock(&m_Lock);
	*bsaveteyp = (long)m_eAudioType;
	if(psavepath != NULL && *m_pwavpath != 0)
		strcpy(psavepath, m_pwavpath);
	return S_OK;
}

STDMETHODIMP CSoundTrans::SetStereo()
{
	m_bmp3stero = true;
	m_DstChannels = 2;
	m_outConfigOption = 1;
	CMediaType inmt=m_insf.toCMediaType(true);
	WAVEFORMATEX *inwfe=(WAVEFORMATEX*)inmt.Format();
	inwfe->nSamplesPerSec = 44100;
	HRESULT hr = m_pOutput->SetMediaType(&inmt);
	m_bMixer = true;
	return hr ;

}

STDMETHODIMP CSoundTrans::PutChannelNum(float channel)
{
	CAutoLock lock(&m_Lock);
	if(channel <= 0 || channel >=10)
	{
		return S_FALSE;
	}
	
	if(channel == 1){
		m_DstChannels = 1;
		m_outConfigOption = 0;
	}
	else if(channel == 2){
		m_DstChannels = 2;
		m_outConfigOption = 1;
	}
	else if(channel == 3){
		m_DstChannels = 3; 
		m_outConfigOption = 2;
	}
	else if(channel > 2 && channel <= 2.1){
		m_DstChannels = 3;
		m_outConfigOption = 3;
	}
	else if(channel > 3 && channel <= 3.1){
		m_DstChannels = 4;
		m_outConfigOption = 4;
	}
	else if(channel ==4)
	{
		m_DstChannels = 4;
		m_outConfigOption = 5;
	}
	else if(channel > 4 && channel <= 4.1){
		m_DstChannels = 5;
		m_outConfigOption = 11;
	}
	else if(channel == 5){
		m_DstChannels = 5;
		m_outConfigOption = 6;
	}
	else if(channel >5 && channel <= 5.1){
		m_DstChannels = 6;
		m_outConfigOption = 13;
	}
	else if(channel == 6){
		m_DstChannels = 6;
		m_outConfigOption = 21;
	}
	else if(channel > 6 && channel <= 6.1 ){
		m_DstChannels = 7;
		m_outConfigOption = 22;
	}
	else if(channel == 7){
		m_DstChannels = 7;
		m_outConfigOption = 23;
	}
	else if(channel > 7 && channel <= 7.1){
		m_DstChannels = 8;
		m_outConfigOption = 24;
	}
	else
	{
		m_DstChannels = m_SrcChannels;
		m_outConfigOption = 16;
	}
	
	//if(m_DstChannels == m_SrcChannels)
	//{
	//	m_bMixer = false;
	//	return E_FAIL;
	//}
	//else
	{
		//m_Cursf = getOutsf();
		m_bMixer = true;
	}
	return S_OK;

}
STDMETHODIMP CSoundTrans::GetChannelNum(float* pchannel)
{
	CAutoLock lock(&m_Lock);
	if(pchannel == NULL)
	{
		return S_FALSE;
	}
	*pchannel = m_DstChannels;
	return S_OK;

}

HRESULT CSoundTrans::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	return CTransformFilter::NewSegment(tStart,tStop,dRate);
}

HRESULT CSoundTrans::StopStreaming()
{
	CloseWavfile();
	m_eAudioType = F_NO;
	return CTransformFilter::StopStreaming();
}

HRESULT CSoundTrans::StartStreaming()
{
	DWORD ncount = 0, ncur = 0;
	get_astream(&ncount, &ncur);
	return CTransformFilter::StartStreaming();
}

HRESULT CSoundTrans::init()
{
	//m_eqfilter.reset();
	m_nastreamcount = 0;
	m_nastreamcur = 0;
	return S_OK;
}

bool CSoundTrans::CloseWavfile()
{
	if(m_eAudioType <= F_NO)
		return TRUE;
	if(m_eAudioType == F_WAV)
	{
		if(m_wavlen > 0)
		{
			LONG len = m_wavlen - sizeof(WAVHEAD);
			WAVHEAD head;
			memcpy(head.szid, "RIFF", 4);
			head.szsize = len;
			fseek(m_wavfile, 0, SEEK_SET);
			fwrite(&head, 1, sizeof(head), m_wavfile);
			//写数据长度
			len = (sizeof(WAV_RIFF) + sizeof(WAVHEAD) + sizeof(WAV_FMT) + sizeof(WAV_FAC));
			fseek(m_wavfile, len, SEEK_SET);
			len = m_wavlen - len - sizeof(WAVHEAD);
			memcpy(head.szid, "data", 4);
			head.szsize = len;
			fwrite(&head, 1, sizeof(head), m_wavfile);
			m_wavlen = 0;
		}		
	}
	else if(m_eAudioType > F_WAV)
	{
		DWORD dwWrite = 0;
		if(m_eAudioType == F_MP3 && m_pMp3Encoder)
		{
			BYTE *pOutBuf = NULL;
			if(m_pMp3Encoder->deinitstream(&pOutBuf, &dwWrite))
			{
				fwrite(pOutBuf, 1, dwWrite, m_wavfile);
			}
		}
		if(m_pMp3Encoder)
		{
			delete m_pMp3Encoder;
			m_pMp3Encoder = NULL;
		}
	}
	if(m_wavfile)
		fclose(m_wavfile);
	m_wavfile = NULL;
	memset(m_pwavpath, 0, MAX_PATH);
	return true;
}