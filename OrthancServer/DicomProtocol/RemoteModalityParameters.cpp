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


#include "RemoteModalityParameters.h"

#include "../../Core/OrthancException.h"

#include <boost/lexical_cast.hpp>
#include <stdexcept>

namespace Orthanc
{
  RemoteModalityParameters::RemoteModalityParameters()
  {
    name_ = "";
    aet_ = "ORTHANC";
    host_ = "localhost";
    port_ = 104;
    manufacturer_ = ModalityManufacturer_Generic;
  }

  void RemoteModalityParameters::SetPort(int port)
  {
    if (port <= 0 || port >= 65535)
    {
      throw OrthancException(ErrorCode_ParameterOutOfRange);
    }

    port_ = port;
  }

  void RemoteModalityParameters::FromJson(const Json::Value& modality)
  {
    if (!modality.isArray() ||
        (modality.size() != 3 && modality.size() != 4))
    {
      throw OrthancException(ErrorCode_BadFileFormat);
    }

    SetApplicationEntityTitle(modality.get(0u, "").asString());
    SetHost(modality.get(1u, "").asString());

    const Json::Value& portValue = modality.get(2u, "");
    try
    {
      SetPort(portValue.asInt());
    }
    catch (std::runtime_error /* error inside JsonCpp */)
    {
      try
      {
        SetPort(boost::lexical_cast<int>(portValue.asString()));
      }
      catch (boost::bad_lexical_cast)
      {
        throw OrthancException(ErrorCode_BadFileFormat);
      }
    }

    if (modality.size() == 4)
    {
      SetManufacturer(modality.get(3u, "").asString());
    }
    else
    {
      SetManufacturer(ModalityManufacturer_Generic);
    }
  }

  void RemoteModalityParameters::ToJson(Json::Value& value) const
  {
    value = Json::arrayValue;
    value.append(GetApplicationEntityTitle());
    value.append(GetHost());
    value.append(GetPort());
    value.append(EnumerationToString(GetManufacturer()));
  }
}
