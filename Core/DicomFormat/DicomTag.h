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

#include <string>
#include <stdint.h>


namespace Orthanc
{
  class DicomTag
  {
    // This must stay a POD (plain old data structure) 

  private:
    uint16_t group_;
    uint16_t element_;

  public:
    DicomTag(uint16_t group,
             uint16_t element) :
      group_(group),
      element_(element)
    {
    }

    uint16_t GetGroup() const
    {
      return group_;
    }

    uint16_t GetElement() const
    {
      return element_;
    }

    const char* GetMainTagsName() const;

    bool operator< (const DicomTag& other) const;

    bool operator== (const DicomTag& other) const
    {
      return group_ == other.group_ && element_ == other.element_;
    }

    bool operator!= (const DicomTag& other) const
    {
      return !(*this == other);
    }

    std::string Format() const;

    friend std::ostream& operator<< (std::ostream& o, const DicomTag& tag);
  };

  // Aliases for the most useful tags
  static const DicomTag DICOM_TAG_ACCESSION_NUMBER(0x0008, 0x0050);
  static const DicomTag DICOM_TAG_SOP_INSTANCE_UID(0x0008, 0x0018);
  static const DicomTag DICOM_TAG_PATIENT_ID(0x0010, 0x0020);
  static const DicomTag DICOM_TAG_SERIES_INSTANCE_UID(0x0020, 0x000e);
  static const DicomTag DICOM_TAG_STUDY_INSTANCE_UID(0x0020, 0x000d);
  static const DicomTag DICOM_TAG_PIXEL_DATA(0x7fe0, 0x0010);

  static const DicomTag DICOM_TAG_IMAGE_INDEX(0x0054, 0x1330);
  static const DicomTag DICOM_TAG_INSTANCE_NUMBER(0x0020, 0x0013);

  static const DicomTag DICOM_TAG_NUMBER_OF_SLICES(0x0054, 0x0081);
  static const DicomTag DICOM_TAG_NUMBER_OF_TIME_SLICES(0x0054, 0x0101);
  static const DicomTag DICOM_TAG_NUMBER_OF_FRAMES(0x0028, 0x0008);
  static const DicomTag DICOM_TAG_CARDIAC_NUMBER_OF_IMAGES(0x0018, 0x1090);
  static const DicomTag DICOM_TAG_IMAGES_IN_ACQUISITION(0x0020, 0x1002);

  static const DicomTag DICOM_TAG_PATIENT_NAME(0x0010, 0x0010);

  // The following is used for "modify" operations
  static const DicomTag DICOM_TAG_SOP_CLASS_UID(0x0008, 0x0016);
  static const DicomTag DICOM_TAG_MEDIA_STORAGE_SOP_CLASS_UID(0x0002, 0x0002);
  static const DicomTag DICOM_TAG_MEDIA_STORAGE_SOP_INSTANCE_UID(0x0002, 0x0003);

  // DICOM tags used for fMRI (thanks to Will Ryder)
  static const DicomTag DICOM_TAG_NUMBER_OF_TEMPORAL_POSITIONS(0x0020, 0x0105);
  static const DicomTag DICOM_TAG_TEMPORAL_POSITION_IDENTIFIER(0x0020, 0x0100);

  // Tags for C-FIND and C-MOVE
  static const DicomTag DICOM_TAG_SPECIFIC_CHARACTER_SET(0x0008, 0x0005);
  static const DicomTag DICOM_TAG_QUERY_RETRIEVE_LEVEL(0x0008, 0x0052);
  static const DicomTag DICOM_TAG_MODALITIES_IN_STUDY(0x0008, 0x0061);
}
