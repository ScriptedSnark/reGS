#include "quakedef.h"
#include "ObjectList.h"

ObjectList::~ObjectList()
{
	Clear( false );
}

void ObjectList::Init()
{
	current = nullptr;
	tail = nullptr;
	head = nullptr;
	number = 0;
}

bool ObjectList::Add( void* newObject )
{
	auto pNode = reinterpret_cast<element_t*>( Mem_ZeroMalloc( sizeof( element_t ) ) );

	if( !pNode )
		return false;

	pNode->object = newObject;

	if( tail )
	{
		pNode->prev = tail;
		tail->next = pNode;
	}

	tail = pNode;

	if( !head )
		head = pNode;

	++number;

	return true;
}

bool ObjectList::Remove( void* object )
{
	if( !head )
		return false;

	auto pNode = head;

	while( pNode->object != object )
	{
		pNode = pNode->next;

		if( !pNode )
			return false;
	}

	if( pNode->prev )
		pNode->prev->next = pNode->next;

	if( pNode->next )
		pNode->next->prev = pNode->prev;

	if( head == pNode )
		head = pNode->next;

	if( tail == pNode )
		tail = pNode->prev;

	if( current == pNode )
		current = pNode->next;

	Mem_Free( pNode );

	--number;

	return true;
}

void ObjectList::Clear( bool freeElementsMemory )
{
	if( !head )
		return;

	for( auto pNode = head; pNode; pNode = pNode->next )
	{
		if( freeElementsMemory )
			Mem_Free( pNode->object );

		Mem_Free( pNode );
	}

	current = nullptr;
	tail = nullptr;
	head = nullptr;
	number = 0;
}

void* ObjectList::GetFirst()
{
	if( head )
	{
		current = head->next;
		return head->object;
	}
	else
	{
		current = nullptr;
		return nullptr;
	}
}

void* ObjectList::GetNext()
{
	if( current )
	{
		auto result = current->object;
		current = current->next;
		return result;
	}
	else
	{
		return nullptr;
	}
}

int ObjectList::CountElements()
{
	return number;
}

bool ObjectList::Contains( void* object )
{
	for( auto pNode = head; pNode; pNode = pNode->next )
	{
		if( pNode->object == object )
		{
			current = pNode;
			return true;
		}
	}

	return false;
}

bool ObjectList::IsEmpty()
{
	return head == nullptr;
}

bool ObjectList::AddHead( void* newObject )
{
	auto pNode = reinterpret_cast<element_t*>( Mem_ZeroMalloc( sizeof( element_t ) ) );

	if( !pNode )
		return false;

	pNode->object = newObject;

	if( head )
	{
		pNode->next = head;
		head->prev = pNode;
	}

	head = pNode;

	if( !tail )
		tail = pNode;

	++number;

	return true;
}

void* ObjectList::RemoveHead()
{
	if( !head )
		return nullptr;

	auto pNode = head;

	head = pNode->next;

	if( head )
		head->prev = nullptr;

	auto pObject = pNode->object;

	if( tail == pNode )
		tail = nullptr;

	Mem_Free( pNode );

	--number;

	return pObject;
}

bool ObjectList::AddTail( void* newObject )
{
	auto pNode = reinterpret_cast<element_t*>( Mem_ZeroMalloc( sizeof( element_t ) ) );

	if( !pNode )
		return false;

	pNode->object = newObject;

	if( tail )
	{
		pNode->prev = head;
		tail->next = pNode;
	}

	tail = pNode;

	if( !head )
		head = pNode;

	++number;

	return true;
}

void* ObjectList::RemoveTail()
{
	if( !tail )
		return nullptr;

	auto pNode = tail;

	tail = pNode->prev;

	if( tail )
		tail->next = nullptr;

	auto pObject = pNode->object;

	if( head == pNode )
		head = nullptr;

	Mem_Free( pNode );

	--number;

	return pObject;
}
