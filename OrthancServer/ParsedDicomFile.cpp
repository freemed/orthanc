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



/*=========================================================================

  This file is based on portions of the following project:

  Program: GDCM (Grassroots DICOM). A DICOM library
  Module:  http://gdcm.sourceforge.net/Copyright.html

Copyright (c) 2006-2011 Mathieu Malaterre
Copyright (c) 1993-2005 CREATIS
(CREATIS = Centre de Recherche et d'Applications en Traitement de l'Image)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Mathieu Malaterre, or CREATIS, nor the names of any
   contributors (CNRS, INSERM, UCB, Universite Lyon I), may be used to
   endorse or promote products derived from this software without specific
   prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/


#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "ParsedDicomFile.h"

#include "FromDcmtkBridge.h"
#include "ToDcmtkBridge.h"
#include "../Core/Toolbox.h"
#include "../Core/OrthancException.h"
#include "../Core/ImageFormats/PngWriter.h"
#include "../Core/Uuid.h"
#include "../Core/DicomFormat/DicomString.h"
#include "../Core/DicomFormat/DicomNullValue.h"
#include "../Core/DicomFormat/DicomIntegerPixelAccessor.h"

#include <list>
#include <limits>

#include <boost/lexical_cast.hpp>

#include <dcmtk/dcmdata/dcchrstr.h>
#include <dcmtk/dcmdata/dcdicent.h>
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcistrmb.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmdata/dcmetinf.h>

#include <dcmtk/dcmdata/dcvrae.h>
#include <dcmtk/dcmdata/dcvras.h>
#include <dcmtk/dcmdata/dcvrcs.h>
#include <dcmtk/dcmdata/dcvrda.h>
#include <dcmtk/dcmdata/dcvrds.h>
#include <dcmtk/dcmdata/dcvrdt.h>
#include <dcmtk/dcmdata/dcvrfd.h>
#include <dcmtk/dcmdata/dcvrfl.h>
#include <dcmtk/dcmdata/dcvris.h>
#include <dcmtk/dcmdata/dcvrlo.h>
#include <dcmtk/dcmdata/dcvrlt.h>
#include <dcmtk/dcmdata/dcvrpn.h>
#include <dcmtk/dcmdata/dcvrsh.h>
#include <dcmtk/dcmdata/dcvrsl.h>
#include <dcmtk/dcmdata/dcvrss.h>
#include <dcmtk/dcmdata/dcvrst.h>
#include <dcmtk/dcmdata/dcvrtm.h>
#include <dcmtk/dcmdata/dcvrui.h>
#include <dcmtk/dcmdata/dcvrul.h>
#include <dcmtk/dcmdata/dcvrus.h>
#include <dcmtk/dcmdata/dcvrut.h>
#include <dcmtk/dcmdata/dcpixel.h>
#include <dcmtk/dcmdata/dcpixseq.h>
#include <dcmtk/dcmdata/dcpxitem.h>


#include <boost/math/special_functions/round.hpp>
#include <glog/logging.h>
#include <dcmtk/dcmdata/dcostrmb.h>


static const char* CONTENT_TYPE_OCTET_STREAM = "application/octet-stream";



namespace Orthanc
{
  struct ParsedDicomFile::PImpl
  {
    std::auto_ptr<DcmFileFormat> file_;
  };


  // This method can only be called from the constructors!
  void ParsedDicomFile::Setup(const char* buffer, size_t size)
  {
    DcmInputBufferStream is;
    if (size > 0)
    {
      is.setBuffer(buffer, size);
    }
    is.setEos();

    pimpl_->file_.reset(new DcmFileFormat);
    pimpl_->file_->transferInit();
    if (!pimpl_->file_->read(is).good())
    {
      delete pimpl_;  // Avoid a memory leak due to exception
                      // throwing, as we are in the constructor

      throw OrthancException(ErrorCode_BadFileFormat);
    }
    pimpl_->file_->loadAllDataIntoMemory();
    pimpl_->file_->transferEnd();
  }


  static void SendPathValueForDictionary(RestApiOutput& output,
                                         DcmItem& dicom)
  {
    Json::Value v = Json::arrayValue;

    for (unsigned long i = 0; i < dicom.card(); i++)
    {
      DcmElement* element = dicom.getElement(i);
      if (element)
      {
        char buf[16];
        sprintf(buf, "%04x-%04x", element->getTag().getGTag(), element->getTag().getETag());
        v.append(buf);
      }
    }

    output.AnswerJson(v);
  }

  static inline uint16_t GetCharValue(char c)
  {
    if (c >= '0' && c <= '9')
      return c - '0';
    else if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    else
      return 0;
  }

  static inline uint16_t GetTagValue(const char* c)
  {
    return ((GetCharValue(c[0]) << 12) + 
            (GetCharValue(c[1]) << 8) + 
            (GetCharValue(c[2]) << 4) + 
            GetCharValue(c[3]));
  }

  static void ParseTagAndGroup(DcmTagKey& key,
                               const std::string& tag)
  {
    DicomTag t = FromDcmtkBridge::ParseTag(tag);
    key = DcmTagKey(t.GetGroup(), t.GetElement());
  }


  static void SendSequence(RestApiOutput& output,
                           DcmSequenceOfItems& sequence)
  {
    // This element is a sequence
    Json::Value v = Json::arrayValue;

    for (unsigned long i = 0; i < sequence.card(); i++)
    {
      v.append(boost::lexical_cast<std::string>(i));
    }

    output.AnswerJson(v);
  }


  static unsigned int GetPixelDataBlockCount(DcmPixelData& pixelData,
                                             E_TransferSyntax transferSyntax)
  {
    DcmPixelSequence* pixelSequence = NULL;
    if (pixelData.getEncapsulatedRepresentation
        (transferSyntax, NULL, pixelSequence).good() && pixelSequence)
    {
      return pixelSequence->card();
    }
    else
    {
      return 1;
    }
  }


  static void AnswerDicomField(RestApiOutput& output,
                               DcmElement& element,
                               E_TransferSyntax transferSyntax)
  {
    // This element is nor a sequence, neither a pixel-data
    std::string buffer;
    buffer.resize(65536);
    Uint32 length = element.getLength(transferSyntax);
    Uint32 offset = 0;

    output.GetLowLevelOutput().SendOkHeader(CONTENT_TYPE_OCTET_STREAM, true, length, NULL);

    while (offset < length)
    {
      Uint32 nbytes;
      if (length - offset < buffer.size())
      {
        nbytes = length - offset;
      }
      else
      {
        nbytes = buffer.size();
      }

      OFCondition cond = element.getPartialValue(&buffer[0], offset, nbytes);

      if (cond.good())
      {
        output.GetLowLevelOutput().Send(&buffer[0], nbytes);
        offset += nbytes;
      }
      else
      {
        LOG(ERROR) << "Error while sending a DICOM field: " << cond.text();
        return;
      }
    }

    output.MarkLowLevelOutputDone();
  }


  static bool AnswerPixelData(RestApiOutput& output,
                              DcmItem& dicom,
                              E_TransferSyntax transferSyntax,
                              const std::string* blockUri)
  {
    DcmTag k(DICOM_TAG_PIXEL_DATA.GetGroup(),
             DICOM_TAG_PIXEL_DATA.GetElement());

    DcmElement *element = NULL;
    if (!dicom.findAndGetElement(k, element).good() ||
        element == NULL)
    {
      return false;
    }

    try
    {
      DcmPixelData& pixelData = dynamic_cast<DcmPixelData&>(*element);
      if (blockUri == NULL)
      {
        // The user asks how many blocks are presents in this pixel data
        unsigned int blocks = GetPixelDataBlockCount(pixelData, transferSyntax);

        Json::Value result(Json::arrayValue);
        for (unsigned int i = 0; i < blocks; i++)
        {
          result.append(boost::lexical_cast<std::string>(i));
        }
        
        output.AnswerJson(result);
        return true;
      }


      unsigned int block = boost::lexical_cast<unsigned int>(*blockUri);

      if (block < GetPixelDataBlockCount(pixelData, transferSyntax))
      {
        DcmPixelSequence* pixelSequence = NULL;
        if (pixelData.getEncapsulatedRepresentation
            (transferSyntax, NULL, pixelSequence).good() && pixelSequence)
        {
          // This is the case for JPEG transfer syntaxes
          if (block < pixelSequence->card())
          {
            DcmPixelItem* pixelItem = NULL;
            if (pixelSequence->getItem(pixelItem, block).good() && pixelItem)
            {
              if (pixelItem->getLength() == 0)
              {
                output.AnswerBuffer(NULL, 0, CONTENT_TYPE_OCTET_STREAM);
                return true;
              }

              Uint8* buffer = NULL;
              if (pixelItem->getUint8Array(buffer).good() && buffer)
              {
                output.AnswerBuffer(buffer, pixelItem->getLength(), CONTENT_TYPE_OCTET_STREAM);
                return true;
              }
            }
          }
        }
        else
        {
          // This is the case for raw, uncompressed image buffers
          assert(*blockUri == "0");
          AnswerDicomField(output, *element, transferSyntax);
        }
      }
    }
    catch (boost::bad_lexical_cast&)
    {
      // The URI entered by the user is not a number
    }
    catch (std::bad_cast&)
    {
      // This should never happen
    }

    return false;
  }



  static void SendPathValueForLeaf(RestApiOutput& output,
                                   const std::string& tag,
                                   DcmItem& dicom,
                                   E_TransferSyntax transferSyntax)
  {
    DcmTagKey k;
    ParseTagAndGroup(k, tag);

    DcmSequenceOfItems* sequence = NULL;
    if (dicom.findAndGetSequence(k, sequence).good() && 
        sequence != NULL &&
        sequence->getVR() == EVR_SQ)
    {
      SendSequence(output, *sequence);
      return;
    }

    DcmElement* element = NULL;
    if (dicom.findAndGetElement(k, element).good() && 
        element != NULL &&
        //element->getVR() != EVR_UNKNOWN &&  // This would forbid private tags
        element->getVR() != EVR_SQ)
    {
      AnswerDicomField(output, *element, transferSyntax);
    }
  }

  void ParsedDicomFile::SendPathValue(RestApiOutput& output,
                                      const UriComponents& uri)
  {
    DcmItem* dicom = pimpl_->file_->getDataset();
    E_TransferSyntax transferSyntax = pimpl_->file_->getDataset()->getOriginalXfer();

    // Special case: Accessing the pixel data
    if (uri.size() == 1 || 
        uri.size() == 2)
    {
      DcmTagKey tag;
      ParseTagAndGroup(tag, uri[0]);

      if (tag.getGroup() == DICOM_TAG_PIXEL_DATA.GetGroup() &&
          tag.getElement() == DICOM_TAG_PIXEL_DATA.GetElement())
      {
        AnswerPixelData(output, *dicom, transferSyntax, uri.size() == 1 ? NULL : &uri[1]);
        return;
      }
    }        

    // Go down in the tag hierarchy according to the URI
    for (size_t pos = 0; pos < uri.size() / 2; pos++)
    {
      size_t index;
      try
      {
        index = boost::lexical_cast<size_t>(uri[2 * pos + 1]);
      }
      catch (boost::bad_lexical_cast&)
      {
        return;
      }

      DcmTagKey k;
      DcmItem *child = NULL;
      ParseTagAndGroup(k, uri[2 * pos]);
      if (!dicom->findAndGetSequenceItem(k, child, index).good() ||
          child == NULL)
      {
        return;
      }

      dicom = child;
    }

    // We have reached the end of the URI
    if (uri.size() % 2 == 0)
    {
      SendPathValueForDictionary(output, *dicom);
    }
    else
    {
      SendPathValueForLeaf(output, uri.back(), *dicom, transferSyntax);
    }
  }


  


  static DcmElement* CreateElementForTag(const DicomTag& tag)
  {
    DcmTag key(tag.GetGroup(), tag.GetElement());

    switch (key.getEVR())
    {
      // http://support.dcmtk.org/docs/dcvr_8h-source.html

      /**
       * TODO.
       **/
    
      case EVR_OB:  // other byte
      case EVR_OF:  // other float
      case EVR_OW:  // other word
      case EVR_AT:  // attribute tag
        throw OrthancException(ErrorCode_NotImplemented);

      case EVR_UN:  // unknown value representation
        throw OrthancException(ErrorCode_ParameterOutOfRange);


      /**
       * String types.
       * http://support.dcmtk.org/docs/classDcmByteString.html
       **/
      
      case EVR_AS:  // age string
        return new DcmAgeString(key);

      case EVR_AE:  // application entity title
        return new DcmApplicationEntity(key);

      case EVR_CS:  // code string
        return new DcmCodeString(key);        

      case EVR_DA:  // date string
        return new DcmDate(key);
        
      case EVR_DT:  // date time string
        return new DcmDateTime(key);

      case EVR_DS:  // decimal string
        return new DcmDecimalString(key);

      case EVR_IS:  // integer string
        return new DcmIntegerString(key);

      case EVR_TM:  // time string
        return new DcmTime(key);

      case EVR_UI:  // unique identifier
        return new DcmUniqueIdentifier(key);

      case EVR_ST:  // short text
        return new DcmShortText(key);

      case EVR_LO:  // long string
        return new DcmLongString(key);

      case EVR_LT:  // long text
        return new DcmLongText(key);

      case EVR_UT:  // unlimited text
        return new DcmUnlimitedText(key);

      case EVR_SH:  // short string
        return new DcmShortString(key);

      case EVR_PN:  // person name
        return new DcmPersonName(key);

        
      /**
       * Numerical types
       **/ 
      
      case EVR_SL:  // signed long
        return new DcmSignedLong(key);

      case EVR_SS:  // signed short
        return new DcmSignedShort(key);

      case EVR_UL:  // unsigned long
        return new DcmUnsignedLong(key);

      case EVR_US:  // unsigned short
        return new DcmUnsignedShort(key);

      case EVR_FL:  // float single-precision
        return new DcmFloatingPointSingle(key);

      case EVR_FD:  // float double-precision
        return new DcmFloatingPointDouble(key);


      /**
       * Sequence types, should never occur at this point.
       **/

      case EVR_SQ:  // sequence of items
        throw OrthancException(ErrorCode_ParameterOutOfRange);


      /**
       * Internal to DCMTK.
       **/ 

      case EVR_ox:  // OB or OW depending on context
      case EVR_xs:  // SS or US depending on context
      case EVR_lt:  // US, SS or OW depending on context, used for LUT Data (thus the name)
      case EVR_na:  // na="not applicable", for data which has no VR
      case EVR_up:  // up="unsigned pointer", used internally for DICOMDIR suppor
      case EVR_item:  // used internally for items
      case EVR_metainfo:  // used internally for meta info datasets
      case EVR_dataset:  // used internally for datasets
      case EVR_fileFormat:  // used internally for DICOM files
      case EVR_dicomDir:  // used internally for DICOMDIR objects
      case EVR_dirRecord:  // used internally for DICOMDIR records
      case EVR_pixelSQ:  // used internally for pixel sequences in a compressed image
      case EVR_pixelItem:  // used internally for pixel items in a compressed image
      case EVR_UNKNOWN: // used internally for elements with unknown VR (encoded with 4-byte length field in explicit VR)
      case EVR_PixelData:  // used internally for uncompressed pixeld data
      case EVR_OverlayData:  // used internally for overlay data
      case EVR_UNKNOWN2B:  // used internally for elements with unknown VR with 2-byte length field in explicit VR
      default:
        break;
    }

    throw OrthancException(ErrorCode_InternalError);          
  }



  static void FillElementWithString(DcmElement& element,
                                    const DicomTag& tag,
                                    const std::string& value)
  {
    DcmTag key(tag.GetGroup(), tag.GetElement());
    bool ok = false;
    
    try
    {
      switch (key.getEVR())
      {
        // http://support.dcmtk.org/docs/dcvr_8h-source.html

        /**
         * TODO.
         **/

        case EVR_OB:  // other byte
        case EVR_OF:  // other float
        case EVR_OW:  // other word
        case EVR_AT:  // attribute tag
          throw OrthancException(ErrorCode_NotImplemented);
    
        case EVR_UN:  // unknown value representation
          throw OrthancException(ErrorCode_ParameterOutOfRange);


        /**
         * String types.
         **/
      
        case EVR_DS:  // decimal string
        case EVR_IS:  // integer string
        case EVR_AS:  // age string
        case EVR_DA:  // date string
        case EVR_DT:  // date time string
        case EVR_TM:  // time string
        case EVR_AE:  // application entity title
        case EVR_CS:  // code string
        case EVR_SH:  // short string
        case EVR_LO:  // long string
        case EVR_ST:  // short text
        case EVR_LT:  // long text
        case EVR_UT:  // unlimited text
        case EVR_PN:  // person name
        case EVR_UI:  // unique identifier
        {
          ok = element.putString(value.c_str()).good();
          break;
        }

        
        /**
         * Numerical types
         **/ 
      
        case EVR_SL:  // signed long
        {
          ok = element.putSint32(boost::lexical_cast<Sint32>(value)).good();
          break;
        }

        case EVR_SS:  // signed short
        {
          ok = element.putSint16(boost::lexical_cast<Sint16>(value)).good();
          break;
        }

        case EVR_UL:  // unsigned long
        {
          ok = element.putUint32(boost::lexical_cast<Uint32>(value)).good();
          break;
        }

        case EVR_US:  // unsigned short
        {
          ok = element.putUint16(boost::lexical_cast<Uint16>(value)).good();
          break;
        }

        case EVR_FL:  // float single-precision
        {
          ok = element.putFloat32(boost::lexical_cast<float>(value)).good();
          break;
        }

        case EVR_FD:  // float double-precision
        {
          ok = element.putFloat64(boost::lexical_cast<double>(value)).good();
          break;
        }


        /**
         * Sequence types, should never occur at this point.
         **/

        case EVR_SQ:  // sequence of items
        {
          ok = false;
          break;
        }


        /**
         * Internal to DCMTK.
         **/ 

        case EVR_ox:  // OB or OW depending on context
        case EVR_xs:  // SS or US depending on context
        case EVR_lt:  // US, SS or OW depending on context, used for LUT Data (thus the name)
        case EVR_na:  // na="not applicable", for data which has no VR
        case EVR_up:  // up="unsigned pointer", used internally for DICOMDIR suppor
        case EVR_item:  // used internally for items
        case EVR_metainfo:  // used internally for meta info datasets
        case EVR_dataset:  // used internally for datasets
        case EVR_fileFormat:  // used internally for DICOM files
        case EVR_dicomDir:  // used internally for DICOMDIR objects
        case EVR_dirRecord:  // used internally for DICOMDIR records
        case EVR_pixelSQ:  // used internally for pixel sequences in a compressed image
        case EVR_pixelItem:  // used internally for pixel items in a compressed image
        case EVR_UNKNOWN: // used internally for elements with unknown VR (encoded with 4-byte length field in explicit VR)
        case EVR_PixelData:  // used internally for uncompressed pixeld data
        case EVR_OverlayData:  // used internally for overlay data
        case EVR_UNKNOWN2B:  // used internally for elements with unknown VR with 2-byte length field in explicit VR
        default:
          break;
      }
    }
    catch (boost::bad_lexical_cast&)
    {
      ok = false;
    }

    if (!ok)
    {
      throw OrthancException(ErrorCode_InternalError);
    }
  }


  void ParsedDicomFile::Remove(const DicomTag& tag)
  {
    DcmTagKey key(tag.GetGroup(), tag.GetElement());
    DcmElement* element = pimpl_->file_->getDataset()->remove(key);
    if (element != NULL)
    {
      delete element;
    }
  }



  void ParsedDicomFile::RemovePrivateTags()
  {
    typedef std::list<DcmElement*> Tags;

    Tags privateTags;

    DcmDataset& dataset = *pimpl_->file_->getDataset();
    for (unsigned long i = 0; i < dataset.card(); i++)
    {
      DcmElement* element = dataset.getElement(i);
      DcmTag tag(element->getTag());
      if (!strcmp("PrivateCreator", tag.getTagName()) ||  // TODO - This may change with future versions of DCMTK
          tag.getPrivateCreator() != NULL)
      {
        privateTags.push_back(element);
      }
    }

    for (Tags::iterator it = privateTags.begin(); 
         it != privateTags.end(); ++it)
    {
      DcmElement* tmp = dataset.remove(*it);
      if (tmp != NULL)
      {
        delete tmp;
      }
    }
  }



  void ParsedDicomFile::Insert(const DicomTag& tag,
                               const std::string& value)
  {
    std::auto_ptr<DcmElement> element(CreateElementForTag(tag));
    FillElementWithString(*element, tag, value);

    if (!pimpl_->file_->getDataset()->insert(element.release(), false, false).good())
    {
      // This field already exists
      throw OrthancException(ErrorCode_InternalError);
    }
  }


  void ParsedDicomFile::Replace(const DicomTag& tag,
                                const std::string& value,
                                DicomReplaceMode mode)
  {
    DcmTagKey key(tag.GetGroup(), tag.GetElement());
    DcmElement* element = NULL;

    if (!pimpl_->file_->getDataset()->findAndGetElement(key, element).good() ||
        element == NULL)
    {
      // This field does not exist, act wrt. the specified "mode"
      switch (mode)
      {
        case DicomReplaceMode_InsertIfAbsent:
          Insert(tag, value);
          break;

        case DicomReplaceMode_ThrowIfAbsent:
          throw OrthancException(ErrorCode_InexistentItem);

        case DicomReplaceMode_IgnoreIfAbsent:
          return;
      }
    }
    else
    {
      FillElementWithString(*element, tag, value);
    }


    /**
     * dcmodify will automatically correct 'Media Storage SOP Class
     * UID' and 'Media Storage SOP Instance UID' in the metaheader, if
     * you make changes to the related tags in the dataset ('SOP Class
     * UID' and 'SOP Instance UID') via insert or modify mode
     * options. You can disable this behaviour by using the -nmu
     * option.
     **/

    if (tag == DICOM_TAG_SOP_CLASS_UID)
    {
      Replace(DICOM_TAG_MEDIA_STORAGE_SOP_CLASS_UID, value, DicomReplaceMode_InsertIfAbsent);
    }

    if (tag == DICOM_TAG_SOP_INSTANCE_UID)
    {
      Replace(DICOM_TAG_MEDIA_STORAGE_SOP_INSTANCE_UID, value, DicomReplaceMode_InsertIfAbsent);
    }
  }

    
  void ParsedDicomFile::Answer(RestApiOutput& output)
  {
    std::string serialized;
    if (FromDcmtkBridge::SaveToMemoryBuffer(serialized, pimpl_->file_->getDataset()))
    {
      output.AnswerBuffer(serialized, CONTENT_TYPE_OCTET_STREAM);
    }
  }



  bool ParsedDicomFile::GetTagValue(std::string& value,
                                    const DicomTag& tag)
  {
    DcmTagKey k(tag.GetGroup(), tag.GetElement());
    DcmDataset& dataset = *pimpl_->file_->getDataset();
    DcmElement* element = NULL;
    if (!dataset.findAndGetElement(k, element).good() ||
        element == NULL)
    {
      return false;
    }

    std::auto_ptr<DicomValue> v(FromDcmtkBridge::ConvertLeafElement(*element));

    if (v.get() == NULL)
    {
      value = "";
    }
    else
    {
      value = v->AsString();
    }

    return true;
  }



  DicomInstanceHasher ParsedDicomFile::GetHasher()
  {
    std::string patientId, studyUid, seriesUid, instanceUid;

    if (!GetTagValue(patientId, DICOM_TAG_PATIENT_ID) ||
        !GetTagValue(studyUid, DICOM_TAG_STUDY_INSTANCE_UID) ||
        !GetTagValue(seriesUid, DICOM_TAG_SERIES_INSTANCE_UID) ||
        !GetTagValue(instanceUid, DICOM_TAG_SOP_INSTANCE_UID))
    {
      throw OrthancException(ErrorCode_BadFileFormat);
    }

    return DicomInstanceHasher(patientId, studyUid, seriesUid, instanceUid);
  }


  static void StoreElement(Json::Value& target,
                           DcmElement& element,
                           unsigned int maxStringLength);

  static void StoreItem(Json::Value& target,
                        DcmItem& item,
                        unsigned int maxStringLength)
  {
    target = Json::Value(Json::objectValue);

    for (unsigned long i = 0; i < item.card(); i++)
    {
      DcmElement* element = item.getElement(i);
      StoreElement(target, *element, maxStringLength);
    }
  }


  static void StoreElement(Json::Value& target,
                           DcmElement& element,
                           unsigned int maxStringLength)
  {
    assert(target.type() == Json::objectValue);

    DicomTag tag(FromDcmtkBridge::GetTag(element));
    const std::string formattedTag = tag.Format();

#if 0
    const std::string tagName = FromDcmtkBridge::GetName(tag);
#else
    // This version of the code gives access to the name of the private tags
    DcmTag tagbis(element.getTag());
    const std::string tagName(tagbis.getTagName());      
#endif

    if (element.isLeaf())
    {
      Json::Value value(Json::objectValue);
      value["Name"] = tagName;

      if (tagbis.getPrivateCreator() != NULL)
      {
        value["PrivateCreator"] = tagbis.getPrivateCreator();
      }

      std::auto_ptr<DicomValue> v(FromDcmtkBridge::ConvertLeafElement(element));
      if (v->IsNull())
      {
        value["Type"] = "Null";
        value["Value"] = Json::nullValue;
      }
      else
      {
        std::string s = v->AsString();
        if (maxStringLength == 0 ||
            s.size() <= maxStringLength)
        {
          value["Type"] = "String";
          value["Value"] = s;
        }
        else
        {
          value["Type"] = "TooLong";
          value["Value"] = Json::nullValue;
        }
      }

      target[formattedTag] = value;
    }
    else
    {
      Json::Value children(Json::arrayValue);

      // "All subclasses of DcmElement except for DcmSequenceOfItems
      // are leaf nodes, while DcmSequenceOfItems, DcmItem, DcmDataset
      // etc. are not." The following cast is thus OK.
      DcmSequenceOfItems& sequence = dynamic_cast<DcmSequenceOfItems&>(element);

      for (unsigned long i = 0; i < sequence.card(); i++)
      {
        DcmItem* child = sequence.getItem(i);
        Json::Value& v = children.append(Json::objectValue);
        StoreItem(v, *child, maxStringLength);
      }  

      target[formattedTag]["Name"] = tagName;
      target[formattedTag]["Type"] = "Sequence";
      target[formattedTag]["Value"] = children;
    }
  }


  template <typename T>
  static void ExtractPngImageTruncate(std::string& result,
                                      DicomIntegerPixelAccessor& accessor,
                                      PixelFormat format)
  {
    assert(accessor.GetChannelCount() == 1);

    PngWriter w;

    std::vector<T> image(accessor.GetWidth() * accessor.GetHeight(), 0);
    T* pixel = &image[0];
    for (unsigned int y = 0; y < accessor.GetHeight(); y++)
    {
      for (unsigned int x = 0; x < accessor.GetWidth(); x++, pixel++)
      {
        int32_t v = accessor.GetValue(x, y);
        if (v < static_cast<int32_t>(std::numeric_limits<T>::min()))
          *pixel = std::numeric_limits<T>::min();
        else if (v > static_cast<int32_t>(std::numeric_limits<T>::max()))
          *pixel = std::numeric_limits<T>::max();
        else
          *pixel = static_cast<T>(v);
      }
    }

    w.WriteToMemory(result, accessor.GetWidth(), accessor.GetHeight(),
                    accessor.GetWidth() * sizeof(T), format, &image[0]);
  }


  void ParsedDicomFile::SaveToMemoryBuffer(std::string& buffer)
  {
    FromDcmtkBridge::SaveToMemoryBuffer(buffer, pimpl_->file_->getDataset());
  }


  void ParsedDicomFile::SaveToFile(const std::string& path)
  {
    // TODO Avoid using a temporary memory buffer, write directly on disk
    std::string content;
    SaveToMemoryBuffer(content);
    Toolbox::WriteFile(content, path);
  }


  ParsedDicomFile::ParsedDicomFile() : pimpl_(new PImpl)
  {
    pimpl_->file_.reset(new DcmFileFormat);
    Replace(DICOM_TAG_PATIENT_ID, FromDcmtkBridge::GenerateUniqueIdentifier(ResourceType_Patient));
    Replace(DICOM_TAG_STUDY_INSTANCE_UID, FromDcmtkBridge::GenerateUniqueIdentifier(ResourceType_Study));
    Replace(DICOM_TAG_SERIES_INSTANCE_UID, FromDcmtkBridge::GenerateUniqueIdentifier(ResourceType_Series));
    Replace(DICOM_TAG_SOP_INSTANCE_UID, FromDcmtkBridge::GenerateUniqueIdentifier(ResourceType_Instance));
  }


  ParsedDicomFile::ParsedDicomFile(const char* content, size_t size) : pimpl_(new PImpl)
  {
    Setup(content, size);
  }

  ParsedDicomFile::ParsedDicomFile(const std::string& content) : pimpl_(new PImpl)
  {
    if (content.size() == 0)
    {
      Setup(NULL, 0);
    }
    else
    {
      Setup(&content[0], content.size());
    }
  }


  ParsedDicomFile::ParsedDicomFile(ParsedDicomFile& other) : 
    pimpl_(new PImpl)
  {
    pimpl_->file_.reset(dynamic_cast<DcmFileFormat*>(other.pimpl_->file_->clone()));
  }


  ParsedDicomFile::~ParsedDicomFile()
  {
    delete pimpl_;
  }


  void* ParsedDicomFile::GetDcmtkObject()
  {
    return pimpl_->file_.get();
  }


  ParsedDicomFile* ParsedDicomFile::Clone()
  {
    return new ParsedDicomFile(*this);
  }
}
