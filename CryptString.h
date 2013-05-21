// CryptString.h: Schnittstelle für die Klasse CCryptString.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRYPTSTRING_H__FED1A219_D7A0_4030_9539_0B1F94E7EDBA__INCLUDED_)
#define AFX_CRYPTSTRING_H__FED1A219_D7A0_4030_9539_0B1F94E7EDBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCryptString  
{
public:
	CCryptString();
	virtual ~CCryptString();
	BOOL DecryptString(TCHAR* szEncryptPwd,TCHAR* szPassword,TCHAR *szKey);
	BOOL EncryptString(TCHAR* szPassword,TCHAR* szEncryptPwd,TCHAR *szKey);
private:
	SetupCryptoClient();
	BOOL m_bInitOK;
};

#endif // !defined(AFX_CRYPTSTRING_H__FED1A219_D7A0_4030_9539_0B1F94E7EDBA__INCLUDED_)
