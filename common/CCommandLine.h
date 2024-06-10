#ifndef COMMON_CCOMMANDLINE_H
#define COMMON_CCOMMANDLINE_H

#include "ICommandLine.h"

class CCommandLine final : public ICommandLine
{
public:
	CCommandLine();
	~CCommandLine();

	void CreateCmdLine( const char* commandline ) override;
	void CreateCmdLine( int argc, const char** argv ) override;

	char* GetCmdLine() override;

	char* CheckParm( const char* psz, char** ppszValue ) override;

	void RemoveParm( const char* pszParm ) override;
	void AppendParm( const char* pszParm, const char* pszValues ) override;

	void SetParm( const char* pszParm, const char* pszValues )override;
	void SetParm( const char* pszParm, int iValue ) override;

private:
	char* m_pszCmdLine = nullptr;
};

#endif //COMMON_CCOMMANDLINE_H
