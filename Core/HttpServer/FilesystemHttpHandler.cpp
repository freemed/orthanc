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


#include "FilesystemHttpHandler.h"

#include "../OrthancException.h"
#include "FilesystemHttpSender.h"

#include <boost/filesystem.hpp>


namespace Orthanc
{
  struct FilesystemHttpHandler::PImpl
  {
    UriComponents baseUri_;
    boost::filesystem::path root_;
  };



  static void OutputDirectoryContent(HttpOutput& output,
                                     const UriComponents& uri,
                                     const boost::filesystem::path& p)
  {
    namespace fs = boost::filesystem;

    output.SendOkHeader("text/html", false, 0, NULL);
    output.SendString("<html>");
    output.SendString("  <body>");
    output.SendString("    <h1>Subdirectories</h1>");
    output.SendString("    <ul>");

    if (uri.size() > 0)
    {
      std::string h = Toolbox::FlattenUri(uri) + "/..";
      output.SendString("<li><a href=\"" + h + "\">..</a></li>");
    }

    fs::directory_iterator end;
    for (fs::directory_iterator it(p) ; it != end; ++it)
    {
#if BOOST_HAS_FILESYSTEM_V3 == 1
      std::string f = it->path().filename().string();
#else
      std::string f = it->path().filename();
#endif

      std::string h = Toolbox::FlattenUri(uri) + "/" + f;
      if (fs::is_directory(it->status()))
        output.SendString("<li><a href=\"" + h + "\">" + f + "</a></li>");
    }      

    output.SendString("    </ul>");      
    output.SendString("    <h1>Files</h1>");
    output.SendString("    <ul>");

    for (fs::directory_iterator it(p) ; it != end; ++it)
    {
#if BOOST_HAS_FILESYSTEM_V3 == 1
      std::string f = it->path().filename().string();
#else
      std::string f = it->path().filename();
#endif

      std::string h = Toolbox::FlattenUri(uri) + "/" + f;
      if (fs::is_regular_file(it->status()))
        output.SendString("<li><a href=\"" + h + "\">" + f + "</a></li>");
    }      

    output.SendString("    </ul>");
    output.SendString("  </body>");
    output.SendString("</html>");
  }


  FilesystemHttpHandler::FilesystemHttpHandler(const std::string& baseUri,
                                               const std::string& root) : pimpl_(new PImpl)
  {
    Toolbox::SplitUriComponents(pimpl_->baseUri_, baseUri);
    pimpl_->root_ = root;
    listDirectoryContent_ = false;
    
    namespace fs = boost::filesystem;
    if (!fs::exists(pimpl_->root_) || 
        !fs::is_directory(pimpl_->root_))
    {
      throw OrthancException("The path does not point to a directory");
    }
  }


  bool FilesystemHttpHandler::IsServedUri(const UriComponents& uri)
  {
    return Toolbox::IsChildUri(pimpl_->baseUri_, uri);
  }


  void FilesystemHttpHandler::Handle(
    HttpOutput& output,
    HttpMethod method,
    const UriComponents& uri,
    const Arguments& headers,
    const Arguments& arguments,
    const std::string&)
  {
    if (method != HttpMethod_Get)
    {
      output.SendMethodNotAllowedError("GET");
      return;
    }

    namespace fs = boost::filesystem;

    fs::path p = pimpl_->root_;
    for (size_t i = pimpl_->baseUri_.size(); i < uri.size(); i++)
    {
      p /= uri[i];
    }

    if (fs::exists(p) && fs::is_regular_file(p))
    {
      FilesystemHttpSender(p).Send(output);

      //output.AnswerFileAutodetectContentType(p.string());
    }
    else if (listDirectoryContent_ &&
             fs::exists(p) && 
             fs::is_directory(p))
    {
      OutputDirectoryContent(output, uri, p);
    }
    else
    {
      output.SendHeader(HttpStatus_404_NotFound);
    }
  } 
}
