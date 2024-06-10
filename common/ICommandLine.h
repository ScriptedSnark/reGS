#ifndef COMMON_ICOMMANDLINE_H
#define COMMON_ICOMMANDLINE_H

/**
*	Stores off the command line and allows modification.
*/
class ICommandLine
{
public:
	virtual void CreateCmdLine( const char* commandline ) = 0;
	virtual void CreateCmdLine( int argc, const char** argv ) = 0;

	virtual char* GetCmdLine() = 0;

	virtual char* CheckParm( const char* psz, char** ppszValue ) = 0;

	virtual void RemoveParm( const char* pszParm ) = 0;
	virtual void AppendParm( const char* pszParm, const char* pszValues ) = 0;
	
	virtual void SetParm( const char* pszParm, const char* pszValues ) = 0;
	virtual void SetParm( const char* pszParm, int iValue ) = 0;
};

/**
*	Global command line instance.
*/
extern ICommandLine* cmdline;

#endif //COMMON_ICOMMANDLINE_H
