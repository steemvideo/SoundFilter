// ColorProperty.cpp: 
#include "stdafx.h"
#include <streams.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <stdio.h>
#include <Windows.h>
#include "resource.h"
//#include "alldefine.h"
#include <CommDlg.h>
#include "ISound.h"
#include "IMix.h"
#include "SoundProperty.h"


#pragma warning( disable : 4996 )

//延迟
#define	 DELAYMAX     20000			//定义最大周期数
#define	 DELAYMIN     -20000		//定义最小周期数
#define  DELXYMAX	 2000			//最大的X/Y值
#define  CHAR_100    100


RECT soundslitrect[] = 
{
	{285, 100, 135, 20},
};
sp X_SP[]=
{
	{80,   90, 300, 20, TBS_HORZ,  10,   100, 1},  //声音放大
	{190,   298, 110, 20, TBS_HORZ,  -100,   100, 1},//lbalance
	{18,    150, 20,  110, TBS_VERT, 0,  200, 1},//以下为均衡器
	{60,    150, 20,  110, TBS_VERT, 0,  200, 1},
	{102,  150, 20,  110, TBS_VERT, 0,  200, 1},
	{144,  150, 20,  110, TBS_VERT, 0,  200, 1},
	{186,  150, 20,  110, TBS_VERT, 0,  200, 1},
	{228,  150, 20,  110, TBS_VERT, 0,  200, 1},
	{270,  150, 20,  110, TBS_VERT, 0,  200, 1},
	{312,  150, 20,  110, TBS_VERT, 0,  200, 1},
	{354,  150, 20,  110, TBS_VERT, 0,  200, 1},
	{396,  150, 20,  110, TBS_VERT, 0,  200, 1},
};

sp X_EDIT[]=
{
	//1
	{20,   405, 38, 20, 0,  10,   38, 1},  
	{60,   405, 38, 20, 0,  10,   38, 1},
	{100,   405, 38,  20, 0, 10,  38, 1},
	{140,   405, 38,  20, 0, 10,  38, 1},
	{180,  405, 38,  20, 0, 10,  38, 1},
	{220,  405, 38,  20, 0, 10,  38, 1},
	{260, 405, 38,  20, 0, 10,  38, 1},
	{300, 405, 38,  20, 0, 10,  38, 1},
	{340,  405, 38,  20, 0, 10,  38, 1},
//2
	{20,   428, 38, 20, 0,  10,   38, 1},  
	{60,   428, 38, 20, 0,  10,   38, 1},
	{100,   428, 38,  20, 0, 10,  38, 1},
	{140,   428, 38,  20, 0, 10,  38, 1},
	{180,  428, 38,  20, 0, 10,  38, 1},
	{220,  428, 38,  20, 0, 10,  38, 1},
	{260, 428, 38,  20, 0, 10,  38, 1},
	{300, 428, 38,  20, 0, 10,  38, 1},
	{340,  428, 38,  20, 0, 10,  38, 1},
//3
	{20,   451, 38, 20, 0,  10,   38, 1},  
	{60,   451, 38, 20, 0,  10,   38, 1},
	{100,   451, 38,  20, 0, 10,  38, 1},
	{140,   451, 38,  20, 0, 10,  38, 1},
	{180,  451, 38,  20, 0, 10,  38, 1},
	{220,  451, 38,  20, 0, 10,  38, 1},
	{260, 451, 38,  20, 0, 10,  38, 1},
	{300, 451, 38,  20, 0, 10,  38, 1},
	{340,  451, 38,  20, 0, 10,  38, 1},
//4
	{20,   474, 38, 20, 0,  10,   38, 1},  
	{60,   474, 38, 20, 0,  10,   38, 1},
	{100,   474, 38,  20, 0, 10,  38, 1},
	{140,   474, 38,  20, 0, 10,  38, 1},
	{180,  474, 38,  20, 0, 10,  38, 1},
	{220,  474, 38,  20, 0, 10,  38, 1},
	{260, 474, 38,  20, 0, 10,  38, 1},
	{300, 474, 38,  20, 0, 10,  38, 1},
	{340,  474, 38,  20, 0, 10,  38, 1},
//5
	{20,   497, 38, 20, 0,  10,   38, 1},  
	{60,   497, 38, 20, 0,  10,   38, 1},
	{100,   497, 38,  20, 0, 10,  38, 1},
	{140,   497, 38,  20, 0, 10,  38, 1},
	{180,  497, 38,  20, 0, 10,  38, 1},
	{220,  497, 38,  20, 0, 10,  38, 1},
	{260, 497, 38,  20, 0, 10,  38, 1},
	{300, 497, 38,  20, 0, 10,  38, 1},
	{340,  497, 38,  20, 0, 10,  38, 1},
//6
	{20,   520, 38, 20, 0,  10,   38, 1},  
	{60,   520, 38, 20, 0,  10,   38, 1},
	{100,   520, 38,  20, 0, 10,  38, 1},
	{140,   520, 38,  20, 0, 10,  38, 1},
	{180,  520, 38,  20, 0, 10,  38, 1},
	{220,  520, 38,  20, 0, 10,  38, 1},
	{260, 520, 38,  20, 0, 10,  38, 1},
	{300, 520, 38,  20, 0, 10,  38, 1},
	{340,  520, 38,  20, 0, 10,  38, 1},	
//7
	{20,   543, 38, 20, 0,  10,   38, 1},  
	{60,   543, 38, 20, 0,  10,   38, 1},
	{100,   543, 38,  20, 0, 10,  38, 1},
	{140,   543, 38,  20, 0, 10,  38, 1},
	{180,  543, 38,  20, 0, 10,  38, 1},
	{220,  543, 38,  20, 0, 10,  38, 1},
	{260, 543, 38,  20, 0, 10,  38, 1},
	{300, 543, 38,  20, 0, 10,  38, 1},
	{340,  543, 38,  20, 0, 10,  38, 1},
//8
	{20,   566, 38, 20, 0,  10,   38, 1},  
	{60,   566, 38, 20, 0,  10,   38, 1},
	{100,   566, 38,  20, 0, 10,  38, 1},
	{140,   566, 38,  20, 0, 10,  38, 1},
	{180,  566, 38,  20, 0, 10,  38, 1},
	{220,  566, 38,  20, 0, 10,  38, 1},
	{260, 566, 38,  20, 0, 10,  38, 1},
	{300, 566, 38,  20, 0, 10,  38, 1},
	{340,  566, 38,  20, 0, 10,  38, 1},
//9
	{20,   589, 38, 20, 0,  10,   38, 1},  
	{60,   589, 38, 20, 0,  10,   38, 1},
	{100,   589, 38,  20, 0, 10,  38, 1},
	{140,   589, 38,  20, 0, 10,  38, 1},
	{180,  589, 38,  20, 0, 10,  38, 1},
	{220,  589, 38,  20, 0, 10,  38, 1},
	{260, 589, 38,  20, 0, 10,  38, 1},
	{300, 589, 38,  20, 0, 10,  38, 1},
	{340,  589, 38,  20, 0, 10,  38, 1},
};

