#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "public/FileSystem.h"
#include "filesystem.h"

FileHandle_t FS_Open( const char *pFileName, const char *pOptions )
{
	return g_pFileSystem->Open( pFileName, pOptions );
}

FileHandle_t FS_OpenPathID( const char *pFileName, const char *pOptions, const char *pathID )
{
	return g_pFileSystem->Open( pFileName, pOptions, pathID );
}

void FS_Close( FileHandle_t file )
{
	g_pFileSystem->Close( file );
}

bool FS_EndOfFile( FileHandle_t file )
{
	return g_pFileSystem->EndOfFile( file );
}

char* FS_ReadLine( char *pOutput, int maxChars, FileHandle_t file )
{
	return g_pFileSystem->ReadLine( pOutput, maxChars, file );
}

void FS_AddSearchPath( const char *pPath, const char *pathID )
{
	g_pFileSystem->AddSearchPath( pPath, pathID );
}

void FS_CreateDirHierarchy( const char *path, const char *pathID )
{
	g_pFileSystem->CreateDirHierarchy( path, pathID );
}

int FS_FPrintf( FileHandle_t file, const char* pFormat, ... )
{
	char data[ 8192 ];
	va_list va;

	va_start( va, pFormat );
	vsnprintf( data, sizeof( data ), pFormat, va );
	va_end( va );

	return g_pFileSystem->FPrintf( file, "%s", data );
}

bool FS_FileExists( const char *pFileName )
{
	return g_pFileSystem->FileExists( pFileName );
}

int FS_FileSize( const char *pFileName )
{
	return g_pFileSystem->Size( pFileName );
}

void FS_FileTimeToString( char *pStrip, int maxCharsIncludingTerminator, int32 fileTime )
{
	g_pFileSystem->FileTimeToString( pStrip, maxCharsIncludingTerminator, fileTime );
}

void __cdecl FS_FindClose( FileFindHandle_t handle )
{
	g_pFileSystem->FindClose( handle );
}

const char* FS_FindFirst( const char *pWildCard, FileFindHandle_t *pHandle, const char *pathID )
{
	return g_pFileSystem->FindFirst( pWildCard, pHandle, pathID );
}

bool FS_FindIsDirectory( FileFindHandle_t handle )
{
	return g_pFileSystem->FindIsDirectory( handle );
}

const char* FS_FindNext( FileFindHandle_t handle )
{
	return g_pFileSystem->FindNext( handle );
}

void FS_Flush( FileHandle_t file )
{
	g_pFileSystem->Flush( file );
}

bool FS_FullPathToRelativePath( const char *pFullpath, char *pRelative )
{
	return g_pFileSystem->FullPathToRelativePath(
		pFullpath,
		pRelative );
}

char FS_GetCharacter( FileHandle_t f )
{
	char retval;

	g_pFileSystem->Read( &retval, 1, f );
	return retval;
}

int FS_GetCurrentDirectory( char *pDirectory, int maxlen )
{
	return g_pFileSystem->GetCurrentDirectory( pDirectory, maxlen );
}

int32 FS_GetFileTime( const char *pFileName )
{
	return g_pFileSystem->GetFileTime( pFileName );
}

void FS_GetInterfaceVersion( char *p, int maxlen )
{
	g_pFileSystem->GetInterfaceVersion( p, maxlen );
}

void FS_GetLocalCopy( const char *pFileName )
{
	g_pFileSystem->GetLocalCopy( pFileName );
}

const char* FS_GetLocalPath( const char *pFileName, char *pLocalPath, int localPathBufferSize )
{
	return g_pFileSystem->GetLocalPath( pFileName, pLocalPath, localPathBufferSize );
}

void* FS_GetReadBuffer( FileHandle_t file, int *outBufSize )
{
	return g_pFileSystem->GetReadBuffer( file, outBufSize, true );
}

int FS_IsDirectory( const char *pFileName )
{
	return g_pFileSystem->IsDirectory( pFileName );
}

int FS_IsOk( FileHandle_t file )
{
	return g_pFileSystem->IsOk( file );
}

CSysModule* FS_LoadLibrary( const char *dllName )
{
	if( dllName )
	{
		g_pFileSystem->GetLocalCopy( dllName );
		return Sys_LoadModule( dllName );
	}

	return nullptr;
}

void FS_LogLevelLoadFinished( const char *name )
{
	g_pFileSystem->LogLevelLoadFinished( name );
}

void FS_LogLevelLoadStarted( const char *name )
{
	g_pFileSystem->LogLevelLoadStarted( name );
}

char* FS_ParseFile( char *pFileBytes, char *pToken, int *pWasQuoted )
{
	bool wasquoted;

	char* result = g_pFileSystem->ParseFile( pFileBytes, pToken, &wasquoted );
	
	if( pWasQuoted )
		*pWasQuoted = wasquoted;

	return result;
}

void FS_PrintOpenedFiles()
{
	g_pFileSystem->PrintOpenedFiles();
}

int FS_Read( void *pOutput, int size, FileHandle_t hFile )
{
	return g_pFileSystem->Read( pOutput, size, hFile );
}

void FS_ReleaseReadBuffer( FileHandle_t file, void *buffer )
{
	g_pFileSystem->ReleaseReadBuffer( file, buffer );
}

void FS_RemoveAllSearchPaths()
{
	g_pFileSystem->RemoveAllSearchPaths();
}

void FS_RemoveFile( const char *pRelativePath, const char *pathID )
{
	g_pFileSystem->RemoveFile( pRelativePath, pathID );
}

bool FS_RemoveSearchPath( const char *pPath )
{
	return g_pFileSystem->RemoveSearchPath( pPath );
}

void FS_Rename( const char *originalName, const char *newName )
{
	char localPath[ 512 ];

	if( g_pFileSystem->GetLocalPath( originalName, localPath, sizeof( localPath ) ) )
	{
		char newPath[ 512 ];

		strcpy( newPath, localPath );
		char* pszPathEnd = strstr( newPath, originalName );
		if( pszPathEnd )
		{
			*pszPathEnd = '\0';
			strcat( newPath, newName );
			rename( localPath, newPath );
		}
	}
}

void FS_Seek( FileHandle_t file, int pos, FileSystemSeek_t seekType )
{
	g_pFileSystem->Seek( file, pos, seekType );
}

int FS_SetVBuf( FileHandle_t stream, char *buffer, int mode, size_t size )
{
	return g_pFileSystem->SetVBuf( stream, buffer, mode, size );
}

void FS_SetWarningFunc( FileSystemWarningFunc pfnWarning )
{
	g_pFileSystem->SetWarningFunc( pfnWarning );
}

void FS_SetWarningLevel( FileWarningLevel_t level )
{
	g_pFileSystem->SetWarningLevel( level );
}

unsigned int FS_Size( FileHandle_t file )
{
	return g_pFileSystem->Size( file );
}

unsigned int FS_Tell( FileHandle_t file )
{
	return g_pFileSystem->Tell( file );
}

void FS_Unlink( const char *filename )
{
	char localPath[ 512 ];

	g_pFileSystem->GetLocalPath( filename, localPath, sizeof( localPath ) );
	_unlink( localPath );
}

int FS_Write( const void *pInput, int size, FileHandle_t hFile )
{
	return g_pFileSystem->Write( pInput, size, hFile );
}
