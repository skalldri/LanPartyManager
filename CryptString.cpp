// CryptString.cpp: Implementierung der Klasse CCryptString.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CryptString.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CCryptString::CCryptString()
{
	m_bInitOK = FALSE;
	m_bInitOK = SetupCryptoClient();
}

CCryptString::~CCryptString()
{

}

BOOL CCryptString::SetupCryptoClient()
{
	// Ensure that the default cryptographic client is set up.	
	HCRYPTPROV hProv;
	HCRYPTKEY hKey;		
	// Attempt to acquire a handle to the default key container.
	if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0))	
	{
		// Some sort of error occured, create default key container.
		if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET))
		{	
			// Error creating key container!			
			return FALSE;		
		}	
	}
	// Attempt to get handle to signature key.
	if (!CryptGetUserKey(hProv, AT_SIGNATURE, &hKey))	
	{
		if (GetLastError() == NTE_NO_KEY)		
		{			
			// Create signature key pair.
			if (!CryptGenKey(hProv, AT_SIGNATURE, 0, &hKey))			
			{
				// Error during CryptGenKey!				
				CryptReleaseContext(hProv, 0);
				return FALSE;			
			}			
			else			
			{				
				CryptDestroyKey(hKey);			
			}		
		}		
		else 		
		{
			// Error during CryptGetUserKey!			
			CryptReleaseContext(hProv, 0);
			return FALSE;		
		}	
	}	
	
	// Attempt to get handle to exchange key.
	if (!CryptGetUserKey(hProv,AT_KEYEXCHANGE,&hKey))	
	{
		if (GetLastError()==NTE_NO_KEY)		
		{			
			// Create key exchange key pair.
			if (!CryptGenKey(hProv,AT_KEYEXCHANGE,0,&hKey))			
			{
				// Error during CryptGenKey!				
				CryptReleaseContext(hProv, 0);
				return FALSE;			
			}			
			else			
			{				
				CryptDestroyKey(hKey);			
			}		
		}		
		else		
		{
			// Error during CryptGetUserKey!			
			CryptReleaseContext(hProv, 0);
			return FALSE;		
		}	
	}	

	CryptReleaseContext(hProv, 0);	
	return TRUE;
}


BOOL CCryptString::EncryptString(TCHAR* szPassword,TCHAR* szEncryptPwd,TCHAR *szKey)
{	
	
	// was intialize OK?
	if (!m_bInitOK)
		return FALSE;

	BOOL bResult = TRUE;	
	HKEY hRegKey = NULL;	
	HCRYPTPROV hProv = NULL;	
	HCRYPTKEY hKey = NULL;
	HCRYPTKEY hXchgKey = NULL;	
	HCRYPTHASH hHash = NULL;	
	DWORD dwLength;

	// Get handle to user default provider.
	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0))	
	{
		// Create hash object.		
		if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))		
		{
			// Hash password string.			
			dwLength = sizeof(TCHAR)*_tcslen(szKey);
			if (CryptHashData(hHash, (BYTE *)szKey, dwLength, 0))			
			{
				// Create block cipher session key based on hash of the password.
				if (CryptDeriveKey(hProv, MY_ENCRYPT, hHash, CRYPT_EXPORTABLE, &hKey))				
				{
					// Determine number of bytes to encrypt at a time.
					dwLength = sizeof(TCHAR)*_tcslen(szPassword);					
					// Allocate memory.
					BYTE *pbBuffer = (BYTE *)malloc(dwLength);					
					if (pbBuffer != NULL)					
					{
						memcpy(pbBuffer, szPassword, dwLength);						
						// Encrypt data
						if (CryptEncrypt(hKey, 0, TRUE, 0, pbBuffer, &dwLength, dwLength)) 						
						{
							// return encrypted string
							memcpy(szEncryptPwd, pbBuffer, dwLength);

						}	
						else						
						{							
							bResult = FALSE;						
						}						
						// Free memory
						free(pbBuffer);					
					}
					else					
					{						
						bResult = FALSE;					
					}
					CryptDestroyKey(hKey);  // Release provider handle.				
				}				
				else				
				{
					// Error during CryptDeriveKey!					
					bResult = FALSE;				
				}			
			}			
			else			
			{
				// Error during CryptHashData!				
				bResult = FALSE;			
			}
			CryptDestroyHash(hHash); 
			// Destroy session key.		
		}		
		else		
		{
			// Error during CryptCreateHash!			
			bResult = FALSE;		
		}
		CryptReleaseContext(hProv, 0);	
	}	
	return bResult;
}


BOOL CCryptString::DecryptString(TCHAR* szEncryptPwd,TCHAR* szPassword,TCHAR *szKey) 
{	
	// was initalise OK?
	if (!m_bInitOK)
		return FALSE;

	BOOL bResult = TRUE;	
	HCRYPTPROV hProv = NULL;		
	HCRYPTKEY hKey = NULL;		
	HCRYPTKEY hXchgKey = NULL;
	HCRYPTHASH hHash = NULL;
	TCHAR szPasswordTemp[32] = _T("");
	DWORD dwLength;

	// Get handle to user default provider.
	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0))		
	{
		// Create hash object.			
		if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
		{				
			// Hash password string.
			dwLength = sizeof(TCHAR)*_tcslen(szKey);
			if (CryptHashData(hHash, (BYTE *)szKey, dwLength, 0))				
			{
				// Create block cipher session key based on hash of the password.
				if (CryptDeriveKey(
					hProv, MY_ENCRYPT, hHash, CRYPT_EXPORTABLE, &hKey))					
				{
					// we know the encrypted password and the length
					dwLength = sizeof(TCHAR)*_tcslen(szEncryptPwd);						
					// copy encrypted password to temporary TCHAR
					_tcscpy(szPasswordTemp,szEncryptPwd);
					if (!CryptDecrypt(
							hKey, 0, TRUE, 0, (BYTE *)szPasswordTemp, &dwLength))
						bResult = FALSE;						
					CryptDestroyKey(hKey);  // Release provider handle.					
					// copy decrypted password to outparameter
					_tcscpy(szPassword,szPasswordTemp);
				}					
				else					
				{
					// Error during CryptDeriveKey!						
					bResult = FALSE;					
				}				
			}				
			else
			{					
				// Error during CryptHashData!					
				bResult = FALSE;				
			}
			CryptDestroyHash(hHash); // Destroy session key.			
		}			
		else			
		{
			// Error during CryptCreateHash!				
			bResult = FALSE;			
		}
		CryptReleaseContext(hProv, 0);		
	}		
	return bResult;
}