DEFAULTEQ _X_DEFAULT[] =
{
	{107,	93,		86,		79,		107,	107,	100,	100,	71,		71},
	{107,	100,	86,		86,		107,	107,	100,	100,	71,		57},
	{129,	100,	100,	114,	100,	100,	86,		79,		100,	121},
	{129,	100,	100,	100,	100,	100,	86,		100,	86,		100},
	{142,	93,		71,		107,	114,	114,	100,	100,	136,	136},
	{107,	100,	100,	86,		86,		100,	100,	100,	79,		79},
	{100,	100,	100,	79,		79,		79,		100,	86,		129,	129},
	{100,	93,		86,		100,	100,	100,	100,	100,	86,		86},
	{107,	100,	93,		93,		86,		100,	100,	100,	86,		100},
	{100,	79,		79,		100,	100,	100,	100,	100,	93,		93},
	{100,	57,		57,		79,		100,	100,	100,	100,	86,		86},
	{107,	100,	86,		93,		100,	100,	100,	100,	107,	121},
	{114,	100,	86,		93,		100,	100,	100,	100,	114,	136},
	{107,	100,	100,	121,	100,	79,		71,		100,	79,		71},
	{100,	100,	100,	79,		71,		86,		64,		86,		100,	100},
	{107,	100,	100,	100,	79,		79,		100,	86,		71,		71},
	{114,	100,	86,		93,		100,	100,	100,	100,	114,	136},
	{100,	100,	100,	100,	100,	100,	100,	100,	114,	100},
	{100,	100,	100,	100,	100,	100,	100,	114,	121,	100},
	{100,	100,	100,	100,	100,	100,	100,	100,	100,	100},
};
const WCHAR *ceqname[]=
{
	{L"摇滚"},
	{L"打击乐"},
	{L"垃圾乐"},
	{L"金属乐"},
	{L"Techno 电子乐"},
	{L"乡村音乐"},
	{L"爵士乐"},
	{L"非电声音乐"},
	{L"民乐"},
	{L"新纪元音乐"},
	{L"古典乐"},
	{L"布鲁斯"},
	{L"怀旧音乐"},
	{L"瑞格乐"},
	{L"歌剧"},
	{L"摇摆乐"},
	{L"语音"},
	{L"音乐56K"},
	{L"音乐28K"},
	{L"默认值"}
};
CUnknown * WINAPI CSoundProperty::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);

    CUnknown *punk = new CSoundProperty(lpunk, phr);

    if(punk == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }
    return punk;
}

