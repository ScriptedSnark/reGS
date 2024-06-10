#ifndef VGUI2_SRC_MEMORYBITMAP_H
#define VGUI2_SRC_MEMORYBITMAP_H

#include <tier0/platform.h>
#include <SDK_Color.h>
#include <vgui/IImage.h>

namespace vgui2
{
class MemoryBitmap : public IImage
{
public:
	MemoryBitmap( byte* texture, int wide, int tall );
	~MemoryBitmap();

	void Paint() override;

	void SetPos( int x, int y ) override;

	void GetContentSize( int &wide, int &tall ) override;

	void GetSize( int &wide, int &tall ) override;

	void SetSize( int wide, int tall ) override;

	void SetColor( SDK_Color col ) override;

	const char* GetName() { return "MemoryBitmap"; }

	HTexture GetID() { return _id; }

private:
	void ForceUpload();

private:
	HTexture _id = NULL_HANDLE;
	bool _uploaded = false;
	bool _valid = true;

	byte* _texture;

	int _pos[ 2 ] = {};

	SDK_Color _color = SDK_Color( 255, 255, 255, 255 );

	int _w;
	int _h;

private:
	MemoryBitmap( const MemoryBitmap& ) = delete;
	MemoryBitmap& operator=( const MemoryBitmap& ) = delete;
};
}

#endif //VGUI2_SRC_MEMORYBITMAP_H
