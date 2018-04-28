
#define SPEAKER_FRONT_LEFT              0x1
#define SPEAKER_FRONT_RIGHT             0x2
#define SPEAKER_FRONT_CENTER            0x4
#define SPEAKER_LOW_FREQUENCY           0x8
#define SPEAKER_BACK_LEFT               0x10
#define SPEAKER_BACK_RIGHT              0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER    0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER   0x80
#define SPEAKER_BACK_CENTER             0x100
#define SPEAKER_SIDE_LEFT               0x200
#define SPEAKER_SIDE_RIGHT              0x400
#define SPEAKER_TOP_CENTER              0x800
#define SPEAKER_TOP_FRONT_LEFT          0x1000
#define SPEAKER_TOP_FRONT_CENTER        0x2000
#define SPEAKER_TOP_FRONT_RIGHT         0x4000
#define SPEAKER_TOP_BACK_LEFT           0x8000
#define SPEAKER_TOP_BACK_CENTER         0x10000
#define SPEAKER_TOP_BACK_RIGHT          0x20000


typedef signed __int8     int8_t;
typedef signed __int16    int16_t;
typedef signed __int32    int32_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;

struct TalternateSampleFormat
{
	int alternateFormatId; // 0 : none, 1 : standard, 2 : xonar, 3 : azuentech
	GUID originalSubType; 
	GUID mediaSubtype;
	WORD wFormatTag;
	GUID wSubFormat;
	unsigned int nChannels;
	WORD wBitsPerSample;
	DWORD nSamplesPerSec;
	DWORD dwChannelMask;
	bool isExtensible;
};



struct TsampleFormat
{
protected:
	void init(const WAVEFORMATEX &wfex,bool wfextcheck,const GUID *subtype)
	{
		alternateSF=-1;; // Used for audio renderers that do not accept official bitstream media types
		freq=wfex.nSamplesPerSec;
		setChannels(wfex.nChannels,0);
		if (wfex.wFormatTag==WAVE_FORMAT_IEEE_FLOAT)
			sf=SF_FLOAT32;
		else if (wfex.wFormatTag==WAVE_FORMAT_DOLBY_AC3_SPDIF)
			sf=SF_AC3;
		else
			switch (wfex.wBitsPerSample)
		{
			case 8:sf=SF_PCM8;break;
			case 0:
			case 16:
			default:sf=SF_PCM16;break;
			case 20:sf=SF_LPCM20;break;
			case 24:sf=SF_PCM24;break;
			case 32:sf=SF_PCM32;break;
		}
		dolby=DOLBY_NO;
	}
public:
	TsampleFormat(void):
	  channelmask(0),
		  nchannels(0),
		  dolby(DOLBY_NO),
		  freq(0),
		  sf(SF_NULL),
		  pcm_be(false),
		  alternateSF(-1)
	  {
	  }

	  TsampleFormat(const AM_MEDIA_TYPE &mt):pcm_be(false)
	  {
		  alternateSF=-1;
		  if (mt.formattype==FORMAT_WaveFormatEx)
			  init(*(const WAVEFORMATEX*)mt.pbFormat,true,&mt.subtype);
		  else
		  {
			  nchannels=NULL;
			  sf=SF_NULL;
		  }
	  }

	  void reset(void)
	  {
		  channelmask=0;
		  nchannels=0;
		  freq=0;
		  sf=0;
		  dolby=DOLBY_NO;
		  alternateSF=-1;
	  }

