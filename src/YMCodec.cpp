/*
 *      Copyright (C) 2008-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "xbmc/libXBMC_addon.h"
#include "StSoundLibrary.h"
#include "YmMusic.h"

extern "C" {
#include <stdio.h>
#include <stdint.h>

#include "xbmc/xbmc_audiodec_dll.h"

ADDON::CHelper_libXBMC_addon *XBMC           = NULL;

ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
  if (!XBMC)
    XBMC = new ADDON::CHelper_libXBMC_addon;

  if (!XBMC->RegisterMe(hdl))
  {
    delete XBMC, XBMC=NULL;
    return ADDON_STATUS_PERMANENT_FAILURE;
  }

  return ADDON_STATUS_OK;
}

//-- Stop ---------------------------------------------------------------------
// This dll must cease all runtime activities
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void ADDON_Stop()
{
}

//-- Destroy ------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void ADDON_Destroy()
{
  XBMC=NULL;
}

//-- HasSettings --------------------------------------------------------------
// Returns true if this add-on use settings
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
bool ADDON_HasSettings()
{
  return false;
}

//-- GetStatus ---------------------------------------------------------------
// Returns the current Status of this visualisation
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
ADDON_STATUS ADDON_GetStatus()
{
  return ADDON_STATUS_OK;
}

//-- GetSettings --------------------------------------------------------------
// Return the settings for XBMC to display
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
  return 0;
}

//-- FreeSettings --------------------------------------------------------------
// Free the settings struct passed from XBMC
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------

void ADDON_FreeSettings()
{
}

//-- SetSetting ---------------------------------------------------------------
// Set a specific Setting value (called from XBMC)
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
ADDON_STATUS ADDON_SetSetting(const char *strSetting, const void* value)
{
  return ADDON_STATUS_OK;
}

//-- Announce -----------------------------------------------------------------
// Receive announcements from XBMC
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}

void* Init(const char* strFile, unsigned int filecache, int* channels,
           int* samplerate, int* bitspersample, int64_t* totaltime,
           int* bitrate, AEDataFormat* format, const AEChannel** channelinfo)
{
  YMMUSIC *pMusic = (YMMUSIC*)new CYmMusic;

  void* file = XBMC->OpenFile(strFile,0);
  if (!file)
    return NULL;
  int len = XBMC->GetFileLength(file);
  char *data = new char[len];
  XBMC->ReadFile(file, data, len);
  XBMC->CloseFile(file);

  if (ymMusicLoadMemory(pMusic, data, len))
  {
    ymMusicSetLoopMode(pMusic, YMFALSE);
    ymMusicPlay(pMusic);
    ymMusicInfo_t info;
    ymMusicGetInfo(pMusic, &info);

    *channels = 1;
    *samplerate = 44100;
    *bitspersample = 16;
    *totaltime = info.musicTimeInSec*1000;
    *format = AE_FMT_S16NE;
    *channelinfo = NULL;
    *bitrate = 0;
    return pMusic;
  }

  ymMusicDestroy(pMusic);

  return 0;
}

int ReadPCM(void* context, uint8_t* pBuffer, int size, int *actualsize)
{
  if (ymMusicCompute((YMMUSIC*)context,(ymsample*)pBuffer,size/2))
  {
    *actualsize = size;
    return 0;
  }
  else
    return 1;
}

int64_t Seek(void* context, int64_t time)
{
  if (ymMusicIsSeekable((YMMUSIC*)context))
  {
    ymMusicSeek((YMMUSIC*)context, time);
    return time;
  }

  return 0;
}

bool DeInit(void* context)
{
  ymMusicStop((YMMUSIC*)context);
  ymMusicDestroy((YMMUSIC*)context);
}

bool ReadTag(const char* strFile, char* title, char* artist, int* length)
{
  YMMUSIC *pMusic = (YMMUSIC*)new CYmMusic;

  void* file = XBMC->OpenFile(strFile,0);
  if (!file)
    return NULL;
  int len = XBMC->GetFileLength(file);
  char *data = new char[len];
  XBMC->ReadFile(file, data, len);
  XBMC->CloseFile(file);

  *length = 0;
  if (ymMusicLoadMemory(pMusic, data, len))
  {
    ymMusicInfo_t info;
    ymMusicGetInfo(pMusic, &info);
    strcpy(title, info.pSongName);
    strcpy(artist, info.pSongAuthor);
    *length = info.musicTimeInSec;
  }
  delete[] data;

  ymMusicDestroy(pMusic);
  return *length != 0;
}

int TrackCount(const char* strFile)
{
  return 1;
}
}
