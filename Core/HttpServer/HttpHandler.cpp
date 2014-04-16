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


#include "HttpHandler.h"

#include <string.h>

namespace Orthanc
{
  static void SplitGETNameValue(HttpHandler::Arguments& result,
                                const char* start,
                                const char* end)
  {
    std::string name, value;
    
    const char* equal = strchr(start, '=');
    if (equal == NULL || equal >= end)
    {
      name = std::string(start, end - start);
      //value = "";
    }
    else
    {
      name = std::string(start, equal - start);
      value = std::string(equal + 1, end);
    }

    Toolbox::UrlDecode(name);
    Toolbox::UrlDecode(value);

    result.insert(std::make_pair(name, value));
  }


  void HttpHandler::ParseGetQuery(HttpHandler::Arguments& result, const char* query)
  {
    const char* pos = query;

    while (pos != NULL)
    {
      const char* ampersand = strchr(pos, '&');
      if (ampersand)
      {
        SplitGETNameValue(result, pos, ampersand);
        pos = ampersand + 1;
      }
      else
      {
        // No more ampersand, this is the last argument
        SplitGETNameValue(result, pos, pos + strlen(pos));
        pos = NULL;
      }
    }
  }



  std::string HttpHandler::GetArgument(const Arguments& getArguments,
                                       const std::string& name,
                                       const std::string& defaultValue)
  {
    Arguments::const_iterator it = getArguments.find(name);
    if (it == getArguments.end())
    {
      return defaultValue;
    }
    else
    {
      return it->second;
    }
  }



  void HttpHandler::ParseCookies(HttpHandler::Arguments& result, 
                                 const HttpHandler::Arguments& httpHeaders)
  {
    result.clear();

    HttpHandler::Arguments::const_iterator it = httpHeaders.find("cookie");
    if (it != httpHeaders.end())
    {
      const std::string& cookies = it->second;

      size_t pos = 0;
      while (pos != std::string::npos)
      {
        size_t nextSemicolon = cookies.find(";", pos);
        std::string cookie;

        if (nextSemicolon == std::string::npos)
        {
          cookie = cookies.substr(pos);
          pos = std::string::npos;
        }
        else
        {
          cookie = cookies.substr(pos, nextSemicolon - pos);
          pos = nextSemicolon + 1;
        }

        size_t equal = cookie.find("=");
        if (equal != std::string::npos)
        {
          std::string name = Toolbox::StripSpaces(cookie.substr(0, equal));
          std::string value = Toolbox::StripSpaces(cookie.substr(equal + 1));
          result[name] = value;
        }
      }
    }
  }
}