	  void fillCommonWAVEFORMATEX(WAVEFORMATEX *pWfe, WAVEFORMATEXTENSIBLE *pWfex, bool alwayextensible) const
	  {
		  bool hdFormat = false;

		  pWfe->wFormatTag=(WORD)WAVE_FORMAT_PCM;

		  if (!hdFormat) // channels and nSamplesPerSec different for HD formats (set before)
		  {
			  pWfe->nChannels=WORD(nchannels);
			  pWfe->nSamplesPerSec=freq;
			  pWfe->wBitsPerSample=(WORD)bitsPerSample();
		  }

		  pWfe->nBlockAlign=WORD(pWfe->nChannels*pWfe->wBitsPerSample/8);
		  pWfe->nAvgBytesPerSec=pWfe->nSamplesPerSec*pWfe->nBlockAlign;

		  // FIXME: 24/32 bit only seems to work with WAVE_FORMAT_EXTENSIBLE
		  int dwChannelMask;
		  if (channelmask==0 && (sf==TsampleFormat::SF_PCM24 || sf==TsampleFormat::SF_PCM32 || nchannels>2))
			  dwChannelMask=makeChannelMask2();
		  else
			  dwChannelMask=channelmask;

		  if (!alwayextensible && dwChannelMask==standardChannelMasks[nchannels-1])
			  dwChannelMask=0;

		  if (!hdFormat)
		  {
			  if (dwChannelMask)
			  {
				  pWfe->wFormatTag=WAVE_FORMAT_EXTENSIBLE;
				  if (pWfex!=NULL)
				  {
					  pWfex->dwChannelMask=dwChannelMask;
					  pWfex->Samples.wValidBitsPerSample=pWfe->wBitsPerSample;
					  pWfex->SubFormat=sf==SF_FLOAT32?MEDIASUBTYPE_IEEE_FLOAT:MEDIASUBTYPE_PCM;
				  }
			  }
			  else
				  pWfe->cbSize=0;
		  }
		  else if (pWfex!=NULL && pWfex->dwChannelMask==0)
			  pWfex->dwChannelMask=dwChannelMask;
	  }

	  WAVEFORMATEXTENSIBLE toWAVEFORMATEXTENSIBLE(bool alwayextensible) const
	  {
		  WAVEFORMATEXTENSIBLE wfex;
		  memset(&wfex,0,sizeof(wfex));
		  wfex.Format.cbSize=sizeof(wfex)-sizeof(wfex.Format);
		  WAVEFORMATEX *pWfe=&wfex.Format;
		  WAVEFORMATEXTENSIBLE *pWfex=&wfex;

		  fillCommonWAVEFORMATEX(pWfe, pWfex, alwayextensible);
		  return wfex;
	  }

	  WAVEFORMATEX toWAVEFORMATEX() const
	  {
		  WAVEFORMATEX wfe;
		  memset(&wfe,0,sizeof(wfe));
		  wfe.cbSize=0;

		  fillCommonWAVEFORMATEX(&wfe, NULL, false);
		  return wfe;
	  }

	  static void updateAlternateMediaType(CMediaType &mt, int newSF)
	  {
		  if (newSF==-1 || mt.pbFormat==NULL) return;
	  }

	  CMediaType toCMediaType(bool alwaysextensible) const
	  {
		  CMediaType mt;
		  mt.majortype=MEDIATYPE_Audio;
		  mt.subtype=MEDIASUBTYPE_PCM;
		  mt.formattype=FORMAT_WaveFormatEx;
		  WAVEFORMATEXTENSIBLE wfex=toWAVEFORMATEXTENSIBLE(alwaysextensible);
		  mt.SetFormat((BYTE*)&wfex,sizeof(wfex.Format)+wfex.Format.cbSize);
		  updateAlternateMediaType(mt, alternateSF);
		  return mt;
	  }

	  enum
	  {
		  SF_NULL   =0,
		  SF_PCM16  =1,
		  SF_PCM24  =2,
		  SF_PCM32  =4,
		  SF_FLOAT32=8,

		  SF_ALL     =SF_PCM16|SF_PCM24|SF_PCM32|SF_FLOAT32,
		  SF_ALLINT  =SF_PCM16|SF_PCM24|SF_PCM32,
		  SF_ALL_24  =SF_PCM16|SF_PCM32|SF_FLOAT32,

		  SF_AC3     =16,
		  SF_PCM8    =32,
		  SF_LPCM16  =64,
		  SF_LPCM20  =128,
		  SF_FLOAT64 =65536,

