#ifndef ENGINE_CL_DEMO_H
#define ENGINE_CL_DEMO_H

#include "cdll_int.h"
#include "demo_api.h"
#include "client.h"
#include "common.h"
#include <pm_movevars.h>

// NOTE:  Change this any time the file version of the demo changes
#define DEMO_PROTOCOL 5

struct demoentry_t
{
    int nEntryType;                // DEMO_STARTUP or DEMO_NORMAL
    char szDescription[64];
    int nFlags;
    int nCDTrack;
    float fTrackTime;            // Time of track
    int nFrames;                // # of frames in track
    int nOffset;                // File offset of track data
    int nFileLength;            // Length of track
};

struct demodirectory_t
{
    // Number of tracks
    int nEntries;
    // Track entry info
    demoentry_t* p_rgEntries;
};

struct demoheader_t
{
    char szFileStamp[6];        // Should be HLDEMO
    int nDemoProtocol;            // Should be DEMO_PROTOCOL
    int nNetProtocolVersion;    // Should be PROTOCOL_VERSION
    char szMapName[MAX_PATH];    // Name of map
    char szDllDir[MAX_PATH];    // Name of game directory (com_gamedir)
    CRC32_t mapCRC;
    int nDirectoryOffset;        // Offset of Entry Directory.
};

struct demo_info_t
{
    float timestamp;
    ref_params_t rp;
    usercmd_t cmd;
    movevars_t movevars;
    vec3_t view;
    int viewmodel;
};

extern client_textmessage_t tm_demomessage;

extern client_textmessage_t tm_demomessage;

int CL_DemoAPIRecording();

int CL_DemoAPIPlayback();

int CL_DemoAPITimedemo();

void CL_WriteClientDLLMessage( int size, byte* buf );

void CL_WriteDLLUpdate( client_data_t* cdat );

void CL_DemoAnim( int anim, int body );

void CL_DemoEvent( int flags, int idx, float delay, event_args_t* pargs );

#endif //ENGINE_CL_DEMO_H
