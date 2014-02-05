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


#include "Enumerations.h"

#include "OrthancException.h"
#include "Toolbox.h"

namespace Orthanc
{
  const char* EnumerationToString(HttpMethod method)
  {
    switch (method)
    {
      case HttpMethod_Get:
        return "GET";

      case HttpMethod_Post:
        return "POST";

      case HttpMethod_Delete:
        return "DELETE";

      case HttpMethod_Put:
        return "PUT";

      default:
        return "?";
    }
  }


  const char* EnumerationToString(HttpStatus status)
  {
    switch (status)
    {
    case HttpStatus_100_Continue:
      return "Continue";

    case HttpStatus_101_SwitchingProtocols:
      return "Switching Protocols";

    case HttpStatus_102_Processing:
      return "Processing";

    case HttpStatus_200_Ok:
      return "OK";

    case HttpStatus_201_Created:
      return "Created";

    case HttpStatus_202_Accepted:
      return "Accepted";

    case HttpStatus_203_NonAuthoritativeInformation:
      return "Non-Authoritative Information";

    case HttpStatus_204_NoContent:
      return "No Content";

    case HttpStatus_205_ResetContent:
      return "Reset Content";

    case HttpStatus_206_PartialContent:
      return "Partial Content";

    case HttpStatus_207_MultiStatus:
      return "Multi-Status";

    case HttpStatus_208_AlreadyReported:
      return "Already Reported";

    case HttpStatus_226_IMUsed:
      return "IM Used";

    case HttpStatus_300_MultipleChoices:
      return "Multiple Choices";

    case HttpStatus_301_MovedPermanently:
      return "Moved Permanently";

    case HttpStatus_302_Found:
      return "Found";

    case HttpStatus_303_SeeOther:
      return "See Other";

    case HttpStatus_304_NotModified:
      return "Not Modified";

    case HttpStatus_305_UseProxy:
      return "Use Proxy";

    case HttpStatus_307_TemporaryRedirect:
      return "Temporary Redirect";

    case HttpStatus_400_BadRequest:
      return "Bad Request";

    case HttpStatus_401_Unauthorized:
      return "Unauthorized";

    case HttpStatus_402_PaymentRequired:
      return "Payment Required";

    case HttpStatus_403_Forbidden:
      return "Forbidden";

    case HttpStatus_404_NotFound:
      return "Not Found";

    case HttpStatus_405_MethodNotAllowed:
      return "Method Not Allowed";

    case HttpStatus_406_NotAcceptable:
      return "Not Acceptable";

    case HttpStatus_407_ProxyAuthenticationRequired:
      return "Proxy Authentication Required";

    case HttpStatus_408_RequestTimeout:
      return "Request Timeout";

    case HttpStatus_409_Conflict:
      return "Conflict";

    case HttpStatus_410_Gone:
      return "Gone";

    case HttpStatus_411_LengthRequired:
      return "Length Required";

    case HttpStatus_412_PreconditionFailed:
      return "Precondition Failed";

    case HttpStatus_413_RequestEntityTooLarge:
      return "Request Entity Too Large";

    case HttpStatus_414_RequestUriTooLong:
      return "Request-URI Too Long";

    case HttpStatus_415_UnsupportedMediaType:
      return "Unsupported Media Type";

    case HttpStatus_416_RequestedRangeNotSatisfiable:
      return "Requested Range Not Satisfiable";

    case HttpStatus_417_ExpectationFailed:
      return "Expectation Failed";

    case HttpStatus_422_UnprocessableEntity:
      return "Unprocessable Entity";

    case HttpStatus_423_Locked:
      return "Locked";

    case HttpStatus_424_FailedDependency:
      return "Failed Dependency";

    case HttpStatus_426_UpgradeRequired:
      return "Upgrade Required";

    case HttpStatus_500_InternalServerError:
      return "Internal Server Error";

    case HttpStatus_501_NotImplemented:
      return "Not Implemented";

    case HttpStatus_502_BadGateway:
      return "Bad Gateway";

    case HttpStatus_503_ServiceUnavailable:
      return "Service Unavailable";

    case HttpStatus_504_GatewayTimeout:
      return "Gateway Timeout";

    case HttpStatus_505_HttpVersionNotSupported:
      return "HTTP Version Not Supported";

    case HttpStatus_506_VariantAlsoNegotiates:
      return "Variant Also Negotiates";

    case HttpStatus_507_InsufficientStorage:
      return "Insufficient Storage";

    case HttpStatus_509_BandwidthLimitExceeded:
      return "Bandwidth Limit Exceeded";

    case HttpStatus_510_NotExtended:
      return "Not Extended";

    default:
      throw OrthancException(ErrorCode_ParameterOutOfRange);
    }
  }


  const char* EnumerationToString(ResourceType type)
  {
    switch (type)
    {
      case ResourceType_Patient:
        return "Patient";

      case ResourceType_Study:
        return "Study";

      case ResourceType_Series:
        return "Series";

      case ResourceType_Instance:
        return "Instance";
      
      default:
        throw OrthancException(ErrorCode_ParameterOutOfRange);
    }
  }


  ResourceType StringToResourceType(const char* type)
  {
    std::string s(type);
    Toolbox::ToUpperCase(s);

    if (s == "PATIENT" || s == "PATIENTS")
    {
      return ResourceType_Patient;
    }
    else if (s == "STUDY" || s == "STUDIES")
    {
      return ResourceType_Study;
    }
    else if (s == "SERIES")
    {
      return ResourceType_Series;
    }
    else if (s == "INSTANCE"  || s == "IMAGE" || 
             s == "INSTANCES" || s == "IMAGES")
    {
      return ResourceType_Instance;
    }
    else
    {
      throw OrthancException(ErrorCode_ParameterOutOfRange);
    }
  }
}