CSoundProperty::CSoundProperty(LPUNKNOWN pUnk, HRESULT *phr) :
    CBasePropertyPage(NAME("SoundTrans Property Page"), pUnk, IDD_SOUNDPROP, IDS_SOUNDTITLE),
	m_pSoundtrans(NULL)
{
    InitCommonControls();
} 

void CSoundProperty::SetDirty()
{
    m_bDirty = TRUE;
    if(m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }
}

BOOL CSoundProperty::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
			if(m_pSoundtrans)
			{
				char cchannel = 0;
				m_pSoundtrans->get_channel(&cchannel);
				UINT ncheck = IDC_RADIO1 + cchannel;
				CheckRadioButton(hwnd, IDC_RADIO1, IDC_RADIO6, ncheck);
				LONGLONG lshittime = 0;
				m_pSoundtrans->get_soundshift(&lshittime);
				lshittime = lshittime/10000i64;
				HWND hedit      = ::GetDlgItem(hwnd, IDC_EDIT2);
				HWND hupdown = CreateUpDownControl(WS_CHILD|WS_BORDER|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_NOTHOUSANDS,
					90,5,20,15, hwnd, IDC_SPIN1, g_hInst, hedit, DELAYMAX, DELAYMIN, (int)lshittime);//建立第一个旋转按钮
				UDACCEL uda;
				uda.nInc = 10;
				uda.nSec = 1;
				::SendMessage(hupdown, UDM_SETACCEL, 1, (LPARAM)&uda);
				//创建音量放大控制
				RECT *prect = soundslitrect;
				LONG laddsound = 0;
				m_pSoundtrans->get_addsound(&laddsound);
				psp _psp = X_SP;
				CreateSlider(hwnd, _psp, (short)laddsound);
				char lptext[MAX_PATH];
				sprintf(lptext, "%0.1fx", (float)laddsound/10);
				SetDlgItemTextA(hwnd, IDC_STATIC6, lptext);
				_psp++;
				LONG lbalance = 0; 
				m_pSoundtrans->get_balance(&lbalance);
				CreateSlider(hwnd, _psp, (short)lbalance);
				
				bool bseq = false;
				m_pSoundtrans->get_seq(&bseq);
				if(bseq)
				{
					HWND hWnd = ::GetDlgItem(hwnd, IDC_CHECK2);
					::SendMessage( hWnd, (UINT)BM_SETCHECK, BST_CHECKED, 0 );
				}
				int neq[10];
				if(bseq)
					m_pSoundtrans->get_eq(neq);
				else
				{
					for (int ii = 0; ii < 10; ii++)
					{
						neq[ii] = 100;
					}
				}
				for (int i = 0; i < 10 ; i++)
				{
					_psp++;
					CreateSlider(hwnd, _psp, (short)(200-neq[i]));
				}
				psp pEdit = X_EDIT;
				for(int i = 0;i<81;i++)
				{
					CreateEdit(hwnd, pEdit, (short)lbalance);
					pEdit++;
				}
				//加载配置输出声道
				HWND hCombox = ::GetDlgItem(hwnd, IDC_CONFIGURATION);
				for (int i = 0; i < 12; i++)
				{
					//wcscpy_s(m_eqname[i].etext,100, ceqname[i]);
					SendMessage(hCombox, CB_ADDSTRING, 0,	(LPARAM) (LPCSTR) chConfigs[i].name); 
				}
				SendMessage(hCombox, CB_SETCURSEL, 0, 0);
				//加载字符
				 hCombox = ::GetDlgItem(hwnd, IDC_COMBO3);
				for (int i = 0; i < EQ_COUNT; i++)
				{
					wcscpy_s(m_eqname[i].etext,100, ceqname[i]);
					SendMessage(hCombox, CB_ADDSTRING, 0,	(LPARAM) (LPCSTR) m_eqname[i].etext); 
					//SendMessage(hCombox, CB_SETITEMHEIGHT, 0, i * 50);
				}
				SendMessage(hCombox, CB_SETCURSEL, (EQ_COUNT-1), 0);
				//加载音频流
				hCombox = ::GetDlgItem(hwnd, IDC_COMBO2);
				DWORD ncount = 0, ncur = 0;
				m_pSoundtrans->get_astream(&ncount, &ncur);
				if(ncount == 0 ) ncount = 1;
				for (UINT i = 0; i< ncount; i++)
				{
					WCHAR lp[100];
					memset(lp, 0, 200);
					wsprintf(lp, L"音频流 %d", i+1);
					SendMessage(hCombox, CB_ADDSTRING, 0,	(LPARAM)lp); 
					//SendMessage(hCombox, CB_SETITEMHEIGHT, 0, i * 30);
				}
				//SendMessage(hCombox, CB_SETITEMHEIGHT, 0, 40);
				SendMessage(hCombox, CB_SETCURSEL, ncur, 0);


				
				//加载保存WAV数据
				CHAR psavepath[MAX_PATH];
				memset(psavepath, 0, MAX_PATH);
				LONG bsavetype = 0;
				m_pSoundtrans->get_wav(&bsavetype, psavepath);
				if(bsavetype == 1 || bsavetype == 2)//wav or mp3
				{
					HWND hWnd = ::GetDlgItem(hwnd, IDC_CHECK1);
					::SendMessage( hWnd, (UINT)BM_SETCHECK, BST_CHECKED, 0 );
				}
				if(*psavepath != 0)
				{
					::SetDlgItemTextA(hwnd, IDC_EDIT1, psavepath);
				}
			}
            return 1;
        }
        case WM_HSCROLL:
        {
            ChangerSound(hwnd, wParam, lParam);
            return 1;
        }
		case WM_VSCROLL:
		{
            ChangerSound(hwnd, wParam, lParam);
            return 1;
		}
        case WM_COMMAND:
        {
			UINT ncommand = LOWORD(wParam);
			SetDirty();
			switch(ncommand)
			{
			case IDC_EDIT2:
				{
					if(HIWORD(wParam) == EN_CHANGE)
					{
						WCHAR ap[MAX_PATH];
						memset(ap, 0, MAX_PATH);
						::GetDlgItemText(hwnd, IDC_EDIT2, ap, MAX_PATH);
						LONGLONG lshifttime = 0;
						lshifttime = _wtoi(ap);
						lshifttime = lshifttime * 10000i64;
						m_pSoundtrans->put_soundshift(lshifttime);						
					}
				}
				break;
			case IDC_RADIO1:
			case IDC_RADIO2:
			case IDC_RADIO3:
			case IDC_RADIO4:
			case IDC_RADIO5:
			case IDC_RADIO6:
				{
					char achannel = (char)(ncommand - IDC_RADIO1);
					if(m_pSoundtrans)
					{
						m_pSoundtrans->put_channel(achannel);
					}
				}
				break;
			case IDC_CHECK2:
				{
					HWND hWnd = ::GetDlgItem(hwnd, IDC_CHECK2);
					LRESULT hrAuto = ::SendMessage( hWnd, (UINT)BM_GETCHECK,0,0);
					bool bseq = BST_UNCHECKED == hrAuto ? false : true;
					if(m_pSoundtrans)
					{
						m_pSoundtrans->put_seq(bseq);
					}
				}
				break;
			case IDC_CHECK1:	//写WAV文件
				{
					HWND hWnd = ::GetDlgItem(hwnd, IDC_CHECK1);
					LRESULT hrAuto = ::SendMessage( hWnd, (UINT)BM_GETCHECK,0,0);
					bool bseq = BST_UNCHECKED == hrAuto ? false : true;
					long bsavetype = 0;
					if(bseq)	//保存
					{
						WCHAR pszFile[MAX_PATH];
						memset(pszFile, 0, MAX_PATH);
						WCHAR pszfilter[MAX_PATH];
						memset(pszfilter, 0, MAX_PATH);
						WCHAR *pszfilter1 = pszfilter;
						int nlen = wsprintf(pszfilter1, L"wav 文件 ");
						pszfilter1 += nlen+1;
						nlen = wsprintf(pszfilter1, L"*.wav");
						pszfilter1 += nlen+1;
						nlen = wsprintf(pszfilter1, L"mp3 文件 ");
						pszfilter1 += nlen+1;
						nlen = wsprintf(pszfilter1, L"*.mp3");
						OPENFILENAME ofn;
						ZeroMemory(&ofn, sizeof(ofn));
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = hwnd;
						ofn.lpstrTitle = L"保存wav";
						ofn.lpstrFilter = pszfilter;
						ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
						ofn.lpstrFile = pszFile;
						ofn.nMaxFile = MAX_PATH;
						ofn.nFilterIndex = 1;
						BOOL blnSave = GetSaveFileName(&ofn);
						if(blnSave)
						{//
							if(ofn.nFilterIndex == 1)
							{
								if(wcsstr(pszFile, L".wav") == NULL)
								{
									wcscat(pszFile, L".wav");
								}
								bsavetype = 1;
							}
							else
							{
								if(wcsstr(pszFile, L".mp3") == NULL)
								{
									wcscat(pszFile, L".mp3");
								}
								bsavetype = 2;
							}

							SetDlgItemText(hwnd, IDC_EDIT1, pszFile);
							//AllInput(COMBOX_INPUT, pszFile);
						}						
					}
					CHAR *psavepath = NULL;
					if(bsavetype > 0)
					{
						psavepath = new CHAR[MAX_PATH];
						memset(psavepath, 0, MAX_PATH);
						::GetDlgItemTextA(hwnd, IDC_EDIT1, psavepath, MAX_PATH);
						if(strlen(psavepath) < 4 || (strstr(psavepath, ".wav") == NULL && strstr(psavepath, ".mp3") == NULL))
						{
							delete []psavepath;
							psavepath = NULL;
							return 1;
						}
					}
					if(m_pSoundtrans)
					{
						m_pSoundtrans->put_wav(bsavetype, psavepath);
					}
					if(psavepath)
						delete []psavepath;
					psavepath = NULL;
				}
				break;
			case IDC_COMBO3:
				{
					UINT notification  = HIWORD(wParam);
					if(notification == CBN_SELCHANGE)
					{
						HWND hCombox = ::GetDlgItem(hwnd, IDC_COMBO3);
						int nindex = SendMessage(hCombox, CB_GETCURSEL, 0, 0);
						int neq[10];
						psp _psp = X_SP;
						_psp = _psp + 2;
						short *peq = _X_DEFAULT[nindex].eq;
						for (int i = 0; i < 10; i++)
						{
							neq[i] = peq[i];
							SendMessage(_psp->hwnd, TBM_SETPOS, TRUE, neq[i]);
							_psp++;
						}
						if(m_pSoundtrans)
						{
							m_pSoundtrans->put_eq(neq);
						}
					}
				}
				break;
			case IDC_COMBO2:
				{
					UINT notification  = HIWORD(wParam);
					if(notification == CBN_SELCHANGE)
					{
						HWND hCombox = ::GetDlgItem(hwnd, IDC_COMBO2);
						DWORD nindex = SendMessage(hCombox, CB_GETCURSEL, 0, 0);
						if(m_pSoundtrans)
						{
							m_pSoundtrans->put_astream(nindex+1);
						}
					}
				}
				break;
			case IDC_BUTTON1:
				{
					m_pSoundtrans->PutChannelNum(6.1);

					//int neq[10];
					//psp _psp = X_SP;
					//_psp = _psp + 2;
					//for (int i = 0; i < 10; i++)
					//{
					//	neq[i] = CHAR_100;
					//	SendMessage(_psp->hwnd, TBM_SETPOS, TRUE, CHAR_100);
					//	_psp++;
					//}
					//if(m_pSoundtrans)
					//	m_pSoundtrans->put_eq(neq);
					//HWND hCombox = ::GetDlgItem(hwnd, IDC_COMBO3);
					//SendMessage(hCombox, CB_SETCURSEL, (EQ_COUNT-1), 0);
					////pa default
					//_psp = X_SP;
					//_psp++;
					//::SendMessage(_psp->hwnd, TBM_SETPOS, TRUE, 0);
					//if(m_pSoundtrans)
					//{
					//	m_pSoundtrans->put_balance(0);
					//	char lptext[MAX_PATH];
					//	sprintf(lptext, "%d", 0);
					//	SetDlgItemTextA(hwnd, IDC_STATIC5, lptext);
					//}
				}
				break;
			case IDC_CHANNEL:
				m_pSoundtrans->SetStereo();
				break;
			case IDC_RECOVER:
				m_pSoundtrans->PutChannelNum(9);
				break;

			default:
				break;
			}
        }
		break;
        case WM_DESTROY:
        {
            return (LRESULT) 1;
        }

    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
} 

