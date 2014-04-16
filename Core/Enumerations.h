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

#include <laaw/laaw.h>

namespace Orthanc
{
  enum Endianness
  {
    Endianness_Unknown,
    Endianness_Big,
    Endianness_Little
  };

  enum ErrorCode
  {
    // Generic error codes
    ErrorCode_Success,
    ErrorCode_Custom,
    ErrorCode_InternalError,
    ErrorCode_NotImplemented,
    ErrorCode_ParameterOutOfRange,
    ErrorCode_NotEnoughMemory,
    ErrorCode_BadParameterType,
    ErrorCode_BadSequenceOfCalls,
    ErrorCode_InexistentItem,
    ErrorCode_BadRequest,
    ErrorCode_NetworkProtocol,

    // Specific error codes
    ErrorCode_UriSyntax,
    ErrorCode_InexistentFile,
    ErrorCode_CannotWriteFile,
    ErrorCode_BadFileFormat,
    ErrorCode_Timeout,
    ErrorCode_UnknownResource,
    ErrorCode_IncompatibleDatabaseVersion,
    ErrorCode_FullStorage,
    ErrorCode_CorruptedFile,
    ErrorCode_InexistentTag
  };

  /**
   * {summary}{The memory layout of the pixels (resp. voxels) of a 2D (resp. 3D) image.}
   **/
  enum LAAW_API PixelFormat
  {
    /**
     * {summary}{Color image in RGB24 format.}
     * {description}{This format describes a color image. The pixels are stored in 3
     * consecutive bytes. The memory layout is RGB.
     **/
    PixelFormat_RGB24,

    /**
     * {summary}{Graylevel 8bpp image.}
     * {description}{The image is graylevel. Each pixel is unsigned and stored in one byte.}
     **/
    PixelFormat_Grayscale8,
      
    /**
     * {summary}{Graylevel, unsigned 16bpp image.}
     * {description}{The image is graylevel. Each pixel is unsigned and stored in two bytes.}
     **/
    PixelFormat_Grayscale16,
      
    /**
     * {summary}{Graylevel, signed 16bpp image.}
     * {description}{The image is graylevel. Each pixel is signed and stored in two bytes.}
     **/
    PixelFormat_SignedGrayscale16
  };


  /**
   * {summary}{The extraction mode specifies the way the values of the pixels are scaled when downloading a 2D image.}
   **/
  enum LAAW_API ImageExtractionMode
  {
    /**
     * {summary}{Rescaled to 8bpp.}
     * {description}{The minimum value of the image is set to 0, and its maximum value is set to 255.}
     **/
    ImageExtractionMode_Preview,

    /**
     * {summary}{Truncation to the [0, 255] range.}
     **/
    ImageExtractionMode_UInt8,

    /**
     * {summary}{Truncation to the [0, 65535] range.}
     **/
    ImageExtractionMode_UInt16,

    /**
     * {summary}{Truncation to the [-32768, 32767] range.}
     **/
    ImageExtractionMode_Int16
  };


  /**
   * Most common, non-joke and non-experimental HTTP status codes
   * http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
   **/
  enum HttpStatus
  {
    HttpStatus_None = -1,

    // 1xx Informational
    HttpStatus_100_Continue = 100,
    HttpStatus_101_SwitchingProtocols = 101,
    HttpStatus_102_Processing = 102,

    // 2xx Success
    HttpStatus_200_Ok = 200,
    HttpStatus_201_Created = 201,
    HttpStatus_202_Accepted = 202,
    HttpStatus_203_NonAuthoritativeInformation = 203,
    HttpStatus_204_NoContent = 204,
    HttpStatus_205_ResetContent = 205,
    HttpStatus_206_PartialContent = 206,
    HttpStatus_207_MultiStatus = 207,
    HttpStatus_208_AlreadyReported = 208,
    HttpStatus_226_IMUsed = 226,

    // 3xx Redirection
    HttpStatus_300_MultipleChoices = 300,
    HttpStatus_301_MovedPermanently = 301,
    HttpStatus_302_Found = 302,
    HttpStatus_303_SeeOther = 303,
    HttpStatus_304_NotModified = 304,
    HttpStatus_305_UseProxy = 305,
    HttpStatus_307_TemporaryRedirect = 307,

    // 4xx Client Error
    HttpStatus_400_BadRequest = 400,
    HttpStatus_401_Unauthorized = 401,
    HttpStatus_402_PaymentRequired = 402,
    HttpStatus_403_Forbidden = 403,
    HttpStatus_404_NotFound = 404,
    HttpStatus_405_MethodNotAllowed = 405,
    HttpStatus_406_NotAcceptable = 406,
    HttpStatus_407_ProxyAuthenticationRequired = 407,
    HttpStatus_408_RequestTimeout = 408,
    HttpStatus_409_Conflict = 409,
    HttpStatus_410_Gone = 410,
    HttpStatus_411_LengthRequired = 411,
    HttpStatus_412_PreconditionFailed = 412,
    HttpStatus_413_RequestEntityTooLarge = 413,
    HttpStatus_414_RequestUriTooLong = 414,
    HttpStatus_415_UnsupportedMediaType = 415,
    HttpStatus_416_RequestedRangeNotSatisfiable = 416,
    HttpStatus_417_ExpectationFailed = 417,
    HttpStatus_422_UnprocessableEntity = 422,
    HttpStatus_423_Locked = 423,
    HttpStatus_424_FailedDependency = 424,
    HttpStatus_426_UpgradeRequired = 426,

    // 5xx Server Error
    HttpStatus_500_InternalServerError = 500,
    HttpStatus_501_NotImplemented = 501,
    HttpStatus_502_BadGateway = 502,
    HttpStatus_503_ServiceUnavailable = 503,
    HttpStatus_504_GatewayTimeout = 504,
    HttpStatus_505_HttpVersionNotSupported = 505,
    HttpStatus_506_VariantAlsoNegotiates = 506,
    HttpStatus_507_InsufficientStorage = 507,
    HttpStatus_509_BandwidthLimitExceeded = 509,
    HttpStatus_510_NotExtended = 510
  };


  enum HttpMethod
  {
    HttpMethod_Get = 0,
    HttpMethod_Post = 1,
    HttpMethod_Delete = 2,
    HttpMethod_Put = 3
  };


  /**
   * WARNING: Do not change the explicit values in the enumerations
   * below this point. This would result in incompatible databases
   * between versions of Orthanc!
   **/

  enum CompressionType
  {
    CompressionType_None = 1,
    CompressionType_Zlib = 2
  };

  enum FileContentType
  {
    FileContentType_Dicom = 1,
    FileContentType_DicomAsJson = 2,

    // Make sure that the value "65535" can be stored into this enumeration
    FileContentType_StartUser = 1024,
    FileContentType_EndUser = 65535
  };

  enum ResourceType
  {
    ResourceType_Patient = 1,
    ResourceType_Study = 2,
    ResourceType_Series = 3,
    ResourceType_Instance = 4
  };


  const char* EnumerationToString(HttpMethod method);

  const char* EnumerationToString(HttpStatus status);

  const char* EnumerationToString(ResourceType type);

  ResourceType StringToResourceType(const char* type);
}
