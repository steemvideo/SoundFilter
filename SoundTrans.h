// SoundTrans.h: interface for the CSoundTrans class.

#if !defined(_SOUNDTRANS_H___)
#define _SOUNDTRANS_H___

class CMp3_Encoder;

enum SCHANNEL
{
	S_MONO =0,			//混合音
	S_LEFT,				//左声道
	S_RIGHT,			//右声道
	S_DEFAULT,			//默认
	S_LTOALL,			//左声道扩展到全部
	S_RTOALL			//右声道扩展到全部
};

enum SAVEAUDIOFILE
{
	F_NO = 0,			//不保存
	F_WAV,				//WAV
	F_MP3,				//MP3
	F_VIDEOMP3			//VIDEOMP3			
};

struct WAVHEAD
{
	char  szid[4];
	DWORD szsize;
};
struct WAV_RIFF
{
	WAVHEAD head;
	char sz[4];
};
struct  WAV_FMT
{
	WORD	wFormatTag;
	WORD	wChannels;
	DWORD	dwSamplesPerSec;
	DWORD	dwAvgBytesPerSec;
	WORD	wBlockAlign;
	WORD	wBitsPerSample;
};
struct WAV_FAC
{
	WAVHEAD head;
	char szface[4];
};
struct WAV_DATA
{
	WAVHEAD head;
};
#define BYTESAMPLE8			8
#define BYTESAMPLE16		16
#define BYTESAMPLE24		24
#define BYTESAMPLE32		32
#define AF_NCH				8
#define	KM					10
#define	LOWDB				-1200
#define HIGHDB				1200

class CEqFilter
{
public:
	CEqFilter();
	~CEqFilter();
public:
	void process(int ntag, int nchannels, BYTE *samples, long numsamples);
	void init(double dhz, int *ndb);
	void reset();
	void geteq(int *ndb);
	bool ifeq();
private:
	//16
	float eq_16v(const float *g, short in, int ci);
	float limit_16v(float yt);
	void process_16v(short *samples, size_t numsamples, int nchannels);
	//24
	void process_24v(BYTE *samples, size_t numsamples, int nchannels);
	float eq_24v(const float *g, int in, int ci);
	float limit_24v(float yt);
	//32
	void process_32v(int *samples, size_t numsamples, int nchannels);
	float eq_32v(const float *g, int in, int ci);
	int limit_32v(float yt);


	static void bp2(float *a, float *b, double fc, double q);
private:
	float	wq[AF_NCH][KM][2];
	FLOAT	g[AF_NCH][KM];
	float	a[KM][2];
	float	b[KM][2];
	int		nf[10];
	int		neq[10];
	int		K;
};
#include "ISound.h"
#include <stdio.h>

#include "IMix.h"


class CSoundTrans : public CTransformFilter,
					public ISoundtrans,
					public ISpecifyPropertyPages					//属性页
{
public:
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    DECLARE_IUNKNOWN;
	bool IsDiffrentMediatype(IMediaSample *pSample);
	HRESULT Receive(IMediaSample *pSample);
    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut);
    HRESULT GetMediaType(int iPosition, CMediaType *mtOut);
	HRESULT CheckConnect(PIN_DIRECTION dir,IPin *pPin);
	//HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);
	TsampleFormat getOutsf(void);

    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);
	CMediaType CreateMediaType(int sf, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask);

    virtual HRESULT NewSegment(
                        REFERENCE_TIME tStart,
                        REFERENCE_TIME tStop,
                        double dRate);
	//HRESULT CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin); 

	virtual HRESULT StartStreaming();
	virtual HRESULT StopStreaming();
    // ISoundtrans
	STDMETHODIMP put_channel(char cchannel);
	STDMETHODIMP get_channel(char *cchannel);
	STDMETHODIMP put_mute(bool bmute);
	STDMETHODIMP get_mute(bool *bmute);
	STDMETHODIMP put_soundshift(LONGLONG lshift);
	STDMETHODIMP get_soundshift(LONGLONG *lshift);
	STDMETHODIMP put_addsound(LONG ladd);
	STDMETHODIMP get_addsound(LONG *ladd);
	STDMETHODIMP put_eq(int *neq);
	STDMETHODIMP get_eq(int *neq);
	STDMETHODIMP put_seq(bool bseq);
	STDMETHODIMP get_seq(bool *bseq);
	STDMETHODIMP put_balance(LONG lbal);
	STDMETHODIMP get_balance(LONG *lbal);
	STDMETHODIMP put_astream(DWORD nastream);
	STDMETHODIMP get_astream(DWORD *ncount, DWORD *ncur);
	STDMETHODIMP put_wav(LONG  bsavetype, char *psavepath);
	STDMETHODIMP get_wav(LONG *bsaveteyp, char *psavepath);
	STDMETHODIMP init();

	STDMETHODIMP PutChannelNum(float channel);
	STDMETHODIMP GetChannelNum(float* pchannel);
	STDMETHODIMP SetStereo();
    // ISpecifyPropertyPages method

    STDMETHODIMP GetPages(CAUUID *pPages);

	CBasePin *GetPin(int n);
	HRESULT QueryAcceptDownstream(const AM_MEDIA_TYPE* pmt);
	//HRESULT Reconnect(CMediaType &mtNew);
	 HRESULT ReconnectOutput(size_t numsamples,CMediaType& outmt);
	bool CanReconnect();
	void SetReconnect(){m_bReconnect = true;}