HRESULT CSoundProperty::OnConnect(IUnknown *pUnknown)
{
    CheckPointer(pUnknown,E_POINTER);
    HRESULT hr = pUnknown->QueryInterface(IID_ISoundtrans, (void **) &m_pSoundtrans);
    if(FAILED(hr))
    {
		m_pSoundtrans = NULL;
        return E_NOINTERFACE;
    }
    return NOERROR;
} 

HRESULT CSoundProperty::OnDisconnect()
{
    if (!m_pSoundtrans)
        return E_UNEXPECTED;
    m_pSoundtrans->Release();
    m_pSoundtrans = NULL;
    return NOERROR;
}

HRESULT CSoundProperty::OnApplyChanges()
{
	m_bDirty = FALSE;
	return NOERROR;
}

HWND CSoundProperty::CreateSlider(HWND hwndParent, psp _psp, short value)
{
    ULONG Styles = WS_CHILD | WS_VISIBLE | WS_TABSTOP |TBS_NOTICKS| TBS_BOTH|_psp->v_h;
    HWND hwndSlider = CreateWindow(TRACKBAR_CLASS, TEXT(""), Styles, _psp->x, _psp->y, _psp->xw, _psp->yh,
                                   hwndParent, NULL, g_hInst, NULL);
    if(hwndSlider == NULL)
    {
        return NULL;
    }
	_psp->hwnd = hwndSlider;
    SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(_psp->nmin, _psp->nmax));
    SendMessage(hwndSlider, TBM_SETTIC, 0, 0L);
    SendMessage(hwndSlider, TBM_SETPOS, TRUE, value);
    return hwndSlider;
}

