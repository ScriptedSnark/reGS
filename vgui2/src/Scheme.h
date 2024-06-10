#ifndef VGUI2_SRC_SCHEME_H
#define VGUI2_SRC_SCHEME_H

#include <SDK_Color.h>
#include <UtlRBTree.h>
#include <UtlVector.h>
#include <vgui/IScheme.h>

class KeyValues;

namespace vgui2
{
class Bitmap;
class Border;
}

class CScheme : public vgui2::IScheme
{
protected:
	friend class CSchemeManager;

private:
	struct SchemeBorder_t
	{
		vgui2::Border* border;
		int borderSymbol;
		bool bSharedBorder;
	};

	struct fontalias_t
	{
		char _fontName[ 64 ];
		vgui2::HFont _font;
	};

	static const size_t MAX_FONT_MUNGE_BUFFER = 64;

public:
	CScheme();
	~CScheme();

	const char *GetResourceString( const char *stringName ) override;

	vgui2::IBorder *GetBorder( const char *borderName ) override;

	vgui2::HFont GetFont( const char *fontName, bool proportional = false ) override;

	SDK_Color GetColor( const char *colorName, SDK_Color defaultColor ) override;

	void LoadFromFile( const char* infilename, const char* inTag, KeyValues* inKeys );

	void Shutdown( bool full );

private:
	const char* GetMungedFontName( const char* fontName, const char* scheme, bool proportional );
	vgui2::HFont FindFontInAliasList( const char* fontName );

	int GetAdjustedFontHeightForCurrentLanguage( int tall );

	void LoadFonts();
	void LoadBorders();

private:
	char fileName[ 256 ] = { '\0' };
	char tag[ 64 ] = { '\0' };

	KeyValues* data = nullptr;
	KeyValues* baseSettings = nullptr;
	KeyValues* colors = nullptr;

	CUtlVector<SchemeBorder_t> m_BorderList;
	vgui2::IBorder* _baseBorder = nullptr;
	KeyValues* _borders = nullptr;

	CUtlVector<CScheme::fontalias_t> _fontAliases;

private:
	CScheme( const CScheme& ) = delete;
	CScheme& operator=( const CScheme& ) = delete;
};

class CSchemeManager : public vgui2::ISchemeManager009
{
private:
	struct CachedBitmapHandle_t
	{
		vgui2::Bitmap* bitmap;
	};

	static const vgui2::HScheme DEFAULT_SCHEME = 1;

public:
	CSchemeManager();
	~CSchemeManager();

	vgui2::HScheme LoadSchemeFromFile( const char *fileName, const char *tag ) override;

	void ReloadSchemes() override;

	vgui2::HScheme GetDefaultScheme() override;

	vgui2::HScheme GetScheme( const char *tag ) override;

	vgui2::IImage *GetImage( const char *imageName, bool hardwareFiltered ) override;
	vgui2::HTexture GetImageID( const char *imageName, bool hardwareFiltered ) override;
#ifdef _XBOX
	void DeleteImage( const char *pImageName ) override;
#endif

	vgui2::IScheme *GetIScheme( vgui2::HScheme scheme ) override;

	void Shutdown( bool full = true ) override;

	int GetProportionalScaledValue( int normalizedValue ) override;
	int GetProportionalNormalizedValue( int scaledValue ) override;

private:
	static bool BitmapHandleSearchFunc( const CachedBitmapHandle_t& lhs, const CachedBitmapHandle_t& rhs );

	vgui2::HScheme FindLoadedScheme( const char* fileName );

private:
	static const char* s_pszSearchString;

	CUtlVector<CScheme*> _scheme;
	CUtlRBTree<CachedBitmapHandle_t, int> m_Bitmaps;

private:
	CSchemeManager( const CSchemeManager& ) = delete;
	CSchemeManager& operator=( const CSchemeManager& ) = delete;
};

#endif //VGUI2_SRC_SCHEME_H
