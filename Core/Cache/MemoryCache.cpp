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


#include "MemoryCache.h"

#include <stdlib.h>  // This fixes a problem in glog for recent
                     // releases of MinGW
#include <glog/logging.h>

namespace Orthanc
{
  MemoryCache::Page& MemoryCache::Load(const std::string& id)
  {
    // Reuse the cache entry if it already exists
    Page* p = NULL;
    if (index_.Contains(id, p))
    {
      VLOG(1) << "Reusing a cache page";
      assert(p != NULL);
      index_.MakeMostRecent(id);
      return *p;
    }

    // The id is not in the cache yet. Make some room if the cache
    // is full.
    if (index_.GetSize() == cacheSize_)
    {
      VLOG(1) << "Dropping the oldest cache page";
      index_.RemoveOldest(p);
      delete p;
    }

    // Create a new cache page
    std::auto_ptr<Page> result(new Page);
    result->id_ = id;
    result->content_.reset(provider_.Provide(id));

    // Add the newly create page to the cache
    VLOG(1) << "Registering new data in a cache page";
    p = result.release();
    index_.Add(id, p);
    return *p;
  }

  MemoryCache::MemoryCache(ICachePageProvider& provider,
                           size_t cacheSize) : 
    provider_(provider),
    cacheSize_(cacheSize)
  {
  }

  MemoryCache::~MemoryCache()
  {
    while (!index_.IsEmpty())
    {
      Page* element = NULL;
      index_.RemoveOldest(element);
      assert(element != NULL);
      delete element;
    }
  }

  IDynamicObject& MemoryCache::Access(const std::string& id)
  {
    return *Load(id).content_;
  }
}
