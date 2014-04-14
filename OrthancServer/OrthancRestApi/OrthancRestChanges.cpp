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

#include <glog/logging.h>

namespace Orthanc
{
  // Changes API --------------------------------------------------------------
 
  static void GetSinceAndLimit(int64_t& since,
                               unsigned int& limit,
                               bool& last,
                               const RestApi::GetCall& call)
  {
    static const unsigned int MAX_RESULTS = 100;
    
    if (call.HasArgument("last"))
    {
      last = true;
      return;
    }

    last = false;

    try
    {
      since = boost::lexical_cast<int64_t>(call.GetArgument("since", "0"));
      limit = boost::lexical_cast<unsigned int>(call.GetArgument("limit", "0"));
    }
    catch (boost::bad_lexical_cast)
    {
      return;
    }

    if (limit == 0 || limit > MAX_RESULTS)
    {
      limit = MAX_RESULTS;
    }
  }

  static void GetChanges(RestApi::GetCall& call)
  {
    ServerContext& context = OrthancRestApi::GetContext(call);

    //std::string filter = GetArgument(getArguments, "filter", "");
    int64_t since;
    unsigned int limit;
    bool last;
    GetSinceAndLimit(since, limit, last, call);

    Json::Value result;
    if ((!last && context.GetIndex().GetChanges(result, since, limit)) ||
        ( last && context.GetIndex().GetLastChange(result)))
    {
      call.GetOutput().AnswerJson(result);
    }
  }


  static void DeleteChanges(RestApi::DeleteCall& call)
  {
    OrthancRestApi::GetIndex(call).DeleteChanges();
    call.GetOutput().AnswerBuffer("", "text/plain");
  }


  // Exports API --------------------------------------------------------------
 
  static void GetExports(RestApi::GetCall& call)
  {
    ServerContext& context = OrthancRestApi::GetContext(call);

    int64_t since;
    unsigned int limit;
    bool last;
    GetSinceAndLimit(since, limit, last, call);

    Json::Value result;
    if ((!last && context.GetIndex().GetExportedResources(result, since, limit)) ||
        ( last && context.GetIndex().GetLastExportedResource(result)))
    {
      call.GetOutput().AnswerJson(result);
    }
  }


  static void DeleteExports(RestApi::DeleteCall& call)
  {
    OrthancRestApi::GetIndex(call).DeleteExportedResources();
    call.GetOutput().AnswerBuffer("", "text/plain");
  }
  

  void OrthancRestApi::RegisterChanges()
  {
    Register("/changes", GetChanges);
    Register("/changes", DeleteChanges);
    Register("/exports", GetExports);
    Register("/exports", DeleteExports);
  }
}
