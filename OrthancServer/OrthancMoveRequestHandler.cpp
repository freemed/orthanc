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

#include "OrthancMoveRequestHandler.h"

#include <glog/logging.h>

#include "DicomProtocol/DicomUserConnection.h"
#include "OrthancInitialization.h"

namespace Orthanc
{
  namespace
  {
    // Anonymous namespace to avoid clashes between compilation modules

    class OrthancMoveRequestIterator : public IMoveRequestIterator
    {
    private:
      ServerContext& context_;
      std::vector<std::string> instances_;
      DicomUserConnection connection_;
      size_t position_;

    public:
      OrthancMoveRequestIterator(ServerContext& context,
                                 const std::string& target,
                                 const std::string& publicId) :
        context_(context),
        position_(0)
      {
        LOG(INFO) << "Sending resource " << publicId << " to modality \"" << target << "\"";

        std::list<std::string> tmp;
        context_.GetIndex().GetChildInstances(tmp, publicId);

        instances_.reserve(tmp.size());
        for (std::list<std::string>::iterator it = tmp.begin(); it != tmp.end(); ++it)
        {
          instances_.push_back(*it);
        }
    
        ConnectToModalityUsingAETitle(connection_, target);
      }

      virtual unsigned int GetSubOperationCount() const
      {
        return instances_.size();
      }

      virtual Status DoNext()
      {
        if (position_ >= instances_.size())
        {
          return Status_Failure;
        }

        const std::string& id = instances_[position_++];

        std::string dicom;
        context_.ReadFile(dicom, id, FileContentType_Dicom);
        connection_.Store(dicom);

        return Status_Success;
      }
    };
  }


  bool OrthancMoveRequestHandler::LookupResource(std::string& publicId,
                                                 DicomTag tag,
                                                 const DicomMap& input)
  {
    if (!input.HasTag(tag))
    {
      return false;
    }

    std::string value = input.GetValue(tag).AsString();

    std::list<std::string> ids;
    context_.GetIndex().LookupTagValue(ids, tag, value);

    if (ids.size() != 1)
    {
      return false;
    }
    else
    {
      publicId = ids.front();
      return true;
    }
  }


  IMoveRequestIterator* OrthancMoveRequestHandler::Handle(const std::string& target,
                                                          const DicomMap& input)
  {
    LOG(WARNING) << "Move-SCU request received for AET \"" << target << "\"";


    /**
     * Retrieve the query level.
     **/

    const DicomValue* levelTmp = input.TestAndGetValue(DICOM_TAG_QUERY_RETRIEVE_LEVEL);
    if (levelTmp == NULL) 
    {
      throw OrthancException(ErrorCode_BadRequest);
    }

    ResourceType level = StringToResourceType(levelTmp->AsString().c_str());

    /**
     * Lookup for the resource to be sent.
     **/

    bool ok;
    std::string publicId;

    switch (level)
    {
      case ResourceType_Patient:
        ok = LookupResource(publicId, DICOM_TAG_PATIENT_ID, input);
        break;

      case ResourceType_Study:
        ok = LookupResource(publicId, DICOM_TAG_STUDY_INSTANCE_UID, input);
        break;

      case ResourceType_Series:
        ok = LookupResource(publicId, DICOM_TAG_SERIES_INSTANCE_UID, input);
        break;

      case ResourceType_Instance:
        ok = LookupResource(publicId, DICOM_TAG_SOP_INSTANCE_UID, input);
        break;

      default:
        ok = false;
    }

    if (!ok)
    {
      throw OrthancException(ErrorCode_BadRequest);
    }

    return new OrthancMoveRequestIterator(context_, target, publicId);
  }
}
