#ifndef PUBLIC_IREGISTRY_H
#define PUBLIC_IREGISTRY_H

/**
*	Provides access to the registry.
*/
class IRegistry
{
public:
	/**
	*	Initializes the registry.
	*/
	virtual void Init() = 0;

	/**
	*	Shuts down the registry.
	*/
	virtual void Shutdown() = 0;

	virtual int ReadInt( const char* key, int defaultValue ) = 0;
	virtual void WriteInt( const char* key, int value ) = 0;

	virtual const char* ReadString( const char* key, const char* defaultValue ) = 0;
	virtual void WriteString( const char* key, const char* string ) = 0;
};

/**
*	Global registry instance.
*/
extern IRegistry* registry;

#endif //PUBLIC_IREGISTRY_H
