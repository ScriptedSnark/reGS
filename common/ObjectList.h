#ifndef COMMON_OBJECTLIST_H
#define COMMON_OBJECTLIST_H

#include "IObjectContainer.h"

/**
*	Type erased linked list of objects.
*/
class ObjectList : public IObjectContainer
{
private:
	//A single list node.
	struct element_t
	{
		element_t* prev;
		element_t* next;

		void *object;
	};

public:
	ObjectList() = default;
	virtual ~ObjectList();

	void Init() override;

	bool Add( void* newObject ) override;
	bool Remove( void* object ) override;
	void Clear( bool freeElementsMemory ) override;

	void* GetFirst() override;
	void* GetNext() override;

	int CountElements() override;

	bool Contains( void* object ) override;
	bool IsEmpty() override;

	bool AddHead( void* newObject );
	void* RemoveHead();

	bool AddTail( void* newObject );
	void* RemoveTail();

private:
	element_t* head = nullptr;
	element_t* tail = nullptr;
	element_t* current = nullptr;
	int number = 0;

private:
	ObjectList( const ObjectList& ) = delete;
	ObjectList& operator=( const ObjectList& ) = delete;
};

#endif //COMMON_OBJECTLIST_H
