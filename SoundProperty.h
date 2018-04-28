// SoundProperty.h: 

#ifndef _SOUNDPROPERTY_H__
#define _SOUNDPROPERTY_H__
#define  EQ_COUNT	  20
typedef struct _x_sp
{
	int x;
	int y;
	int xw;
	int yh;
	DWORD v_h;
	int nmin;
	int nmax;
	int nstep;
	HWND hwnd;
}sp, *psp;
typedef struct _eqname
{
	WCHAR etext[100];
	_eqname()
	{
		memset(etext, 0, 200);
	}
}EQNAME, *PEQNAME;

typedef struct _defaulteq
{
	short eq[10];
}DEFAULTEQ,*PDEFAULTEQ;

class TeqSettings;
class CSoundProperty : public CBasePropertyPage  
{
public:
	static CUnknown *WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
    BOOL OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnApplyChanges();
	
    void SetDirty();
    CSoundProperty(LPUNKNOWN lpunk, HRESULT *phr);
private:
	HWND CreateSlider(HWND hwndParent, psp _psp, short value);
	HWND CreateEdit(HWND hwndParent, psp _psp, short value);
	void ChangerSound(HWND hdlg, WPARAM wParam, LPARAM lParam);
private:
	ISoundtrans *m_pSoundtrans;
	TeqSettings *peqsettings;
	EQNAME m_eqname[EQ_COUNT];
};

#endif