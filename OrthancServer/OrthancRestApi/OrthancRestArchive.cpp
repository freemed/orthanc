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


#include "OrthancRestApi.h"

#include "../../Core/Compression/HierarchicalZipWriter.h"
#include "../../Core/HttpServer/FilesystemHttpSender.h"
#include "../../Core/Uuid.h"

#include <glog/logging.h>

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

static const uint64_t MEGA_BYTES = 1024 * 1024;
static const uint64_t GIGA_BYTES = 1024 * 1024 * 1024;

namespace Orthanc
{
  // Download of ZIP files ----------------------------------------------------
 
  static std::string GetDirectoryNameInArchive(const Json::Value& resource,
                                               ResourceType resourceType)
  {
    switch (resourceType)
    {
      case ResourceType_Patient:
      {
        std::string p = resource["MainDicomTags"]["PatientID"].asString();
        std::string n = resource["MainDicomTags"]["PatientName"].asString();
        return p + " " + n;
      }

      case ResourceType_Study:
      {
        return resource["MainDicomTags"]["StudyDescription"].asString();
      }
        
      case ResourceType_Series:
      {
        std::string d = resource["MainDicomTags"]["SeriesDescription"].asString();
        std::string m = resource["MainDicomTags"]["Modality"].asString();
        return m + " " + d;
      }
        
      default:
        throw OrthancException(ErrorCode_InternalError);
    }
  }

  static bool CreateRootDirectoryInArchive(HierarchicalZipWriter& writer,
                                           ServerContext& context,
                                           const Json::Value& resource,
                                           ResourceType resourceType)
  {
    if (resourceType == ResourceType_Patient)
    {
      return true;
    }

    ResourceType parentType = GetParentResourceType(resourceType);
    Json::Value parent;

    switch (resourceType)
    {
      case ResourceType_Study:
      {
        if (!context.GetIndex().LookupResource(parent, resource["ParentPatient"].asString(), parentType))
        {
          return false;
        }

        break;
      }
        
      case ResourceType_Series:
        if (!context.GetIndex().LookupResource(parent, resource["ParentStudy"].asString(), parentType) ||
            !CreateRootDirectoryInArchive(writer, context, parent, parentType))
        {
          return false;
        }
        break;
        
      default:
        throw OrthancException(ErrorCode_NotImplemented);
    }

    writer.OpenDirectory(GetDirectoryNameInArchive(parent, parentType).c_str());
    return true;
  }

  static bool ArchiveInstance(HierarchicalZipWriter& writer,
                              ServerContext& context,
                              const std::string& instancePublicId,
                              const char* filename)
  {
    Json::Value instance;

    if (!context.GetIndex().LookupResource(instance, instancePublicId, ResourceType_Instance))
    {
      return false;
    }

    writer.OpenFile(filename);

    std::string dicom;
    context.ReadFile(dicom, instancePublicId, FileContentType_Dicom);
    writer.Write(dicom);

    return true;
  }

  static bool ArchiveInternal(HierarchicalZipWriter& writer,
                              ServerContext& context,
                              const std::string& publicId,
                              ResourceType resourceType,
                              bool isFirstLevel)
  { 
    Json::Value resource;
    if (!context.GetIndex().LookupResource(resource, publicId, resourceType))
    {
      return false;
    }    

    if (isFirstLevel && 
        !CreateRootDirectoryInArchive(writer, context, resource, resourceType))
    {
      return false;
    }

    writer.OpenDirectory(GetDirectoryNameInArchive(resource, resourceType).c_str());

    switch (resourceType)
    {
      case ResourceType_Patient:
        for (Json::Value::ArrayIndex i = 0; i < resource["Studies"].size(); i++)
        {
          std::string studyId = resource["Studies"][i].asString();
          if (!ArchiveInternal(writer, context, studyId, ResourceType_Study, false))
          {
            return false;
          }
        }
        break;

      case ResourceType_Study:
        for (Json::Value::ArrayIndex i = 0; i < resource["Series"].size(); i++)
        {
          std::string seriesId = resource["Series"][i].asString();
          if (!ArchiveInternal(writer, context, seriesId, ResourceType_Series, false))
          {
            return false;
          }
        }
        break;

      case ResourceType_Series:
      {
        // Create a filename prefix, depending on the modality
        char format[16] = "%08d";

        if (resource["MainDicomTags"].isMember("Modality"))
        {
          std::string modality = resource["MainDicomTags"]["Modality"].asString();

          if (modality.size() == 1)
          {
            snprintf(format, sizeof(format) - 1, "%c%%07d", toupper(modality[0]));
          }
          else if (modality.size() >= 2)
          {
            snprintf(format, sizeof(format) - 1, "%c%c%%06d", toupper(modality[0]), toupper(modality[1]));
          }
        }

        char filename[16];

        for (Json::Value::ArrayIndex i = 0; i < resource["Instances"].size(); i++)
        {
          snprintf(filename, sizeof(filename) - 1, format, i);

          std::string publicId = resource["Instances"][i].asString();

          // This was the implementation up to Orthanc 0.7.0:
          // std::string filename = instance["MainDicomTags"]["SOPInstanceUID"].asString() + ".dcm";

          if (!ArchiveInstance(writer, context, publicId, filename))
          {
            return false;
          }
        }

        break;
      }

      default:
        throw OrthancException(ErrorCode_InternalError);
    }

    writer.CloseDirectory();
    return true;
  }                                 

  template <enum ResourceType resourceType>
  static void GetArchive(RestApi::GetCall& call)
  {
    ServerContext& context = OrthancRestApi::GetContext(call);

    std::string id = call.GetUriComponent("id", "");

    /**
     * Determine whether ZIP64 is required. Original ZIP format can
     * store up to 2GB of data (some implementation supporting up to
     * 4GB of data), and up to 65535 files.
     * https://en.wikipedia.org/wiki/Zip_(file_format)#ZIP64
     **/

    uint64_t uncompressedSize;
    uint64_t compressedSize;
    unsigned int countStudies;
    unsigned int countSeries;
    unsigned int countInstances;
    context.GetIndex().GetStatistics(compressedSize, uncompressedSize, 
                                     countStudies, countSeries, countInstances, id);
    const bool isZip64 = (uncompressedSize >= 2 * GIGA_BYTES ||
                          countInstances >= 65535);

    LOG(INFO) << "Creating a ZIP file with " << countInstances << " files of size "
              << (uncompressedSize / MEGA_BYTES) << "MB using the "
              << (isZip64 ? "ZIP64" : "ZIP32") << " file format";

    // Create a RAII for the temporary file to manage the ZIP file
    Toolbox::TemporaryFile tmp;

    {
      // Create a ZIP writer
      HierarchicalZipWriter writer(tmp.GetPath().c_str());
      writer.SetZip64(isZip64);

      // Store the requested resource into the ZIP
      if (!ArchiveInternal(writer, context, id, resourceType, true))
      {
        return;
      }
    }

    // Prepare the sending of the ZIP file
    FilesystemHttpSender sender(tmp.GetPath().c_str());
    sender.SetContentType("application/zip");
    sender.SetDownloadFilename(id + ".zip");

    // Send the ZIP
    call.GetOutput().AnswerFile(sender);

    // The temporary file is automatically removed thanks to the RAII
  }


  void OrthancRestApi::RegisterArchive()
  {
    Register("/patients/{id}/archive", GetArchive<ResourceType_Patient>);
    Register("/studies/{id}/archive", GetArchive<ResourceType_Study>);
    Register("/series/{id}/archive", GetArchive<ResourceType_Series>);
  }
}
