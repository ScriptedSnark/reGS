#include <KeyValues.h>
#include <vgui/IKeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Controls.h>

#include "Bitmap.h"
#include "Border.h"
#include "Scheme.h"

using vgui2::ISchemeManager;

EXPOSE_SINGLE_INTERFACE( CSchemeManager, ISchemeManager, VGUI_SCHEME_INTERFACE_VERSION_GS );

CScheme::CScheme()
{
}

CScheme::~CScheme()
{
}

const char *CScheme::GetResourceString( const char *stringName )
{
	return baseSettings->GetString( stringName, "" );
}

vgui2::IBorder *CScheme::GetBorder( const char *borderName )
{
	auto hBorder = keyvalues()->GetSymbolForString( borderName );

	if( m_BorderList.Count() <= 0 )
		return _baseBorder;

	for( int i = 0; i < m_BorderList.Count(); ++i )
	{
		if( m_BorderList[ i ].borderSymbol == hBorder )
			return m_BorderList[ i ].border;
	}

	return _baseBorder;
}

vgui2::HFont CScheme::GetFont( const char *fontName, bool proportional )
{
	return FindFontInAliasList( GetMungedFontName( fontName, tag, proportional ) );
}

SDK_Color CScheme::GetColor( const char *colorName, SDK_Color defaultColor )
{
	int r, g, b, a;

	auto pszColor = colors->GetString( colorName, nullptr );

	if( pszColor )
	{
		sscanf( pszColor, "%d %d %d %d", &r, &g, &b, &a );
		return SDK_Color( r, g, b, a );
	}

	pszColor = baseSettings->GetString( colorName, nullptr );

	if( pszColor )
	{
		if( sscanf( pszColor, "%d %d %d %d", &r, &g, &b, &a ) >= 3 )
		{
			return SDK_Color( r, g, b, a );
		}

		char color[ 64 ];
		strncpy( color, pszColor, ARRAYSIZE( color ) - 1 );
		color[ ARRAYSIZE( color ) - 1 ] = '\0';
			
		//Check if it's a reference to another color
		return GetColor( color, defaultColor );
	}
	else if( sscanf( colorName, "%d %d %d %d", &r, &g, &b, &a ) >= 3 )
	{
		return SDK_Color( r, g, b, a );
	}

	return defaultColor;
}

void CScheme::LoadFromFile( const char* inFilename, const char* inTag, KeyValues* inKeys )
{
	strncpy( fileName, inFilename, ARRAYSIZE( fileName ) );
	fileName[ ARRAYSIZE( fileName ) - 1 ] = '\0';

	data = inKeys;

	baseSettings = data->FindKey( "BaseSettings", true );
	colors = data->FindKey( "Colors", true );

	auto pName = data->FindKey( "Name", true );

	pName->SetString( "Name", inTag );

	if( inTag )
	{
		strncpy( tag, inTag, ARRAYSIZE( tag ) );
	}
	else
	{
		strcpy( tag, "default" );
	}

	LoadFonts();
	LoadBorders();
}

void CScheme::Shutdown( bool full )
{
	for( int i = 0; i < m_BorderList.Count(); ++i )
	{
		auto& border = m_BorderList[ i ];

		if( !border.bSharedBorder )
			delete border.border;
	}

	_baseBorder = nullptr;
	_borders = nullptr;

	if( full )
	{
		if( data )
		{
			data->deleteThis();
			data = nullptr;
		}
	}
}

const char* CScheme::GetMungedFontName( const char* fontName, const char* scheme, bool proportional )
{
	static char mungeBuffer[ MAX_FONT_MUNGE_BUFFER ];

	const char* pszPostFix = "p";

	if( !proportional )
		pszPostFix = "no";

	if( scheme )
	{
		snprintf(
			mungeBuffer, ARRAYSIZE( mungeBuffer ),
			"%s%s-%s",
			fontName, scheme, pszPostFix
		);
	}
	else
	{
		snprintf(
			mungeBuffer, ARRAYSIZE( mungeBuffer ),
			"%s-%s",
			fontName, pszPostFix
		);
	}

	return mungeBuffer;
}

