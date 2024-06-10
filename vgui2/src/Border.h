#ifndef VGUI2_SRC_BORDER_H
#define VGUI2_SRC_BORDER_H

#include <SDK_Color.h>
#include <vgui/IBorder.h>

namespace vgui2
{
class Border : public IBorder
{
private:
	struct line_t
	{
		SDK_Color col;
		int startOffset;
		int endOffset;
	};

	struct side_t
	{
		int count;
		line_t* lines;
	};

public:
	Border();
	~Border();

	void Paint( VPANEL panel ) override;
	void Paint( int x0, int y0, int x1, int y1 ) override;
	void Paint( int x0, int y0, int x1, int y1, int breakSide, int breakStart, int breakStop ) override;
	void SetInset( int left, int top, int right, int bottom ) override;
	void GetInset( int &left, int &top, int &right, int &bottom ) override;
	void ApplySchemeSettings( IScheme *pScheme, KeyValues *inResourceData ) override;
	const char *GetName() override;
	void SetName( const char *name ) override;

private:
	void ParseSideSettings( int side_index, KeyValues* inResourceData, IScheme* pScheme );

private:
	int _inset[ 4 ] = {};

	char* _name = nullptr;

	side_t _sides[ 4 ] = {};

private:
	Border( const Border& ) = delete;
	Border& operator=( const Border& ) = delete;
};
}

#endif //VGUI2_SRC_BORDER_H
