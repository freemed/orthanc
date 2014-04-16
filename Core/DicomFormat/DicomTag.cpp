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


#include "DicomTag.h"

#include "../OrthancException.h"

#include <iostream>
#include <iomanip>
#include <stdio.h>

namespace Orthanc
{
  bool DicomTag::operator< (const DicomTag& other) const
  {
    if (group_ < other.group_)
      return true;

    if (group_ > other.group_)
      return false;

    return element_ < other.element_;
  }


  std::ostream& operator<< (std::ostream& o, const DicomTag& tag)
  {
    using namespace std;
    ios_base::fmtflags state = o.flags();
    o.flags(ios::right | ios::hex);
    o << "(" << setfill('0') << setw(4) << tag.GetGroup()
      << "," << setw(4) << tag.GetElement() << ")";
    o.flags(state);
    return o;
  }


  std::string DicomTag::Format() const
  {
    char b[16];
    sprintf(b, "%04x,%04x", group_, element_);
    return std::string(b);
  }


  const char* DicomTag::GetMainTagsName() const
  {
    if (*this == DICOM_TAG_ACCESSION_NUMBER)
      return "AccessionNumber";

    if (*this == DICOM_TAG_SOP_INSTANCE_UID)
      return "SOPInstanceUID";

    if (*this == DICOM_TAG_PATIENT_ID)
      return "PatientID";

    if (*this == DICOM_TAG_SERIES_INSTANCE_UID)
      return "SeriesInstanceUID";

    if (*this == DICOM_TAG_STUDY_INSTANCE_UID)
      return "StudyInstanceUID"; 

    if (*this == DICOM_TAG_PIXEL_DATA)
      return "PixelData";

    if (*this == DICOM_TAG_IMAGE_INDEX)
      return "ImageIndex";

    if (*this == DICOM_TAG_INSTANCE_NUMBER)
      return "InstanceNumber";

    if (*this == DICOM_TAG_NUMBER_OF_SLICES)
      return "NumberOfSlices";

    if (*this == DICOM_TAG_NUMBER_OF_FRAMES)
      return "NumberOfFrames";

    if (*this == DICOM_TAG_CARDIAC_NUMBER_OF_IMAGES)
      return "CardiacNumberOfImages";

    if (*this == DICOM_TAG_IMAGES_IN_ACQUISITION)
      return "ImagesInAcquisition";

    if (*this == DICOM_TAG_PATIENT_NAME)
      return "PatientName";

    return "";
  }
}