HWND CSoundProperty::CreateEdit(HWND hwndParent, psp _psp, short value)
{
	ULONG Styles = WS_CHILD | WS_BORDER | WS_VISIBLE/* | WS_TABSTOP |TBS_NOTICKS| TBS_BOTH|_psp->v_h*/| ES_READONLY;
	HWND hwndSlider = CreateWindow(WC_EDIT, TEXT(""), Styles, _psp->x, _psp->y, _psp->xw, _psp->yh,
		hwndParent, NULL, g_hInst, NULL);
	if(hwndSlider == NULL)
	{
		return NULL;
	}
	_psp->hwnd = hwndSlider;
	//SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(_psp->nmin, _psp->nmax));
	//SendMessage(hwndSlider, TBM_SETTIC, 0, 0L);
	//SendMessage(hwndSlider, TBM_SETPOS, TRUE, value);
	return hwndSlider;
}

void CSoundProperty::ChangerSound(HWND hdlg, WPARAM wParam, LPARAM lParam)
{
	HWND htemp = (HWND)lParam;
	UINT ncommand = LOWORD(wParam);
	switch(ncommand)
	{
	case TB_BOTTOM:
			break;
		case TB_TOP:
			break;
		case TB_THUMBPOSITION:
		case TB_THUMBTRACK:
		case TB_ENDTRACK:
		{
			SetDirty();
			int nlevel = 0;
			nlevel = (int)SendMessage(htemp, TBM_GETPOS, 0, 0L);
			sp *_psp = X_SP;
			sp *_psp1 = &X_SP[1];
			if(m_pSoundtrans)
			{
				if(htemp == _psp->hwnd)
				{
					m_pSoundtrans->put_addsound(nlevel);
					char lptext[MAX_PATH];
					sprintf(lptext, "%0.1fx", (float)nlevel/10);
					SetDlgItemTextA(hdlg, IDC_STATIC6, lptext);
				}
				else if(htemp == _psp1->hwnd)
				{
					m_pSoundtrans->put_balance(nlevel);
					char lptext[MAX_PATH];
					sprintf(lptext, "%d", nlevel);
					SetDlgItemTextA(hdlg, IDC_STATIC5, lptext);
				}
				else					//要改变均衡器
				{
					int neq[10];
					psp _psp = X_SP;
					_psp++;
					_psp++;
					for (int i = 0; i < 10; i++)
					{
						int npos = (int)SendMessage(_psp->hwnd, TBM_GETPOS, 0, 0L);
						neq[i] = 200 - npos;
						_psp++;
					}
					m_pSoundtrans->put_eq(neq);
				}
			}

			break;
		}
		default:
			break;
	}
}