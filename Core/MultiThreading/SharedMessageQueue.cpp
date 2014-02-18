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


#include "SharedMessageQueue.h"

namespace Orthanc
{
  SharedMessageQueue::SharedMessageQueue(unsigned int maxSize)
  {
    maxSize_ = maxSize;
  }


  SharedMessageQueue::~SharedMessageQueue()
  {
    for (Queue::iterator it = queue_.begin(); it != queue_.end(); ++it)
    {
      delete *it;
    }
  }


  void SharedMessageQueue::Enqueue(IDynamicObject* message)
  {
    boost::mutex::scoped_lock lock(mutex_);

    if (maxSize_ != 0 && queue_.size() > maxSize_)
    {
      // Too many elements in the queue: First remove the oldest
      delete queue_.front();
      queue_.pop_front();
    }

    queue_.push_back(message);
    elementAvailable_.notify_one();
  }


  IDynamicObject* SharedMessageQueue::Dequeue(int32_t millisecondsTimeout)
  {
    boost::mutex::scoped_lock lock(mutex_);

    // Wait for a message to arrive in the FIFO queue
    while (queue_.empty())
    {
      if (millisecondsTimeout == 0)
      {
        elementAvailable_.wait(lock);
      }
      else
      {
        bool success = elementAvailable_.timed_wait
          (lock, boost::posix_time::milliseconds(millisecondsTimeout));
        if (!success)
        {
          return NULL;
        }
      }
    }

    std::auto_ptr<IDynamicObject> message(queue_.front());
    queue_.pop_front();

    if (queue_.empty())
    {
      emptied_.notify_all();
    }

    return message.release();
  }



  bool SharedMessageQueue::WaitEmpty(int32_t millisecondsTimeout)
  {
    boost::mutex::scoped_lock lock(mutex_);
    
    // Wait for the queue to become empty
    while (!queue_.empty())
    {
      if (millisecondsTimeout == 0)
      {
        emptied_.wait(lock);
      }
      else
      {
        if (!emptied_.timed_wait
            (lock, boost::posix_time::milliseconds(millisecondsTimeout)))
        {
          return false;
        }
      }
    }

    return true;
  }
}