		  SF_TRUEHD  = 256,
		  SF_DTSHD   = 512,
		  SF_EAC3    = 1024,

		  SF_ALLFLOAT=SF_FLOAT32|SF_FLOAT64,
	  };

	  enum
	  {
		  DOLBY_NO         =0,  // Stream is not Dolby-encoded
		  DOLBY_SURROUND   =1,  // Dolby Surround
		  DOLBY_PROLOGIC   =2,  // Dolby Pro Logic
		  DOLBY_PROLOGICII =3,  // Dolby Pro Logic II
		  DOLBY_LFE        =4   // LFE presence flag
	  };

	  int sf;
	  unsigned int freq;
	  unsigned int nchannels;
	  unsigned int channelmask;
	  int alternateSF;
	  int speakers[8];
	  int dolby;
	  bool pcm_be;


	  int countbits(uint32_t x)const
	  {
		  static const int numbits[256]=
		  {
			  0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
			  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
			  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
			  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
			  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			  3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
			  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			  3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
			  3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			  3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
			  4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
		  };
		  return numbits[((uint8_t*)&x)[0]]+
			  numbits[((uint8_t*)&x)[1]]+
			  numbits[((uint8_t*)&x)[2]]+
			  numbits[((uint8_t*)&x)[3]];
	  }

	  void setChannels(int Inchannels,int IchannelMask=0)
	  {
		  nchannels=min(8,Inchannels);
		  channelmask=IchannelMask;
		  if (channelmask==0)
		  {
			  static const int speakersPresets[8][8]=
			  {
				  SPEAKER_FRONT_CENTER,0,0,0,0,0,0,0,
				  SPEAKER_FRONT_LEFT,SPEAKER_FRONT_RIGHT,0,0,0,0,0,0,
				  SPEAKER_FRONT_LEFT,SPEAKER_FRONT_RIGHT,SPEAKER_BACK_CENTER,0,0,0,0,0,
				  SPEAKER_FRONT_LEFT,SPEAKER_FRONT_RIGHT,SPEAKER_BACK_LEFT,SPEAKER_BACK_RIGHT,0,0,0,0,
				  SPEAKER_FRONT_LEFT,SPEAKER_FRONT_RIGHT,SPEAKER_FRONT_CENTER,SPEAKER_BACK_LEFT,SPEAKER_BACK_RIGHT,0,0,0,
				  SPEAKER_FRONT_LEFT,SPEAKER_FRONT_RIGHT,SPEAKER_FRONT_CENTER,SPEAKER_LOW_FREQUENCY,SPEAKER_BACK_LEFT,SPEAKER_BACK_RIGHT,0,0,
				  SPEAKER_FRONT_LEFT,SPEAKER_FRONT_RIGHT,SPEAKER_FRONT_CENTER,SPEAKER_BACK_LEFT,SPEAKER_BACK_RIGHT,SPEAKER_SIDE_LEFT,SPEAKER_SIDE_RIGHT,0,
				  SPEAKER_FRONT_LEFT,SPEAKER_FRONT_RIGHT,SPEAKER_FRONT_CENTER,SPEAKER_LOW_FREQUENCY,SPEAKER_BACK_LEFT,SPEAKER_BACK_RIGHT,SPEAKER_SIDE_LEFT,SPEAKER_SIDE_RIGHT,
			  };
			  memcpy(speakers,speakersPresets[nchannels-1],sizeof(int)*8);
		  }
		  else
		  {
			  static const int speakersOrder[]={SPEAKER_FRONT_LEFT,SPEAKER_FRONT_RIGHT,SPEAKER_FRONT_CENTER,SPEAKER_LOW_FREQUENCY,SPEAKER_BACK_LEFT,SPEAKER_BACK_RIGHT,SPEAKER_FRONT_LEFT_OF_CENTER,SPEAKER_FRONT_RIGHT_OF_CENTER,SPEAKER_BACK_CENTER,SPEAKER_SIDE_LEFT,SPEAKER_SIDE_RIGHT,SPEAKER_TOP_CENTER,SPEAKER_TOP_FRONT_LEFT,SPEAKER_TOP_FRONT_CENTER,SPEAKER_TOP_FRONT_RIGHT,SPEAKER_TOP_BACK_LEFT,SPEAKER_TOP_BACK_CENTER,SPEAKER_TOP_BACK_RIGHT};
			  for (unsigned int i=0,c=0;i<sizeof(speakersOrder) && c<nchannels;i++)
				  if (channelmask&speakersOrder[i])
					  speakers[c++]=speakersOrder[i];
		  }
	  }
	  int findSpeaker(int spk) const
	  {
		  for (unsigned int i=0;i<nchannels;i++)
			  if (speakers[i]==spk)
				  return i;
		  return -1;
	  }
	  int makeChannelMask(void) const
	  {
		  int mask=0;
		  for (unsigned int i=0;i<nchannels;i++)
			  mask|=speakers[i];
		  return mask;
	  }
	  static const int standardChannelMasks[];
	  int makeChannelMask2(void) const
	  {
		  if (channelmask)
			  return channelmask;
		  else
			  return standardChannelMasks[nchannels-1];
	  }
	  int nfront(void) const
	  {
		  return countbits(makeChannelMask2()&(SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_FRONT_LEFT_OF_CENTER|SPEAKER_FRONT_RIGHT_OF_CENTER));
	  }
	  int nrear(void) const
	  {
		  return countbits(makeChannelMask2()&(SPEAKER_BACK_LEFT|SPEAKER_BACK_CENTER|SPEAKER_BACK_RIGHT));
	  }
	  int nside(void) const
	  {
		  return countbits(makeChannelMask2()&(SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT));
	  }

