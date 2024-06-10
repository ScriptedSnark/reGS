#ifndef COMMON_IOBJECTCONTAINER_H
#define COMMON_IOBJECTCONTAINER_H

/**
*	Type erased container of objects.
*/
class IObjectContainer
{
public:
	virtual ~IObjectContainer() {}

	virtual void Init() = 0;

	virtual bool Add( void* newObject ) = 0;
	virtual bool Remove( void* object ) = 0;
	virtual void Clear( bool freeElementsMemory ) = 0;

	virtual void* GetFirst() = 0;
	virtual void* GetNext() = 0;

	virtual int CountElements() = 0;

	virtual bool Contains( void* object ) = 0;
	virtual bool IsEmpty() = 0;
};

#endif //COMMON_IOBJECTCONTAINER_H
