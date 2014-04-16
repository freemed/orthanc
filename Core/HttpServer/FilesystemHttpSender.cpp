/**
 * Orthanc - A Lightweight, RESTful DICOM Store
 * Copyright (C) 2012-2014 Medical Physics Department, CHU of Liege,
 * Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * In addition, as a special exception, the copyright holders of this
 * program give permission to link the code of its release with the
 * OpenSSL project's "OpenSSL" library (or with modified versions of it
 * that use the same license as the "OpenSSL" library), and distribute
 * the linked executables. You must obey the GNU General Public License
 * in all respects for all of the code used other than "OpenSSL". If you
 * modify file(s) with this exception, you may extend this exception to
 * your version of the file(s), but you are not obligated to do so. If
 * you do not wish to do so, delete this exception statement from your
 * version. If you delete this exception statement from all source files
 * in the program, then also delete it here.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "FilesystemHttpSender.h"

#include "../Toolbox.h"

#include <stdio.h>

namespace Orthanc
{
  void FilesystemHttpSender::Setup()
  {
    //SetDownloadFilename(path_.filename().string());

#if BOOST_HAS_FILESYSTEM_V3 == 1
    SetContentType(Toolbox::AutodetectMimeType(path_.filename().string()));
#else
    SetContentType(Toolbox::AutodetectMimeType(path_.filename()));
#endif
  }

  uint64_t FilesystemHttpSender::GetFileSize()
  {
    return Toolbox::GetFileSize(path_.string());
  }

  bool FilesystemHttpSender::SendData(HttpOutput& output)
  {
    FILE* fp = fopen(path_.string().c_str(), "rb");
    if (!fp)
    {
      return false;
    }

    std::vector<uint8_t> buffer(1024 * 1024);  // Chunks of 1MB

    for (;;)
    {
      size_t nbytes = fread(&buffer[0], 1, buffer.size(), fp);
      if (nbytes == 0)
      {
        break;
      }
      else
      {
        output.Send(&buffer[0], nbytes);
      }
    }

    fclose(fp);

    return true;
  }

  FilesystemHttpSender::FilesystemHttpSender(const char* path)
  {
    path_ = std::string(path);
    Setup();
  }

  FilesystemHttpSender::FilesystemHttpSender(const boost::filesystem::path& path)
  {
    path_ = path;
    Setup();
  }

  FilesystemHttpSender::FilesystemHttpSender(const FileStorage& storage,
                                             const std::string& uuid)
  {
    path_ = storage.GetPath(uuid).string();
    Setup();
  }
}