vgui2::HFont CScheme::FindFontInAliasList( const char* fontName )
{
	for( auto index = _fontAliases.Count() - 1; index >= 0; --index )
	{
		if( !strnicmp( fontName, _fontAliases[ index ]._fontName, MAX_FONT_MUNGE_BUFFER ) )
			return _fontAliases[ index ]._font;
	}

	return NULL_HANDLE;
}

int CScheme::GetAdjustedFontHeightForCurrentLanguage( int tall )
{
	char language[ 64 ];

	language[ 0 ] = '\0';

	auto newTall = tall;

	auto pszLanguage = vgui2::surface()->GetLanguage();

	if( pszLanguage && *pszLanguage )
		strncpy( language, pszLanguage, ARRAYSIZE( language ) );

	if( language[ 0 ] ||
		vgui2::system()->GetRegistryString(
			"HKEY_CURRENT_USER\\Software\\Valve\\Steam\\Language",
			language,
			ARRAYSIZE( language ) - 1 ) )
	{
		if( ( !stricmp( language, "korean" ) ||
			  !stricmp( language, "tchinese" ) ||
			  !stricmp( language, "schinese" ) ||
			  !stricmp( language, "thai" ) ) &&
			tall <= 12 )
		{
			newTall = 13;
		}
		else if( !stricmp( language, "russian" ) && tall > 12 )
		{
			newTall = 12;

			const auto scaledTall = static_cast<int>( floor( tall * 0.9 ) );

			if( scaledTall >= 12 )
				newTall = scaledTall;
		}
	}

	return newTall;
}

void CScheme::LoadFonts()
{
	{
		auto pCustomFonts = data->FindKey( "CustomFontFiles", true );

		for( auto pKV = pCustomFonts->GetFirstSubKey(); pKV; pKV = pKV->GetNextKey() )
		{
			auto pszFontFile = pKV->GetString();

			if( pszFontFile && *pszFontFile )
			{
				vgui2::surface()->AddCustomFontFile( pszFontFile );
			}
		}
	}

	int screenWide, screenTall;
	vgui2::surface()->GetScreenSize( screenWide, screenTall );

	int baseWide, baseTall;
	vgui2::surface()->GetProportionalBase( baseWide, baseTall );

	{
		auto pFonts = data->FindKey( "Fonts", true );

		for( auto pKV = pFonts->GetFirstSubKey(); pKV; pKV = pKV->GetNextKey() )
		{
			for( int i = 0; i < 2; ++i )
			{
				auto pszMungedName = GetMungedFontName( pKV->GetName(), tag, i != 0 );

				auto hFont = vgui2::surface()->CreateFont();

				for( auto pGlyph = pKV->GetFirstSubKey();
					 pGlyph;
					 pGlyph = pGlyph->GetNextKey() )
				{
					int fontYResMin = 0, fontYResMax = 0;
					sscanf( pGlyph->GetString( "yres", "" ), "%d %d", &fontYResMin, &fontYResMax );

					if( fontYResMin )
					{
						if( !fontYResMax )
							fontYResMax = fontYResMin;

						if( fontYResMin > screenTall || screenTall > fontYResMax )
							continue;
					}

					int flags = 0;

					if( pGlyph->GetInt( "italic" ) )
						flags |= vgui2::ISurface::FONTFLAG_ITALIC;

					if( pGlyph->GetInt( "underline" ) )
						flags |= vgui2::ISurface::FONTFLAG_UNDERLINE;

					if( pGlyph->GetInt( "symbol" ) )
						flags |= vgui2::ISurface::FONTFLAG_SYMBOL;

					if( pGlyph->GetInt( "antialias" ) &&
						vgui2::surface()->SupportsFeature( vgui2::ISurface::ANTIALIASED_FONTS ) )
						flags |= vgui2::ISurface::FONTFLAG_ANTIALIAS;

					if( pGlyph->GetInt( "dropshadow" ) &&
						vgui2::surface()->SupportsFeature( vgui2::ISurface::DROPSHADOW_FONTS ) )
						flags |= vgui2::ISurface::FONTFLAG_DROPSHADOW;

					if( pGlyph->GetInt( "rotary" ) )
						flags |= vgui2::ISurface::FONTFLAG_ROTARY;

					auto tall = pGlyph->GetInt( "tall" );
					const auto blur = pGlyph->GetInt( "blur" );
					auto scanlines = pGlyph->GetInt( "scanlines" );

					if( !fontYResMin && !fontYResMax && i != 0 )
					{
						tall = static_cast<int>( floor( static_cast<double>( screenTall ) / baseTall * tall ) );
						scanlines = static_cast<int>( floor( static_cast<double>( screenTall ) / baseTall * scanlines ) );
					}

					if( tall > 128 )
						tall = 128;

					if( !( flags & vgui2::ISurface::FONTFLAG_SYMBOL ) )
						tall = GetAdjustedFontHeightForCurrentLanguage( tall );

					const auto weight = pGlyph->GetInt( "weight" );
					const auto pszName = pGlyph->GetString( "name", "" );

					vgui2::surface()->AddGlyphSetToFont(
						hFont, pszName,
						tall, weight, blur, scanlines, flags,
						0, 0xFFFF
					);

					auto& alias = _fontAliases[ _fontAliases.AddToTail() ];

					strncpy( alias._fontName, pszMungedName, ARRAYSIZE( alias._fontName ) );
					alias._font = hFont;
				}
			}
		}
	}
}