	  unsigned int bitsPerSample(void) const
	  {
		  switch (sf)
		  {
		  case SF_PCM8:return 8;
		  default:
		  case SF_LPCM16:
		  case SF_PCM16:return 16;
		  case SF_PCM24:return 24;
		  case SF_PCM32:
		  case SF_FLOAT32:return 32;
		  case SF_FLOAT64:return 64;
		  }
	  }
	  unsigned int avgBytesPerSec(void) const
	  {
		  return freq*blockAlign();
	  }
	  unsigned int blockAlign(void) const
	  {
		  return nchannels*bitsPerSample()/8;
	  }

	  bool operator !(void) const
	  {
		  return !nchannels;
	  }

	  bool operator !=(const TsampleFormat &sf2) const
	  {
		  return memcmp(this,&sf2,sizeof(*this)+sizeof(bool)-sizeof(int))!=0;
	  }
};

struct TchConfig
{
	int id;
	const wchar_t *name;int nameIndex;
	unsigned int nchannels,channelmask;
	int dolby;
};

static const TchConfig chConfigs[] = 
{
	0,L"1/0/0 - mono",            0, 1, 0                                                                                                , 0,
	1,L"2/0/0 - stereo",          1, 2, 0                                                                                                , 0,
	2,L"3/0/0 - 3 front",         2, 3, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER                                      , 0,
	3,L"2/0/1 - surround",        3, 3, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER                                       , 0,
	4,L"3/0/1 - surround",        4, 4, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER                  , 0,
	5,L"2/0/2 - quadro",          5, 4, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT                      , 0,
	6,L"3/0/2 - 5 channels",      6, 5, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT , 0,

	7,L"1/0/0+LFE 1.1 mono",      0, 2, SPEAKER_FRONT_CENTER                                                                            |SPEAKER_LOW_FREQUENCY , 0,
	8,L"2/0/0+LFE 2.1 stereo",    1, 3, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT                                                          |SPEAKER_LOW_FREQUENCY , 0,
	9,L"3/0/0+LFE 3.1 front",     2, 4, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER                                     |SPEAKER_LOW_FREQUENCY , 0,
	10,L"2/0/1+LFE 3.1 surround",  3, 4, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER                                      |SPEAKER_LOW_FREQUENCY , 0,
	11,L"3/0/1+LFE 4.1 surround",  4, 5, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER                 |SPEAKER_LOW_FREQUENCY , 0,
	12,L"2/0/2+LFE 4.1 quadro",    5, 5, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT                     |SPEAKER_LOW_FREQUENCY , 0,
	13,L"3/0/2+LFE 5.1 channels",  6, 6, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_LOW_FREQUENCY , 0,

	14,L"Dolby Surround/ProLogic", 14, 2, 0,1  ,
	15,L"Dolby ProLogic II",       15, 2, 0,3,

	16,L"same as input",           -1, 0, 0, 0,

	17,L"headphone virtual spatialization",     -1, 2, 0, 0,
	18,L"Head-related transfer function (HRTF)",-1, 2, 0, 0,

	19,L"Dolby Surround/ProLogic+LFE", 14, 3, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY,0  ,
	20,L"Dolby ProLogic II+LFE",       15, 3, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY,0,

	21,L"3/2/1 - 6 channels",      21, 6, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_BACK_CENTER , 0,
	22,L"3/2/1+LFE 6.1 channels",  21, 7, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_BACK_CENTER|SPEAKER_LOW_FREQUENCY , 0,
	23, L"3/2/2 - 7 channels",      23, 7, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT , 0,
	24, L"3/2/2+LFE 7.1 channels",  23, 8, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_LOW_FREQUENCY , 0,

	NULL
};

