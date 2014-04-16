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


#include "FindScp.h"

#include "../FromDcmtkBridge.h"
#include "../ToDcmtkBridge.h"
#include "../../Core/OrthancException.h"

#include <glog/logging.h>


namespace Orthanc
{
  namespace
  {  
    struct FindScpData
    {
      IFindRequestHandler* handler_;
      DicomMap input_;
      DicomFindAnswers answers_;
      DcmDataset* lastRequest_;
      const std::string* callingAETitle_;
    };


    void FindScpCallback(
      /* in */ 
      void *callbackData,  
      OFBool cancelled, 
      T_DIMSE_C_FindRQ *request, 
      DcmDataset *requestIdentifiers, 
      int responseCount,
      /* out */
      T_DIMSE_C_FindRSP *response,
      DcmDataset **responseIdentifiers,
      DcmDataset **statusDetail)
    {
      bzero(response, sizeof(T_DIMSE_C_FindRSP));
      *statusDetail = NULL;

      FindScpData& data = *reinterpret_cast<FindScpData*>(callbackData);
      if (data.lastRequest_ == NULL)
      {
        FromDcmtkBridge::Convert(data.input_, *requestIdentifiers);

        try
        {
          data.handler_->Handle(data.answers_, data.input_, *data.callingAETitle_);
        }
        catch (OrthancException& e)
        {
          // Internal error!
          LOG(ERROR) <<  "IFindRequestHandler Failed: " << e.What();
          response->DimseStatus = STATUS_FIND_Failed_UnableToProcess;
          *responseIdentifiers = NULL;   
          return;
        }

        data.lastRequest_ = requestIdentifiers;
      }
      else if (data.lastRequest_ != requestIdentifiers)
      {
        // Internal error!
        response->DimseStatus = STATUS_FIND_Failed_UnableToProcess;
        *responseIdentifiers = NULL;   
        return;
      }

      if (responseCount <= static_cast<int>(data.answers_.GetSize()))
      {
        response->DimseStatus = STATUS_Pending;
        *responseIdentifiers = ToDcmtkBridge::Convert(data.answers_.GetAnswer(responseCount - 1));
      }
      else
      {
        response->DimseStatus = STATUS_Success;
        *responseIdentifiers = NULL;
      }
    }
  }


  OFCondition Internals::findScp(T_ASC_Association * assoc, 
                                 T_DIMSE_Message * msg, 
                                 T_ASC_PresentationContextID presID,
                                 IFindRequestHandler& handler,
                                 const std::string& callingAETitle)
  {
    FindScpData data;
    data.lastRequest_ = NULL;
    data.handler_ = &handler;
    data.callingAETitle_ = &callingAETitle;

    OFCondition cond = DIMSE_findProvider(assoc, presID, &msg->msg.CFindRQ, 
                                          FindScpCallback, &data,
                                          /*opt_blockMode*/ DIMSE_BLOCKING, 
                                          /*opt_dimse_timeout*/ 0);

    // if some error occured, dump corresponding information and remove the outfile if necessary
    if (cond.bad())
    {
      OFString temp_str;
      LOG(ERROR) << "Find SCP Failed: " << cond.text();
    }

    return cond;
  }
}