void CScheme::LoadBorders()
{
	_borders = data->FindKey( "Borders", true );

	//Load all borders
	for( auto pKV = _borders->GetFirstSubKey();
		 pKV;
		 pKV = pKV->GetNextKey() )
	{
		if( pKV->GetDataType() != KeyValues::TYPE_STRING )
		{
			auto& border = m_BorderList[ m_BorderList.AddToTail() ];

			border.border = new vgui2::Border();

			border.border->SetName( pKV->GetName() );

			border.border->ApplySchemeSettings( this, pKV );
			border.bSharedBorder = false;
			border.borderSymbol = pKV->GetNameSymbol();
		}
	}

	//Map aliased borders to the real ones.
	for( auto pKV = _borders->GetFirstSubKey();
		 pKV;
		 pKV = pKV->GetNextKey() )
	{
		if( pKV->GetDataType() == KeyValues::TYPE_STRING )
		{
			auto pBorder = static_cast<vgui2::Border*>( GetBorder( pKV->GetString() ) );

			auto& border = m_BorderList[ m_BorderList.AddToTail() ];

			border.border = pBorder;
			border.bSharedBorder = true;
			border.borderSymbol = pKV->GetNameSymbol();
		}
	}

	//Fix up base border
	_baseBorder = GetBorder( "BaseBorder" );
}

const char* CSchemeManager::s_pszSearchString = nullptr;

bool CSchemeManager::BitmapHandleSearchFunc( const CachedBitmapHandle_t& lhs, const CachedBitmapHandle_t& rhs )
{
	if( lhs.bitmap )
	{
		if( rhs.bitmap )
		{
			return stricmp( lhs.bitmap->GetName(), rhs.bitmap->GetName() ) > 0;
		}
		else
		{
			return stricmp( lhs.bitmap->GetName(), s_pszSearchString ) > 0;
		}
	}

	return stricmp( s_pszSearchString, rhs.bitmap->GetName() ) > 0;
}

CSchemeManager::CSchemeManager()
	: m_Bitmaps( &CSchemeManager::BitmapHandleSearchFunc )
{
	auto nullScheme = new CScheme();

	_scheme.AddToTail( nullScheme );
}

CSchemeManager::~CSchemeManager()
{
	for( unsigned int i = 0; i < m_Bitmaps.Count(); ++i )
	{
		auto& bitmap = m_Bitmaps[ i ];

		if( bitmap.bitmap )
			delete bitmap.bitmap;
	}

	m_Bitmaps.RemoveAll();

	Shutdown( false );
}

