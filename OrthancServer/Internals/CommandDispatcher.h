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

#include "../DicomProtocol/DicomServer.h"
#include "../../Core/MultiThreading/IRunnableBySteps.h"

#include <dcmtk/dcmnet/dimse.h>

namespace Orthanc
{
  namespace Internals
  {
    OFCondition AssociationCleanup(T_ASC_Association *assoc);

    class CommandDispatcher : public IRunnableBySteps
    {
    private:
      uint32_t clientTimeout_;
      uint32_t elapsedTimeSinceLastCommand_;
      const DicomServer& server_;
      T_ASC_Association* assoc_;
      std::string callingIP_;
      std::string callingAETitle_;
      IApplicationEntityFilter* filter_;

    public:
      CommandDispatcher(const DicomServer& server,
                        T_ASC_Association* assoc,
                        const std::string& callingIP,
                        const std::string& callingAETitle,
                        IApplicationEntityFilter* filter) :
        server_(server),
        assoc_(assoc),
        callingIP_(callingIP),
        callingAETitle_(callingAETitle),
        filter_(filter)
      {
        clientTimeout_ = server.GetClientTimeout();
        elapsedTimeSinceLastCommand_ = 0;
      }

      virtual ~CommandDispatcher()
      {
        AssociationCleanup(assoc_);
      }

      virtual bool Step();
    };

    OFCondition EchoScp(T_ASC_Association * assoc, 
                        T_DIMSE_Message * msg, 
                        T_ASC_PresentationContextID presID);

    CommandDispatcher* AcceptAssociation(const DicomServer& server, 
                                         T_ASC_Network *net);
  }
}
