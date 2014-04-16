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


#pragma once

#include "Enumerations.h"

#include <stdint.h>
#include <vector>
#include <string>

namespace Orthanc
{
  typedef std::vector<std::string> UriComponents;

  class NullType
  {
  };

  namespace Toolbox
  {
    void ServerBarrier();

    void ToUpperCase(std::string& s);  // Inplace version

    void ToLowerCase(std::string& s);  // Inplace version

    void ToUpperCase(std::string& result,
                     const std::string& source);

    void ToLowerCase(std::string& result,
                     const std::string& source);

    void ReadFile(std::string& content,
                  const std::string& path);

    void WriteFile(const std::string& content,
                   const std::string& path);

    void USleep(uint64_t microSeconds);

    void RemoveFile(const std::string& path);

    void SplitUriComponents(UriComponents& components,
                            const std::string& uri);
  
    bool IsChildUri(const UriComponents& baseUri,
                    const UriComponents& testedUri);

    std::string AutodetectMimeType(const std::string& path);

    std::string FlattenUri(const UriComponents& components,
                           size_t fromLevel = 0);

    uint64_t GetFileSize(const std::string& path);

    void ComputeMD5(std::string& result,
                    const std::string& data);

    void ComputeMD5(std::string& result,
                    const void* data,
                    size_t length);

    void ComputeSHA1(std::string& result,
                     const std::string& data);

    bool IsSHA1(const std::string& str);

    std::string DecodeBase64(const std::string& data);

    std::string EncodeBase64(const std::string& data);

    std::string GetPathToExecutable();

    std::string GetDirectoryOfExecutable();

    std::string ConvertToUtf8(const std::string& source,
                              const char* fromEncoding);

    std::string ConvertToAscii(const std::string& source);

    std::string StripSpaces(const std::string& source);

    std::string GetNowIsoString();

    // In-place percent-decoding for URL
    void UrlDecode(std::string& s);

    Endianness DetectEndianness();

    std::string WildcardToRegularExpression(const std::string& s);

    void TokenizeString(std::vector<std::string>& result,
                        const std::string& source,
                        char separator);
  }
}
