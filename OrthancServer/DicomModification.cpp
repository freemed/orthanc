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


#include "DicomModification.h"

#include "../Core/OrthancException.h"

namespace Orthanc
{
  void DicomModification::MapDicomIdentifier(ParsedDicomFile& dicom,
                                             DicomRootLevel level)
  {
    std::auto_ptr<DicomTag> tag;

    switch (level)
    {
      case DicomRootLevel_Study:
        tag.reset(new DicomTag(DICOM_TAG_STUDY_INSTANCE_UID));
        break;

      case DicomRootLevel_Series:
        tag.reset(new DicomTag(DICOM_TAG_SERIES_INSTANCE_UID));
        break;

      case DicomRootLevel_Instance:
        tag.reset(new DicomTag(DICOM_TAG_SOP_INSTANCE_UID));
        break;

      default:
        throw OrthancException(ErrorCode_InternalError);
    }

    std::string original;
    if (!dicom.GetTagValue(original, *tag))
    {
      original = "";
    }

    std::string mapped;

    UidMap::const_iterator previous = uidMap_.find(std::make_pair(level, original));
    if (previous == uidMap_.end())
    {
      mapped = FromDcmtkBridge::GenerateUniqueIdentifier(level);
      uidMap_.insert(std::make_pair(std::make_pair(level, original), mapped));
    }
    else
    {
      mapped = previous->second;
    }    

    dicom.Replace(*tag, mapped);
  }

  DicomModification::DicomModification()
  {
    removePrivateTags_ = false;
    level_ = DicomRootLevel_Instance;
  }

  void DicomModification::Keep(const DicomTag& tag)
  {
    removals_.erase(tag);
    replacements_.erase(tag);
  }

  void DicomModification::Remove(const DicomTag& tag)
  {
    removals_.insert(tag);
    replacements_.erase(tag);
  }

  bool DicomModification::IsRemoved(const DicomTag& tag) const
  {
    return removals_.find(tag) != removals_.end();
  }

  void DicomModification::Replace(const DicomTag& tag,
                                  const std::string& value)
  {
    removals_.erase(tag);
    replacements_[tag] = value;
  }

  bool DicomModification::IsReplaced(const DicomTag& tag) const
  {
    return replacements_.find(tag) != replacements_.end();
  }

  const std::string& DicomModification::GetReplacement(const DicomTag& tag) const
  {
    Replacements::const_iterator it = replacements_.find(tag);

    if (it == replacements_.end())
    {
      throw OrthancException(ErrorCode_InexistentItem);
    }
    else
    {
      return it->second;
    } 
  }

  void DicomModification::SetRemovePrivateTags(bool removed)
  {
    removePrivateTags_ = removed;
  }

  void DicomModification::SetLevel(DicomRootLevel level)
  {
    uidMap_.clear();
    level_ = level;
  }

