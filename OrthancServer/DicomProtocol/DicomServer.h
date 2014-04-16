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

#include "IFindRequestHandlerFactory.h"
#include "IMoveRequestHandlerFactory.h"
#include "IStoreRequestHandlerFactory.h"
#include "IApplicationEntityFilter.h"
#include "../../Core/MultiThreading/BagOfRunnablesBySteps.h"

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace Orthanc
{
  class DicomServer : public boost::noncopyable
  {
  private:
    struct PImpl;
    boost::shared_ptr<PImpl> pimpl_;

    bool checkCalledAet_;
    std::string aet_;
    uint16_t port_;
    bool continue_;
    bool started_;
    uint32_t clientTimeout_;
    bool isThreaded_;
    IFindRequestHandlerFactory* findRequestHandlerFactory_;
    IMoveRequestHandlerFactory* moveRequestHandlerFactory_;
    IStoreRequestHandlerFactory* storeRequestHandlerFactory_;
    IApplicationEntityFilter* applicationEntityFilter_;

    BagOfRunnablesBySteps bagOfDispatchers_;  // This is used iff the server is threaded

    static void ServerThread(DicomServer* server);

  public:
    static void InitializeDictionary();

    DicomServer();

    ~DicomServer();

    void SetPortNumber(uint16_t port);
    uint16_t GetPortNumber() const;

    void SetThreaded(bool isThreaded);
    bool IsThreaded() const;

    void SetClientTimeout(uint32_t timeout);
    uint32_t GetClientTimeout() const;

    void SetCalledApplicationEntityTitleCheck(bool check);
    bool HasCalledApplicationEntityTitleCheck() const;

    void SetApplicationEntityTitle(const std::string& aet);
    const std::string& GetApplicationEntityTitle() const;

    void SetFindRequestHandlerFactory(IFindRequestHandlerFactory& handler);
    bool HasFindRequestHandlerFactory() const;
    IFindRequestHandlerFactory& GetFindRequestHandlerFactory() const;

    void SetMoveRequestHandlerFactory(IMoveRequestHandlerFactory& handler);
    bool HasMoveRequestHandlerFactory() const;
    IMoveRequestHandlerFactory& GetMoveRequestHandlerFactory() const;

    void SetStoreRequestHandlerFactory(IStoreRequestHandlerFactory& handler);
    bool HasStoreRequestHandlerFactory() const;
    IStoreRequestHandlerFactory& GetStoreRequestHandlerFactory() const;

    void SetApplicationEntityFilter(IApplicationEntityFilter& handler);
    bool HasApplicationEntityFilter() const;
    IApplicationEntityFilter& GetApplicationEntityFilter() const;

    void Start();
  
    void Stop();

    bool IsMyAETitle(const std::string& aet) const;
  };

}
