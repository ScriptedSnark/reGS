//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================
#ifndef STEAM_STEAM2COMPAT_H
#define STEAM_STEAM2COMPAT_H

#include "steam_api.h"

/**
*	@file
*
*	Declares Steam 2 types and constants for backwards compatibility
*/

struct TSteamProgress
{
	int bValid;
	unsigned int uPercentDone;
	char szProgress[ 255 ];
};

enum ESteamError
{
	eSteamErrorNone = 0,
	eSteamErrorUnknown,
	eSteamErrorLibraryNotInitialized,
	eSteamErrorLibraryAlreadyInitialized,
	eSteamErrorConfig,
	eSteamErrorContentServerConnect,
	eSteamErrorBadHandle,
	eSteamErrorHandlesExhausted,
	eSteamErrorBadArg,
	eSteamErrorNotFound,
	eSteamErrorRead,
	eSteamErrorEOF,
	eSteamErrorSeek,
	eSteamErrorCannotWriteNonUserConfigFile,
	eSteamErrorCacheOpen,
	eSteamErrorCacheRead,
	eSteamErrorCacheCorrupted,
	eSteamErrorCacheWrite,
	eSteamErrorCacheSession,
	eSteamErrorCacheInternal,
	eSteamErrorCacheBadApp,
	eSteamErrorCacheVersion,
	eSteamErrorCacheBadFingerPrint,
	eSteamErrorNotFinishedProcessing,
	eSteamErrorNothingToDo,
	eSteamErrorCorruptEncryptedUserIDTicket,
	eSteamErrorSocketLibraryNotInitialized,
	eSteamErrorFailedToConnectToUserIDTicketValidationServer,
	eSteamErrorBadProtocolVersion,
	eSteamErrorReplayedUserIDTicketFromClient,
	eSteamErrorReceiveResultBufferTooSmall,
	eSteamErrorSendFailed,
	eSteamErrorReceiveFailed,
	eSteamErrorReplayedReplyFromUserIDTicketValidationServer,
	eSteamErrorBadSignatureFromUserIDTicketValidationServer,
	eSteamErrorValidationStalledSoAborted,
	eSteamErrorInvalidUserIDTicket,
	eSteamErrorClientLoginRateTooHigh,
	eSteamErrorClientWasNeverValidated,
	eSteamErrorInternalSendBufferTooSmall,
	eSteamErrorInternalReceiveBufferTooSmall,
	eSteamErrorUserTicketExpired,
	eSteamErrorCDKeyAlreadyInUseOnAnotherClient,
	eSteamErrorNotLoggedIn,
	eSteamErrorAlreadyExists,
	eSteamErrorAlreadySubscribed,
	eSteamErrorNotSubscribed,
	eSteamErrorAccessDenied,
	eSteamErrorFailedToCreateCacheFile,
	eSteamErrorCallStalledSoAborted,
	eSteamErrorEngineNotRunning,
	eSteamErrorEngineConnectionLost,
	eSteamErrorLoginFailed,
	eSteamErrorAccountPending,
	eSteamErrorCacheWasMissingRetry,
	eSteamErrorLocalTimeIncorrect,
	eSteamErrorCacheNeedsDecryption,
	eSteamErrorAccountDisabled,
	eSteamErrorCacheNeedsRepair,
	eSteamErrorRebootRequired,
	eSteamErrorNetwork,
	eSteamErrorOffline,
};

enum EDetailedPlatformErrorType
{
	eNoDetailedErrorAvailable = 0,
	eStandardCerrno,
	eWin32LastError,
	eWinSockLastError,
	eDetailedPlatformErrorCount,
};

struct TSteamError
{
	ESteamError eSteamError;
	EDetailedPlatformErrorType eDetailedErrorType;
	int nDetailedErrorCode;
	char szDesc[ 255 ];
};

typedef unsigned int SteamHandle_t;

#endif //STEAM_STEAM2COMPAT_H