  void DicomModification::SetupAnonymization()
  {
    removals_.clear();
    replacements_.clear();
    removePrivateTags_ = true;
    level_ = DicomRootLevel_Patient;
    uidMap_.clear();

    // This is Table E.1-1 from PS 3.15-2008 - DICOM Part 15: Security and System Management Profiles
    removals_.insert(DicomTag(0x0008, 0x0014));  // Instance Creator UID
    //removals_.insert(DicomTag(0x0008, 0x0018));  // SOP Instance UID => set in Apply()
    removals_.insert(DicomTag(0x0008, 0x0050));  // Accession Number
    removals_.insert(DicomTag(0x0008, 0x0080));  // Institution Name
    removals_.insert(DicomTag(0x0008, 0x0081));  // Institution Address
    removals_.insert(DicomTag(0x0008, 0x0090));  // Referring Physician's Name 
    removals_.insert(DicomTag(0x0008, 0x0092));  // Referring Physician's Address 
    removals_.insert(DicomTag(0x0008, 0x0094));  // Referring Physician's Telephone Numbers 
    removals_.insert(DicomTag(0x0008, 0x1010));  // Station Name 
    removals_.insert(DicomTag(0x0008, 0x1030));  // Study Description 
    removals_.insert(DicomTag(0x0008, 0x103e));  // Series Description 
    removals_.insert(DicomTag(0x0008, 0x1040));  // Institutional Department Name 
    removals_.insert(DicomTag(0x0008, 0x1048));  // Physician(s) of Record 
    removals_.insert(DicomTag(0x0008, 0x1050));  // Performing Physicians' Name 
    removals_.insert(DicomTag(0x0008, 0x1060));  // Name of Physician(s) Reading Study 
    removals_.insert(DicomTag(0x0008, 0x1070));  // Operators' Name 
    removals_.insert(DicomTag(0x0008, 0x1080));  // Admitting Diagnoses Description 
    removals_.insert(DicomTag(0x0008, 0x1155));  // Referenced SOP Instance UID 
    removals_.insert(DicomTag(0x0008, 0x2111));  // Derivation Description 
    //removals_.insert(DicomTag(0x0010, 0x0010));  // Patient's Name => cf. below (*)
    //removals_.insert(DicomTag(0x0010, 0x0020));  // Patient ID => cf. below (*)
    removals_.insert(DicomTag(0x0010, 0x0030));  // Patient's Birth Date 
    removals_.insert(DicomTag(0x0010, 0x0032));  // Patient's Birth Time 
    removals_.insert(DicomTag(0x0010, 0x0040));  // Patient's Sex 
    removals_.insert(DicomTag(0x0010, 0x1000));  // Other Patient Ids 
    removals_.insert(DicomTag(0x0010, 0x1001));  // Other Patient Names 
    removals_.insert(DicomTag(0x0010, 0x1010));  // Patient's Age 
    removals_.insert(DicomTag(0x0010, 0x1020));  // Patient's Size 
    removals_.insert(DicomTag(0x0010, 0x1030));  // Patient's Weight 
    removals_.insert(DicomTag(0x0010, 0x1090));  // Medical Record Locator 
    removals_.insert(DicomTag(0x0010, 0x2160));  // Ethnic Group 
    removals_.insert(DicomTag(0x0010, 0x2180));  // Occupation 
    removals_.insert(DicomTag(0x0010, 0x21b0));  // Additional Patient's History 
    removals_.insert(DicomTag(0x0010, 0x4000));  // Patient Comments 
    removals_.insert(DicomTag(0x0018, 0x1000));  // Device Serial Number 
    removals_.insert(DicomTag(0x0018, 0x1030));  // Protocol Name 
    //removals_.insert(DicomTag(0x0020, 0x000d));  // Study Instance UID => set in Apply()
    //removals_.insert(DicomTag(0x0020, 0x000e));  // Series Instance UID => set in Apply()
    removals_.insert(DicomTag(0x0020, 0x0010));  // Study ID 
    removals_.insert(DicomTag(0x0020, 0x0052));  // Frame of Reference UID 
    removals_.insert(DicomTag(0x0020, 0x0200));  // Synchronization Frame of Reference UID 
    removals_.insert(DicomTag(0x0020, 0x4000));  // Image Comments 
    removals_.insert(DicomTag(0x0040, 0x0275));  // Request Attributes Sequence 
    removals_.insert(DicomTag(0x0040, 0xa124));  // UID
    removals_.insert(DicomTag(0x0040, 0xa730));  // Content Sequence 
    removals_.insert(DicomTag(0x0088, 0x0140));  // Storage Media File-set UID 
    removals_.insert(DicomTag(0x3006, 0x0024));  // Referenced Frame of Reference UID 
    removals_.insert(DicomTag(0x3006, 0x00c2));  // Related Frame of Reference UID 

    // Some more removals (from the experience of DICOM files at the CHU of Liege)
    removals_.insert(DicomTag(0x0010, 0x1040));  // Patient's Address
    removals_.insert(DicomTag(0x0032, 0x1032));  // Requesting Physician
    removals_.insert(DicomTag(0x0010, 0x2154));  // PatientTelephoneNumbers
    removals_.insert(DicomTag(0x0010, 0x2000));  // Medical Alerts

    // Set the DeidentificationMethod tag
    replacements_.insert(std::make_pair(DicomTag(0x0012, 0x0063), "Orthanc " ORTHANC_VERSION " - PS 3.15-2008 Table E.1-1"));

    // Set the PatientIdentityRemoved tag
    replacements_.insert(std::make_pair(DicomTag(0x0012, 0x0062), "YES"));

    // (*) Choose a random patient name and ID
    std::string patientId = FromDcmtkBridge::GenerateUniqueIdentifier(DicomRootLevel_Patient);
    replacements_[DICOM_TAG_PATIENT_ID] = patientId;
    replacements_[DICOM_TAG_PATIENT_NAME] = patientId;
  }

  void DicomModification::Apply(ParsedDicomFile& toModify)
  {
    // Check the request
    assert(DicomRootLevel_Patient + 1 == DicomRootLevel_Study &&
           DicomRootLevel_Study + 1 == DicomRootLevel_Series &&
           DicomRootLevel_Series + 1 == DicomRootLevel_Instance);

    if (IsRemoved(DICOM_TAG_PATIENT_ID) ||
        IsRemoved(DICOM_TAG_STUDY_INSTANCE_UID) ||
        IsRemoved(DICOM_TAG_SERIES_INSTANCE_UID) ||
        IsRemoved(DICOM_TAG_SOP_INSTANCE_UID))
    {
      throw OrthancException(ErrorCode_BadRequest);
    }

    if (level_ == DicomRootLevel_Patient && !IsReplaced(DICOM_TAG_PATIENT_ID))
    {
      throw OrthancException(ErrorCode_BadRequest);
    }

    if (level_ > DicomRootLevel_Patient && IsReplaced(DICOM_TAG_PATIENT_ID))
    {
      throw OrthancException(ErrorCode_BadRequest);
    }

    if (level_ > DicomRootLevel_Study && IsReplaced(DICOM_TAG_STUDY_INSTANCE_UID))
    {
      throw OrthancException(ErrorCode_BadRequest);
    }

    if (level_ > DicomRootLevel_Series && IsReplaced(DICOM_TAG_SERIES_INSTANCE_UID))
    {
      throw OrthancException(ErrorCode_BadRequest);
    }

    // (1) Remove the private tags, if need be
    if (removePrivateTags_)
    {
      toModify.RemovePrivateTags();
    }

    // (2) Remove the tags specified by the user
    for (Removals::const_iterator it = removals_.begin(); 
         it != removals_.end(); ++it)
    {
      toModify.Remove(*it);
    }

    // (3) Replace the tags
    for (Replacements::const_iterator it = replacements_.begin(); 
         it != replacements_.end(); ++it)
    {
      toModify.Replace(it->first, it->second, DicomReplaceMode_InsertIfAbsent);
    }

    // (4) Update the DICOM identifiers
    if (level_ <= DicomRootLevel_Study)
    {
      MapDicomIdentifier(toModify, DicomRootLevel_Study);
    }

    if (level_ <= DicomRootLevel_Series)
    {
      MapDicomIdentifier(toModify, DicomRootLevel_Series);
    }

    if (level_ <= DicomRootLevel_Instance)  // Always true
    {
      MapDicomIdentifier(toModify, DicomRootLevel_Instance);
    }
  }
}
