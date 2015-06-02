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

#include "libXBMC_addon.h"
#include "StSoundLibrary.h"
#include "YmMusic.h"

extern "C" {
#include <stdio.h>
#include <stdint.h>

#include "kodi_audiodec_dll.h"

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

#define SET_IF(ptr, value) \
{ \
  if ((ptr)) \
   *(ptr) = (value); \
}

void* Init(const char* strFile, unsigned int filecache, int* channels,
           int* samplerate, int* bitspersample, int64_t* totaltime,
           int* bitrate, AEDataFormat* format, const AEChannel** channelinfo)
{
  if (!strFile)
    return NULL;

  YMMUSIC *pMusic = ymMusicCreate();
  if (!pMusic)
    return NULL;

  void* file = XBMC->OpenFile(strFile,0);
  if (!file)
    return NULL;
  int len = XBMC->GetFileLength(file);
  char *data = new char[len];
  if (!data)
  {
    XBMC->CloseFile(file);
    ymMusicDestroy(pMusic);
    return NULL;
  }
  XBMC->ReadFile(file, data, len);
  XBMC->CloseFile(file);

  int res = ymMusicLoadMemory(pMusic, data, len);
  delete[] data;
  if (res)
  {
    ymMusicSetLoopMode(pMusic, YMFALSE);
    ymMusicPlay(pMusic);
    ymMusicInfo_t info;
    ymMusicGetInfo(pMusic, &info);

    SET_IF(channels, 1)
    SET_IF(samplerate, 44100)
    SET_IF(bitspersample, 16)
    SET_IF(totaltime, info.musicTimeInSec*1000)
    SET_IF(format, AE_FMT_S16NE)
    *format = AE_FMT_S16NE;
    static enum AEChannel map[3] = {
	    AE_CH_FL, AE_CH_FR, AE_CH_NULL
    };
    SET_IF(channelinfo, map)
    SET_IF(bitrate, 0)

    return pMusic;
  }

  ymMusicDestroy(pMusic);

  return 0;
}

int ReadPCM(void* context, uint8_t* pBuffer, int size, int *actualsize)
{
  if (!context || !pBuffer || !actualsize)
    return 1;

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
  if (!context)
    return 0;

  if (ymMusicIsSeekable((YMMUSIC*)context))
  {
    ymMusicSeek((YMMUSIC*)context, time);
    return time;
  }

  return 0;
}

bool DeInit(void* context)
{
  if (!context)
    return true;

  ymMusicStop((YMMUSIC*)context);
  ymMusicDestroy((YMMUSIC*)context);

  return true;
}

bool ReadTag(const char* strFile, char* title, char* artist, int* length)
{
  if (!strFile)
    return false;

  void* file = XBMC->OpenFile(strFile,0);
  if (!file)
    return false;

  int len = XBMC->GetFileLength(file);
  char *data = new char[len];
  YMMUSIC *pMusic = (YMMUSIC*)new CYmMusic;

  if (!data || !pMusic)
  {
    XBMC->CloseFile(file);
    return false;
  }

  XBMC->ReadFile(file, data, len);
  XBMC->CloseFile(file);

  SET_IF(length, 0)
  if (ymMusicLoadMemory(pMusic, data, len))
  {
    ymMusicInfo_t info;
    ymMusicGetInfo(pMusic, &info);
    if (title)
      strcpy(title, info.pSongName);
    if (artist)
      strcpy(artist, info.pSongAuthor);
    SET_IF(length, info.musicTimeInSec);
  }
  delete[] data;

  ymMusicDestroy(pMusic);

  return length?(*length != 0):false;
}

int TrackCount(const char* strFile)
{
  return 1;
}
}