private:
	CSoundTrans(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
	virtual ~CSoundTrans();

	//copy data
	HRESULT TransData(BYTE *pOutdata, BYTE *pIndata, long nInlen);
	

	void	process(BYTE *pindata, long nlen);						//process addsound;
	void    process_v8(BYTE *pindata, long nlen);
	void    process_v16(short *pindata, long nlen);
	void	process_v24(BYTE *pindata,  long nlen);
	void	process_v32(int  *pindata,  long nlen);
	//filter
	void    EnumFilters(void (*f)(IBaseFilter*,void*,void*), void *param, void *laram);
static  void	GetAudioStreamCount(IBaseFilter *pFilter, void *count, void *nouse);
static	void	SelectAudioStream(IBaseFilter *pFilter, void *done, void *ncur); 

	//
	bool    CreateWavfile(char *psavepath);
	bool	CloseWavfile();
private:
	SCHANNEL				m_schannel;
	bool					m_bmute;				//mute
	WAVEFORMATEXTENSIBLE	m_wavformat;			//audio format
	void					*plugin;				//visualization plugin
	REFERENCE_TIME			m_endsampletime;		//end time of the last sample
	REFERENCE_TIME			m_rtAudioTimeShift;		//偏移
	REFERENCE_TIME			m_rtNextStart, m_rtNextStop;
	int						m_lastmediasamplesize;  //size of the last MediaSample
	LONG					m_laddsound;			//声音放大值
	CEqFilter				m_eqfilter;				//eqfilter
	LONG					m_lbalance;				//声音平衡
	bool					m_bseq;					//是否启用EQ
	DWORD					m_nastreamcount;		//音频流的个数
	DWORD					m_nastreamcur;			//当前的音频流
	//bool					m_bsavewav;				//是否保存WAV
	char					m_pwavpath[MAX_PATH];	//保存WAV的全路径
	FILE					*m_wavfile;				//WAVFILE
	LONG					m_wavlen;				//写的WAV的长度
	WAV_RIFF				m_wavriff;
	WAV_FAC					m_wavface;
	//	
	CMp3_Encoder			*m_pMp3Encoder;
	SAVEAUDIOFILE			m_eAudioType;
	CCritSec		m_Lock;

	bool					m_bReconnect;
	bool					m_bConvert;
	bool                  m_bmp3stero;

public:
	TsampleFormat m_insf,m_outsf,m_Cursf;
	ALLOCATOR_PROPERTIES actual;
	bool m_bMixer;
	float m_DstChannels;
	float m_SrcChannels;
	int m_inmask,m_outmask;
	int m_outConfigOption; 
	Mixer<int32_t,1<<16,4 > *mixerPCM16;
	const TmixerMatrix::mixer_matrix_t *matrixPtr;
	bool m_fDiscontinuity;
	REFERENCE_TIME m_rtStartProc; 

	bool Mix(IMediaSample *insample, IMediaSample *outsample,size_t nSamples,bool bCalMatirx);
	///////////////////////////
};

#endif // !_SOUNDTRANS_H___