template<class T> inline const T& limit(const T& val,const T& min,const T& max)
{
	if (val<min) return min;
	else if (val>max) return max;
	else return val;
}

struct TsampleFormatInfo
{
	static int16_t limit(int32_t v)
	{
		v+=0x8000;
		if ((uint32_t)v>=0x10000)
			v=~v>>31;
		return int16_t(v-0x8000);
	}
};

class TmixerMatrix
{
private:
	static const double LEVEL_PLUS6DB,LEVEL_PLUS3DB,LEVEL_3DB,LEVEL_45DB,LEVEL_6DB;
	enum
	{
		CH_L        =0, // Left channel
		CH_R        =1, // Right channel
		CH_C        =2, // Center channel
		CH_LFE      =3, // LFE channel
		CH_BL       =4, // Back left channel
		CH_BR       =5, // Back right channel
		CH_BC       =6, // Back center channel
		CH_AL       =7, // Side left channel
		CH_AR       =8, // Side right channel
		CH_NONE     =9, // indicates that channel is not used in channel order

		CH_M        =2  // Mono channel = center channel
	};
protected:
	static const int NCHANNELS=9;
	TsampleFormat calc_matrix(TsampleFormat &infmt,TsampleFormat &outfmt,int pos);
public:
	typedef double mixer_matrix_t[NCHANNELS][NCHANNELS];
	mixer_matrix_t matrix;
	TmixerMatrix(void);
};

