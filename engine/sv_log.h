#ifndef ENGINE_SV_LOG_H
#define ENGINE_SV_LOG_H

extern cvar_t sv_log_onefile;
extern cvar_t sv_log_singleplayer;
extern cvar_t sv_logsecret;
extern cvar_t mp_logecho;
extern cvar_t mp_logfile;

void Log_Printf( const char* fmt, ... );

void Log_Close();

void Log_Open();

#endif //ENGINE_SV_LOG_H
