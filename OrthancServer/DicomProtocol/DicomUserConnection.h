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

#include "DicomFindAnswers.h"
#include "../ServerEnumerations.h"
#include "RemoteModalityParameters.h"

#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <list>

namespace Orthanc
{
  class DicomUserConnection : public boost::noncopyable
  {
  private:
    enum FindRootModel
    {
      FindRootModel_Patient,
      FindRootModel_Study,
      FindRootModel_Series,
      FindRootModel_Instance
    };

    struct PImpl;
    boost::shared_ptr<PImpl> pimpl_;

    // Connection parameters
    std::string preferredTransferSyntax_;
    std::string localAet_;
    std::string distantAet_;
    std::string distantHost_;
    uint16_t distantPort_;
    ModalityManufacturer manufacturer_;
    std::set<std::string> storageSOPClasses_;
    std::list<std::string> reservedStorageSOPClasses_;
    std::set<std::string> defaultStorageSOPClasses_;

    void CheckIsOpen() const;

    void SetupPresentationContexts(const std::string& preferredTransferSyntax);

    void Find(DicomFindAnswers& result,
              FindRootModel model,
              const DicomMap& fields);

    void Move(const std::string& targetAet,
              const DicomMap& fields);

    void ResetStorageSOPClasses();

    void CheckStorageSOPClassesInvariant() const;

  public:
    DicomUserConnection();

    ~DicomUserConnection();

    void Connect(const RemoteModalityParameters& parameters);

    void SetLocalApplicationEntityTitle(const std::string& aet);

    const std::string& GetLocalApplicationEntityTitle() const
    {
      return localAet_;
    }

    void SetDistantApplicationEntityTitle(const std::string& aet);

    const std::string& GetDistantApplicationEntityTitle() const
    {
      return distantAet_;
    }

    void SetDistantHost(const std::string& host);

    const std::string& GetDistantHost() const
    {
      return distantHost_;
    }

    void SetDistantPort(uint16_t port);

    uint16_t GetDistantPort() const
    {
      return distantPort_;
    }

    void SetDistantManufacturer(ModalityManufacturer manufacturer);

    ModalityManufacturer GetDistantManufacturer() const
    {
      return manufacturer_;
    }

    void ResetPreferredTransferSyntax();

    void SetPreferredTransferSyntax(const std::string& preferredTransferSyntax);

    const std::string& GetPreferredTransferSyntax() const
    {
      return preferredTransferSyntax_;
    }

    void AddStorageSOPClass(const char* sop);

    void Open();

    void Close();

    bool IsOpen() const;

    bool Echo();

    void Store(const char* buffer, size_t size);

    void Store(const std::string& buffer);

    void StoreFile(const std::string& path);

    void FindPatient(DicomFindAnswers& result,
                     const DicomMap& fields);

    void FindStudy(DicomFindAnswers& result,
                   const DicomMap& fields);

    void FindSeries(DicomFindAnswers& result,
                    const DicomMap& fields);

    void FindInstance(DicomFindAnswers& result,
                      const DicomMap& fields);

    void MoveSeries(const std::string& targetAet,
                    const DicomMap& findResult);

    void MoveSeries(const std::string& targetAet,
                    const std::string& studyUid,
                    const std::string& seriesUid);

    void MoveInstance(const std::string& targetAet,
                      const DicomMap& findResult);

    void MoveInstance(const std::string& targetAet,
                      const std::string& studyUid,
                      const std::string& seriesUid,
                      const std::string& instanceUid);

    static void SetConnectionTimeout(uint32_t seconds);
  };
}
