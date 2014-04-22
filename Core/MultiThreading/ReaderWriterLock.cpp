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


#include "ReaderWriterLock.h"

#include <boost/thread/shared_mutex.hpp>

namespace Orthanc
{
  namespace
  {
    // Anonymous namespace to avoid clashes between compilation
    // modules.

    class ReaderLockable : public ILockable
    {
    private:
      boost::shared_mutex& lock_;

    public:
      ReaderLockable(boost::shared_mutex& lock) : lock_(lock)
      {
      }

      virtual void Lock()
      {
        lock_.lock_shared();
      }

      virtual void Unlock()
      {
        lock_.unlock_shared();        
      }
    };


    class WriterLockable : public ILockable
    {
    private:
      boost::shared_mutex& lock_;

    public:
      WriterLockable(boost::shared_mutex& lock) : lock_(lock)
      {
      }

      virtual void Lock()
      {
        lock_.lock();
      }

      virtual void Unlock()
      {
        lock_.unlock();        
      }
    };
  }

  struct ReaderWriterLock::PImpl
  {
    boost::shared_mutex lock_;
    ReaderLockable reader_;
    WriterLockable writer_;

    PImpl() : reader_(lock_), writer_(lock_)
    {
    }
  };


  ReaderWriterLock::ReaderWriterLock()
  {
    pimpl_ = new PImpl;
  }


  ReaderWriterLock::~ReaderWriterLock()
  {
    delete pimpl_;
  }


  ILockable&  ReaderWriterLock::ForReader()
  {
    return pimpl_->reader_;
  }


  ILockable&  ReaderWriterLock::ForWriter()
  {
    return pimpl_->writer_;
  }
}