vgui2::HScheme CSchemeManager::LoadSchemeFromFile( const char *fileName, const char *tag )
{
	auto hScheme = FindLoadedScheme( fileName );

	if( NULL_HANDLE != hScheme )
		return hScheme;

	auto pData = new KeyValues( "Scheme" );

	if( !pData->LoadFromFile( vgui2::filesystem(), fileName, "SKIN" ) &&
		!pData->LoadFromFile( vgui2::filesystem(), fileName ) )
	{
		pData->deleteThis();
		return NULL_HANDLE;
	}

	auto src = new CScheme();

	src->LoadFromFile( fileName, tag, pData );

	return _scheme.AddToTail( src );
}

void CSchemeManager::ReloadSchemes()
{
	for( int i = 1; i < _scheme.Count(); ++i )
	{
		auto& scheme = _scheme[ i ];

		LoadSchemeFromFile( scheme->fileName, scheme->tag );
	}
}

vgui2::HScheme CSchemeManager::GetDefaultScheme()
{
	return DEFAULT_SCHEME;
}

vgui2::HScheme CSchemeManager::GetScheme( const char *tag )
{
	for( int i = 1; i < _scheme.Count(); ++i )
	{
		if( !stricmp( tag, _scheme[ i ]->tag ) )
			return static_cast<vgui2::HScheme>( i );
	}

	return DEFAULT_SCHEME;
}

vgui2::IImage *CSchemeManager::GetImage( const char *imageName, bool hardwareFiltered )
{
	CachedBitmapHandle_t searchBitmap;
	searchBitmap.bitmap = nullptr;

	s_pszSearchString = imageName;

	auto index = m_Bitmaps.Find( searchBitmap );

	if( index != m_Bitmaps.InvalidIndex() )
	{
		return m_Bitmaps[ index ].bitmap;
	}

	CachedBitmapHandle_t bitmap;

	bitmap.bitmap = new vgui2::Bitmap( imageName, hardwareFiltered );

	return m_Bitmaps[ m_Bitmaps.Insert( bitmap ) ].bitmap;
}

vgui2::HTexture CSchemeManager::GetImageID( const char *imageName, bool hardwareFiltered )
{
	return static_cast<vgui2::Bitmap*>( GetImage( imageName, hardwareFiltered ) )->GetID();
}

#ifdef _XBOX
void CSchemeManager::DeleteImage( const char *pImageName )
{
}
#endif

vgui2::IScheme *CSchemeManager::GetIScheme( vgui2::HScheme scheme )
{
	return _scheme[ scheme ];
}

void CSchemeManager::Shutdown( bool full )
{
	for( int i = 1; i < _scheme.Count(); ++i )
	{
		_scheme[ i ]->Shutdown( full );
	}
}

int CSchemeManager::GetProportionalScaledValue( int normalizedValue )
{
	int wide, tall;
	vgui2::surface()->GetScreenSize( wide, tall );
	int proW, proH;
	vgui2::surface()->GetProportionalBase( proW, proH );

	return static_cast<int>(
		floor(
			static_cast<double>( normalizedValue ) *
			( static_cast<double>( tall ) / proH )
		)
	);
}

int CSchemeManager::GetProportionalNormalizedValue( int scaledValue )
{
	int wide, tall;
	vgui2::surface()->GetScreenSize( wide, tall );
	int proW, proH;
	vgui2::surface()->GetProportionalBase( proW, proH );

	return static_cast<int>(
		floor(
			static_cast<double>( scaledValue ) / tall / proH
		)
	);
}

vgui2::HScheme CSchemeManager::FindLoadedScheme( const char* fileName )
{
	for( int i = 1; i < _scheme.Count(); ++i )
	{
		if( !stricmp( _scheme[ i ]->fileName, fileName ) )
			return static_cast<vgui2::HScheme>( i );
	}

	return NULL_HANDLE;
}
