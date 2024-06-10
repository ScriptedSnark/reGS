#ifndef PUBLIC_REGISTRY_H
#define PUBLIC_REGISTRY_H

#ifndef WIN32
#include <cstdio>

#include <UtlVector.h>
#endif

#include "IRegistry.h"

class CRegistry final : public IRegistry
{
private:
	struct KV
	{
		char key[ 64 ];
		char value[ 64 ];
	};

public:
	CRegistry() = default;
	~CRegistry() = default;

	void Init() override;
	void Shutdown() override;

	int ReadInt( const char* key, int defaultValue ) override;
	void WriteInt( const char* key, int value ) override;

	const char* ReadString( const char* key, const char* defaultValue ) override;
	void WriteString( const char* key, const char* string ) override;

private:
#ifndef WIN32
	void LoadKeyValuesFromDisk();
	void WriteKeyValuesToDisk();
	void GetKeyValues();
#endif

private:
#ifdef WIN32
	HKEY m_hKey = NULL;
	bool m_bInitialized = false;
#else
	bool m_bValid = false;
	int m_hKey = 0;
	FILE* m_fConfig = nullptr;
	CUtlVector<KV> m_vecValues;
#endif
};

#endif //PUBLIC_REGISTRY_H