template<class div_t,div_t div,div_t div2>class Mixer :public TmixerMatrix
{
private:
	typedef typename int32_t helper_t;
	typedef helper_t mixer_matrix_t[NCHANNELS][NCHANNELS];
	mixer_matrix_t matrix;
	unsigned int rows,cols;

	template<unsigned int colsT,unsigned int rowsT> static void mix(size_t nsamples,const  int16_t *insamples, int16_t *outsamples,const mixer_matrix_t &matrix)
	{
		for (unsigned int nsample=0;nsample<nsamples;insamples+=rowsT,outsamples+=colsT,nsample++)
			for (unsigned int i=0;i<colsT;i++)
			{
				helper_t sum=0;
				for (unsigned int j=0;j<rowsT;j++)
					sum+=(insamples[j]*matrix[j][i])/helper_t(div/div2);
				outsamples[i]=TsampleFormatInfo::limit(sum);
			}
	}
	typedef void (*TmixFn)(size_t nsamples,const  int16_t *insamples, int16_t *outsamples,const mixer_matrix_t &matrix);
	TmixFn mixFn;

	bool calc_matrix(TsampleFormat &insf,TsampleFormat &outsf ,int pos,const TmixerMatrix::mixer_matrix_t* *matrixPtr,int *inmaskPtr,int *outmaskPtr)
	{
		TmixerMatrix::calc_matrix(insf,outsf,pos);
		*matrixPtr=&this->TmixerMatrix::matrix;
		static const int mspeakers[]={SPEAKER_FRONT_LEFT,SPEAKER_FRONT_RIGHT,SPEAKER_FRONT_CENTER,SPEAKER_LOW_FREQUENCY,SPEAKER_BACK_LEFT,SPEAKER_BACK_RIGHT,SPEAKER_BACK_CENTER,SPEAKER_SIDE_LEFT,SPEAKER_SIDE_RIGHT};
		memset(&matrix, 0, sizeof(mixer_matrix_t));
		int inmask  = insf.makeChannelMask();
		int outmask = outsf.makeChannelMask();
		*inmaskPtr  = inmask;
		*outmaskPtr = outmask;

		rows=0;
		for (int i=0;i<NCHANNELS;i++)
			if (inmask&mspeakers[i])
			{
				cols=0;
				for (int j=0;j<NCHANNELS;j++)
					if (outmask&mspeakers[j])
					{
						matrix[rows][cols]=helper_t(TmixerMatrix::matrix[i][j]*div/div2);
						cols++;
					}
					rows++;
			}

			static const TmixFn mixFns[9][9]=
			{
				mix<1,1>,mix<1,2>,mix<1,3>,mix<1,4>,mix<1,5>,mix<1,6>,mix<1,7>,mix<1,8>,mix<1,9>,
				mix<2,1>,mix<2,2>,mix<2,3>,mix<2,4>,mix<2,5>,mix<2,6>,mix<2,7>,mix<2,8>,mix<2,9>,
				mix<3,1>,mix<3,2>,mix<3,3>,mix<3,4>,mix<3,5>,mix<3,6>,mix<3,7>,mix<3,8>,mix<3,9>,
				mix<4,1>,mix<4,2>,mix<4,3>,mix<4,4>,mix<4,5>,mix<4,6>,mix<4,7>,mix<4,8>,mix<4,9>,
				mix<5,1>,mix<5,2>,mix<5,3>,mix<5,4>,mix<5,5>,mix<5,6>,mix<5,7>,mix<5,8>,mix<5,9>,
				mix<6,1>,mix<6,2>,mix<6,3>,mix<6,4>,mix<6,5>,mix<6,6>,mix<6,7>,mix<6,8>,mix<6,9>,
				mix<7,1>,mix<7,2>,mix<7,3>,mix<7,4>,mix<7,5>,mix<7,6>,mix<7,7>,mix<7,8>,mix<7,9>,
				mix<8,1>,mix<8,2>,mix<8,3>,mix<8,4>,mix<8,5>,mix<8,6>,mix<8,7>,mix<8,8>,mix<8,9>,
				mix<9,1>,mix<9,2>,mix<9,3>,mix<9,4>,mix<9,5>,mix<9,6>,mix<9,7>,mix<9,8>,mix<9,9>
			};
			mixFn=mixFns[cols-1][insf.nchannels-1];	
			return true;
	}
public:
	Mixer(void)
	{
	}
	void process(TsampleFormat &fmt,TsampleFormat &outfmt, int16_t* &samples1, int16_t* &samples2,size_t &numsamples,int pos,const TmixerMatrix::mixer_matrix_t* *matrixPtr,int *inmask,int *outmask,bool wasChanged)
	{
		if (wasChanged){
			calc_matrix(fmt,outfmt,pos,matrixPtr,inmask,outmask);
		}		
		TsampleFormat fmt2=fmt;
		fmt2.setChannels(outfmt.nchannels,outfmt.channelmask);
		mixFn(numsamples,samples1,samples2,matrix);
	}
};



