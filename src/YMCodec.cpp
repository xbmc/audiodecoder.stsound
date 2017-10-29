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

#include <kodi/addon-instance/AudioDecoder.h>
#include <kodi/Filesystem.h>
#include "StSoundLibrary.h"
#include "YmMusic.h"
#include <stdio.h>
#include <stdint.h>

class CYMCodec : public kodi::addon::CInstanceAudioDecoder,
                 public kodi::addon::CAddonBase
{
public:
  CYMCodec(KODI_HANDLE instance) :
    CInstanceAudioDecoder(instance) {}

  virtual ~CYMCodec()
  {
    if (m_tune)
    {
      ymMusicStop(m_tune);
      ymMusicDestroy(m_tune);
    }
  }

  virtual bool Init(const std::string& filename, unsigned int filecache,
                    int& channels, int& samplerate,
                    int& bitspersample, int64_t& totaltime,
                    int& bitrate, AEDataFormat& format,
                    std::vector<AEChannel>& channellist) override
  {
    m_tune = ymMusicCreate();
    if (!m_tune)
      return false;

    kodi::vfs::CFile file;
    if (!file.OpenFile(filename))
      return false;

    int len = file.GetLength();
    char *data = new char[len];
    if (!data)
    {
      file.Close();
      ymMusicDestroy(m_tune);
      return false;
    }
    file.Read(data, len);
    file.Close();

    int res = ymMusicLoadMemory(m_tune, data, len);
    delete[] data;
    if (res)
    {
      ymMusicSetLoopMode(m_tune, YMFALSE);
      ymMusicPlay(m_tune);
      ymMusicInfo_t info;
      ymMusicGetInfo(m_tune, &info);

      channels = 1;
      samplerate = 44100;
      bitspersample = 16;
      totaltime =  info.musicTimeInSec*1000;
      format = AE_FMT_S16NE;
      channellist = { AE_CH_FL, AE_CH_FR };
      bitrate = 0;
      return true;
    }

    return false;
  }

  virtual int ReadPCM(uint8_t* buffer, int size, int& actualsize) override
  {
    if (ymMusicCompute(m_tune,(ymsample*)buffer,size/2))
    {
      actualsize = size;
      return 0;
    }
    else
      return 1;
  }

  virtual int64_t Seek(int64_t time) override
  {
    if (ymMusicIsSeekable(m_tune))
    {
      ymMusicSeek(m_tune, time);
      return time;
    }

    return 0;
  }

  virtual bool ReadTag(const std::string& filename, std::string& title,
                       std::string& artist, int& length) override
  {
    YMMUSIC* tune = ymMusicCreate();
    kodi::vfs::CFile file;
    if (!file.OpenFile(filename))
      return false;

    int len = file.GetLength();
    char *data = new char[len];
    if (!data)
    {
      file.Close();
      ymMusicDestroy(tune);
      return false;
    }
    file.Read(data, len);
    file.Close();

    length = 0;
    if (ymMusicLoadMemory(tune, data, len))
    {
      ymMusicInfo_t info;
      ymMusicGetInfo(tune, &info);
      title = info.pSongName;
      artist = info.pSongAuthor;
      length = info.musicTimeInSec;
    }
    delete[] data;

    ymMusicDestroy(tune);

    return length != 0;
  }

private:
  YMMUSIC* m_tune = nullptr;
};


class ATTRIBUTE_HIDDEN CMyAddon : public kodi::addon::CAddonBase
{
public:
  CMyAddon() { }
  virtual ADDON_STATUS CreateInstance(int instanceType, std::string instanceID, KODI_HANDLE instance, KODI_HANDLE& addonInstance) override
  {
    addonInstance = new CYMCodec(instance);
    return ADDON_STATUS_OK;
  }
  virtual ~CMyAddon()
  {
  }
};


ADDONCREATOR(CMyAddon);
