#ifndef VGUI2_SRC_BITMAP_H
#define VGUI2_SRC_BITMAP_H

#include <SDK_Color.h>
#include <vgui/IImage.h>

namespace vgui2
{
class Bitmap :  public IImage
{
public:
	Bitmap( const char* filename, bool hardwareFiltered );
	~Bitmap();

	void Paint() override;

	void SetPos( int x, int y ) override;

	void GetContentSize( int &wide, int &tall ) override;

	void GetSize( int &wide, int &tall ) override;

	void SetSize( int wide, int tall ) override;

	void SetColor( SDK_Color col ) override;

	const char* GetName() { return _filename; }

	HTexture GetID() { return _id; }

private:
	void ForceUpload();

private:
	HTexture _id = NULL_HANDLE;
	bool _uploaded = false;
	bool _valid = true;

	char* _filename;

	int _pos[ 2 ] = {};

	SDK_Color _color = SDK_Color( 255, 255, 255, 255 );

	bool _filtered;

	int wide = 0;
	int tall = 0;

private:
	Bitmap( const Bitmap& ) = delete;
	Bitmap& operator=( const Bitmap& ) = delete;
};
}

#endif //VGUI2_SRC_BITMAP_H
