#include "gtest/gtest.h"

#include "../OrthancServer/DatabaseWrapper.h"
#include "../OrthancServer/ServerContext.h"
#include "../OrthancServer/ServerIndex.h"
#include "../Core/Uuid.h"
#include "../Core/DicomFormat/DicomNullValue.h"

#include <ctype.h>
#include <glog/logging.h>
#include <algorithm>

using namespace Orthanc;

namespace
{
  enum DatabaseWrapperClass
  {
    DatabaseWrapperClass_SQLite
  };


  class ServerIndexListener : public IServerIndexListener
  {
  public:
    std::vector<std::string> deletedFiles_;
    std::string ancestorId_;
    ResourceType ancestorType_;

    void Reset()
    {
      ancestorId_ = "";
      deletedFiles_.clear();
    }

    virtual void SignalRemainingAncestor(ResourceType type,
                                         const std::string& publicId) 
    {
      ancestorId_ = publicId;
      ancestorType_ = type;
    }

    virtual void SignalFileDeleted(const FileInfo& info)
    {
      const std::string fileUuid = info.GetUuid();
      deletedFiles_.push_back(fileUuid);
      LOG(INFO) << "A file must be removed: " << fileUuid;
    }                                
  };


  class DatabaseWrapperTest : public ::testing::TestWithParam<DatabaseWrapperClass>
  {
  protected:
    std::auto_ptr<ServerIndexListener> listener_;
    std::auto_ptr<DatabaseWrapper> index_;

    DatabaseWrapperTest()
    {
    }

    virtual void SetUp() 
    {
      listener_.reset(new ServerIndexListener);
      index_.reset(new DatabaseWrapper(*listener_));
    }

    virtual void TearDown()
    {
      index_.reset(NULL);
      listener_.reset(NULL);
    }
  };
}


INSTANTIATE_TEST_CASE_P(DatabaseWrapperName,
                        DatabaseWrapperTest,
                        ::testing::Values(DatabaseWrapperClass_SQLite));


TEST_P(DatabaseWrapperTest, Simple)
{
  int64_t a[] = {
    index_->CreateResource("a", ResourceType_Patient),   // 0
    index_->CreateResource("b", ResourceType_Study),     // 1
    index_->CreateResource("c", ResourceType_Series),    // 2
    index_->CreateResource("d", ResourceType_Instance),  // 3
    index_->CreateResource("e", ResourceType_Instance),  // 4
    index_->CreateResource("f", ResourceType_Instance),  // 5
    index_->CreateResource("g", ResourceType_Study)      // 6
  };

  ASSERT_EQ("a", index_->GetPublicId(a[0]));
  ASSERT_EQ("b", index_->GetPublicId(a[1]));
  ASSERT_EQ("c", index_->GetPublicId(a[2]));
  ASSERT_EQ("d", index_->GetPublicId(a[3]));
  ASSERT_EQ("e", index_->GetPublicId(a[4]));
  ASSERT_EQ("f", index_->GetPublicId(a[5]));
  ASSERT_EQ("g", index_->GetPublicId(a[6]));

  ASSERT_EQ(ResourceType_Patient, index_->GetResourceType(a[0]));
  ASSERT_EQ(ResourceType_Study, index_->GetResourceType(a[1]));
  ASSERT_EQ(ResourceType_Series, index_->GetResourceType(a[2]));
  ASSERT_EQ(ResourceType_Instance, index_->GetResourceType(a[3]));
  ASSERT_EQ(ResourceType_Instance, index_->GetResourceType(a[4]));
  ASSERT_EQ(ResourceType_Instance, index_->GetResourceType(a[5]));
  ASSERT_EQ(ResourceType_Study, index_->GetResourceType(a[6]));

  {
    Json::Value t;
    index_->GetAllPublicIds(t, ResourceType_Patient);

    ASSERT_EQ(1u, t.size());
    ASSERT_EQ("a", t[0u].asString());

    index_->GetAllPublicIds(t, ResourceType_Series);
    ASSERT_EQ(1u, t.size());
    ASSERT_EQ("c", t[0u].asString());

    index_->GetAllPublicIds(t, ResourceType_Study);
    ASSERT_EQ(2u, t.size());

    index_->GetAllPublicIds(t, ResourceType_Instance);
    ASSERT_EQ(3u, t.size());
  }

  index_->SetGlobalProperty(GlobalProperty_FlushSleep, "World");

  index_->AttachChild(a[0], a[1]);
  index_->AttachChild(a[1], a[2]);
  index_->AttachChild(a[2], a[3]);
  index_->AttachChild(a[2], a[4]);
  index_->AttachChild(a[6], a[5]);

  int64_t parent;
  ASSERT_FALSE(index_->LookupParent(parent, a[0]));
  ASSERT_TRUE(index_->LookupParent(parent, a[1])); ASSERT_EQ(a[0], parent);
  ASSERT_TRUE(index_->LookupParent(parent, a[2])); ASSERT_EQ(a[1], parent);
  ASSERT_TRUE(index_->LookupParent(parent, a[3])); ASSERT_EQ(a[2], parent);
  ASSERT_TRUE(index_->LookupParent(parent, a[4])); ASSERT_EQ(a[2], parent);
  ASSERT_TRUE(index_->LookupParent(parent, a[5])); ASSERT_EQ(a[6], parent);
  ASSERT_FALSE(index_->LookupParent(parent, a[6]));

  std::string s;
  
  ASSERT_FALSE(index_->GetParentPublicId(s, a[0]));
  ASSERT_FALSE(index_->GetParentPublicId(s, a[6]));
  ASSERT_TRUE(index_->GetParentPublicId(s, a[1])); ASSERT_EQ("a", s);
  ASSERT_TRUE(index_->GetParentPublicId(s, a[2])); ASSERT_EQ("b", s);
  ASSERT_TRUE(index_->GetParentPublicId(s, a[3])); ASSERT_EQ("c", s);
  ASSERT_TRUE(index_->GetParentPublicId(s, a[4])); ASSERT_EQ("c", s);
  ASSERT_TRUE(index_->GetParentPublicId(s, a[5])); ASSERT_EQ("g", s);

  std::list<std::string> l;
  index_->GetChildrenPublicId(l, a[0]); ASSERT_EQ(1u, l.size()); ASSERT_EQ("b", l.front());
  index_->GetChildrenPublicId(l, a[1]); ASSERT_EQ(1u, l.size()); ASSERT_EQ("c", l.front());
  index_->GetChildrenPublicId(l, a[3]); ASSERT_EQ(0u, l.size()); 
  index_->GetChildrenPublicId(l, a[4]); ASSERT_EQ(0u, l.size()); 
  index_->GetChildrenPublicId(l, a[5]); ASSERT_EQ(0u, l.size()); 
  index_->GetChildrenPublicId(l, a[6]); ASSERT_EQ(1u, l.size()); ASSERT_EQ("f", l.front());

  index_->GetChildrenPublicId(l, a[2]); ASSERT_EQ(2u, l.size()); 
  if (l.front() == "d")
  {
    ASSERT_EQ("e", l.back());
  }
  else
  {
    ASSERT_EQ("d", l.back());
    ASSERT_EQ("e", l.front());
  }

  std::list<MetadataType> md;
  index_->ListAvailableMetadata(md, a[4]);
  ASSERT_EQ(0u, md.size());

  index_->AddAttachment(a[4], FileInfo("my json file", FileContentType_DicomAsJson, 42, "md5", 
                                     CompressionType_Zlib, 21, "compressedMD5"));
  index_->AddAttachment(a[4], FileInfo("my dicom file", FileContentType_Dicom, 42, "md5"));
  index_->AddAttachment(a[6], FileInfo("world", FileContentType_Dicom, 44, "md5"));
  index_->SetMetadata(a[4], MetadataType_Instance_RemoteAet, "PINNACLE");
  
  index_->ListAvailableMetadata(md, a[4]);
  ASSERT_EQ(1u, md.size());
  ASSERT_EQ(MetadataType_Instance_RemoteAet, md.front());
  index_->SetMetadata(a[4], MetadataType_ModifiedFrom, "TUTU");
  index_->ListAvailableMetadata(md, a[4]);
  ASSERT_EQ(2u, md.size());
  index_->DeleteMetadata(a[4], MetadataType_ModifiedFrom);
  index_->ListAvailableMetadata(md, a[4]);
  ASSERT_EQ(1u, md.size());
  ASSERT_EQ(MetadataType_Instance_RemoteAet, md.front());

  ASSERT_EQ(21u + 42u + 44u, index_->GetTotalCompressedSize());
  ASSERT_EQ(42u + 42u + 44u, index_->GetTotalUncompressedSize());

  DicomMap m;
  m.SetValue(0x0010, 0x0010, "PatientName");
  index_->SetMainDicomTags(a[3], m);

  int64_t b;
  ResourceType t;
  ASSERT_TRUE(index_->LookupResource("g", b, t));
  ASSERT_EQ(7, b);
  ASSERT_EQ(ResourceType_Study, t);

  ASSERT_TRUE(index_->LookupMetadata(s, a[4], MetadataType_Instance_RemoteAet));
  ASSERT_FALSE(index_->LookupMetadata(s, a[4], MetadataType_Instance_IndexInSeries));
  ASSERT_EQ("PINNACLE", s);
  ASSERT_EQ("PINNACLE", index_->GetMetadata(a[4], MetadataType_Instance_RemoteAet));
  ASSERT_EQ("None", index_->GetMetadata(a[4], MetadataType_Instance_IndexInSeries, "None"));

  ASSERT_TRUE(index_->LookupGlobalProperty(s, GlobalProperty_FlushSleep));
  ASSERT_FALSE(index_->LookupGlobalProperty(s, static_cast<GlobalProperty>(42)));
  ASSERT_EQ("World", s);
  ASSERT_EQ("World", index_->GetGlobalProperty(GlobalProperty_FlushSleep));
  ASSERT_EQ("None", index_->GetGlobalProperty(static_cast<GlobalProperty>(42), "None"));

  FileInfo att;
  ASSERT_TRUE(index_->LookupAttachment(att, a[4], FileContentType_DicomAsJson));
  ASSERT_EQ("my json file", att.GetUuid());
  ASSERT_EQ(21u, att.GetCompressedSize());
  ASSERT_EQ("md5", att.GetUncompressedMD5());
  ASSERT_EQ("compressedMD5", att.GetCompressedMD5());
  ASSERT_EQ(42u, att.GetUncompressedSize());
  ASSERT_EQ(CompressionType_Zlib, att.GetCompressionType());

  ASSERT_TRUE(index_->LookupAttachment(att, a[6], FileContentType_Dicom));
  ASSERT_EQ("world", att.GetUuid());
  ASSERT_EQ(44u, att.GetCompressedSize());
  ASSERT_EQ("md5", att.GetUncompressedMD5());
  ASSERT_EQ("md5", att.GetCompressedMD5());
  ASSERT_EQ(44u, att.GetUncompressedSize());
  ASSERT_EQ(CompressionType_None, att.GetCompressionType());

  ASSERT_EQ(0u, listener_->deletedFiles_.size());
  ASSERT_EQ(7u, index_->GetTableRecordCount("Resources")); 
  ASSERT_EQ(3u, index_->GetTableRecordCount("AttachedFiles"));
  ASSERT_EQ(1u, index_->GetTableRecordCount("Metadata"));
  ASSERT_EQ(1u, index_->GetTableRecordCount("MainDicomTags"));
  index_->DeleteResource(a[0]);

  ASSERT_EQ(2u, listener_->deletedFiles_.size());
  ASSERT_FALSE(std::find(listener_->deletedFiles_.begin(), 
                         listener_->deletedFiles_.end(),
                         "my json file") == listener_->deletedFiles_.end());
  ASSERT_FALSE(std::find(listener_->deletedFiles_.begin(), 
                         listener_->deletedFiles_.end(),
                         "my dicom file") == listener_->deletedFiles_.end());

  ASSERT_EQ(2u, index_->GetTableRecordCount("Resources"));
  ASSERT_EQ(0u, index_->GetTableRecordCount("Metadata"));
  ASSERT_EQ(1u, index_->GetTableRecordCount("AttachedFiles"));
  ASSERT_EQ(0u, index_->GetTableRecordCount("MainDicomTags"));
  index_->DeleteResource(a[5]);
  ASSERT_EQ(0u, index_->GetTableRecordCount("Resources"));
  ASSERT_EQ(0u, index_->GetTableRecordCount("AttachedFiles"));
  ASSERT_EQ(2u, index_->GetTableRecordCount("GlobalProperties"));

  ASSERT_EQ(3u, listener_->deletedFiles_.size());
  ASSERT_FALSE(std::find(listener_->deletedFiles_.begin(), 
                         listener_->deletedFiles_.end(),
                         "world") == listener_->deletedFiles_.end());
}




TEST_P(DatabaseWrapperTest, Upward)
{
  int64_t a[] = {
    index_->CreateResource("a", ResourceType_Patient),   // 0
    index_->CreateResource("b", ResourceType_Study),     // 1
    index_->CreateResource("c", ResourceType_Series),    // 2
    index_->CreateResource("d", ResourceType_Instance),  // 3
    index_->CreateResource("e", ResourceType_Instance),  // 4
    index_->CreateResource("f", ResourceType_Study),     // 5
    index_->CreateResource("g", ResourceType_Series),    // 6
    index_->CreateResource("h", ResourceType_Series)     // 7
  };

  index_->AttachChild(a[0], a[1]);
  index_->AttachChild(a[1], a[2]);
  index_->AttachChild(a[2], a[3]);
  index_->AttachChild(a[2], a[4]);
  index_->AttachChild(a[1], a[6]);
  index_->AttachChild(a[0], a[5]);
  index_->AttachChild(a[5], a[7]);

  {
    Json::Value j;
    index_->GetChildren(j, a[0]);
    ASSERT_EQ(2u, j.size());
    ASSERT_TRUE((j[0u] == "b" && j[1u] == "f") ||
                (j[1u] == "b" && j[0u] == "f"));

    index_->GetChildren(j, a[1]);
    ASSERT_EQ(2u, j.size());
    ASSERT_TRUE((j[0u] == "c" && j[1u] == "g") ||
                (j[1u] == "c" && j[0u] == "g"));

    index_->GetChildren(j, a[2]);
    ASSERT_EQ(2u, j.size());
    ASSERT_TRUE((j[0u] == "d" && j[1u] == "e") ||
                (j[1u] == "d" && j[0u] == "e"));

    index_->GetChildren(j, a[3]); ASSERT_EQ(0u, j.size());
    index_->GetChildren(j, a[4]); ASSERT_EQ(0u, j.size());
    index_->GetChildren(j, a[5]); ASSERT_EQ(1u, j.size()); ASSERT_EQ("h", j[0u].asString());
    index_->GetChildren(j, a[6]); ASSERT_EQ(0u, j.size());
    index_->GetChildren(j, a[7]); ASSERT_EQ(0u, j.size());
  }

  listener_->Reset();
  index_->DeleteResource(a[3]);
  ASSERT_EQ("c", listener_->ancestorId_);
  ASSERT_EQ(ResourceType_Series, listener_->ancestorType_);

  listener_->Reset();
  index_->DeleteResource(a[4]);
  ASSERT_EQ("b", listener_->ancestorId_);
  ASSERT_EQ(ResourceType_Study, listener_->ancestorType_);

  listener_->Reset();
  index_->DeleteResource(a[7]);
  ASSERT_EQ("a", listener_->ancestorId_);
  ASSERT_EQ(ResourceType_Patient, listener_->ancestorType_);

  listener_->Reset();
  index_->DeleteResource(a[6]);
  ASSERT_EQ("", listener_->ancestorId_);  // No more ancestor
}


TEST_P(DatabaseWrapperTest, PatientRecycling)
{
  std::vector<int64_t> patients;
  for (int i = 0; i < 10; i++)
  {
    std::string p = "Patient " + boost::lexical_cast<std::string>(i);
    patients.push_back(index_->CreateResource(p, ResourceType_Patient));
    index_->AddAttachment(patients[i], FileInfo(p, FileContentType_Dicom, i + 10, 
                                              "md5-" + boost::lexical_cast<std::string>(i)));
    ASSERT_FALSE(index_->IsProtectedPatient(patients[i]));
  }

  ASSERT_EQ(10u, index_->GetTableRecordCount("Resources")); 
  ASSERT_EQ(10u, index_->GetTableRecordCount("PatientRecyclingOrder")); 

  listener_->Reset();

  index_->DeleteResource(patients[5]);
  index_->DeleteResource(patients[0]);
  ASSERT_EQ(8u, index_->GetTableRecordCount("Resources")); 
  ASSERT_EQ(8u, index_->GetTableRecordCount("PatientRecyclingOrder"));

  ASSERT_EQ(2u, listener_->deletedFiles_.size());
  ASSERT_EQ("Patient 5", listener_->deletedFiles_[0]);
  ASSERT_EQ("Patient 0", listener_->deletedFiles_[1]);

  int64_t p;
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[1]);
  index_->DeleteResource(p);
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[2]);
  index_->DeleteResource(p);
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[3]);
  index_->DeleteResource(p);
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[4]);
  index_->DeleteResource(p);
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[6]);
  index_->DeleteResource(p);
  index_->DeleteResource(patients[8]);
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[7]);
  index_->DeleteResource(p);
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[9]);
  index_->DeleteResource(p);
  ASSERT_FALSE(index_->SelectPatientToRecycle(p));

  ASSERT_EQ(10u, listener_->deletedFiles_.size());
  ASSERT_EQ(0u, index_->GetTableRecordCount("Resources")); 
  ASSERT_EQ(0u, index_->GetTableRecordCount("PatientRecyclingOrder")); 
}


TEST_P(DatabaseWrapperTest, PatientProtection)
{
  std::vector<int64_t> patients;
  for (int i = 0; i < 5; i++)
  {
    std::string p = "Patient " + boost::lexical_cast<std::string>(i);
    patients.push_back(index_->CreateResource(p, ResourceType_Patient));
    index_->AddAttachment(patients[i], FileInfo(p, FileContentType_Dicom, i + 10,
                                              "md5-" + boost::lexical_cast<std::string>(i)));
    ASSERT_FALSE(index_->IsProtectedPatient(patients[i]));
  }

  ASSERT_EQ(5u, index_->GetTableRecordCount("Resources")); 
  ASSERT_EQ(5u, index_->GetTableRecordCount("PatientRecyclingOrder")); 

  ASSERT_FALSE(index_->IsProtectedPatient(patients[2]));
  index_->SetProtectedPatient(patients[2], true);
  ASSERT_TRUE(index_->IsProtectedPatient(patients[2]));
  ASSERT_EQ(4u, index_->GetTableRecordCount("PatientRecyclingOrder"));
  ASSERT_EQ(5u, index_->GetTableRecordCount("Resources")); 

  index_->SetProtectedPatient(patients[2], true);
  ASSERT_TRUE(index_->IsProtectedPatient(patients[2]));
  ASSERT_EQ(4u, index_->GetTableRecordCount("PatientRecyclingOrder")); 
  index_->SetProtectedPatient(patients[2], false);
  ASSERT_FALSE(index_->IsProtectedPatient(patients[2]));
  ASSERT_EQ(5u, index_->GetTableRecordCount("PatientRecyclingOrder")); 
  index_->SetProtectedPatient(patients[2], false);
  ASSERT_FALSE(index_->IsProtectedPatient(patients[2]));
  ASSERT_EQ(5u, index_->GetTableRecordCount("PatientRecyclingOrder")); 

  ASSERT_EQ(5u, index_->GetTableRecordCount("Resources")); 
  ASSERT_EQ(5u, index_->GetTableRecordCount("PatientRecyclingOrder")); 
  index_->SetProtectedPatient(patients[2], true);
  ASSERT_TRUE(index_->IsProtectedPatient(patients[2]));
  ASSERT_EQ(4u, index_->GetTableRecordCount("PatientRecyclingOrder"));
  index_->SetProtectedPatient(patients[2], false);
  ASSERT_FALSE(index_->IsProtectedPatient(patients[2]));
  ASSERT_EQ(5u, index_->GetTableRecordCount("PatientRecyclingOrder")); 
  index_->SetProtectedPatient(patients[3], true);
  ASSERT_TRUE(index_->IsProtectedPatient(patients[3]));
  ASSERT_EQ(4u, index_->GetTableRecordCount("PatientRecyclingOrder")); 

  ASSERT_EQ(5u, index_->GetTableRecordCount("Resources")); 
  ASSERT_EQ(0u, listener_->deletedFiles_.size());

  // Unprotecting a patient puts it at the last position in the recycling queue
  int64_t p;
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[0]);
  index_->DeleteResource(p);
  ASSERT_TRUE(index_->SelectPatientToRecycle(p, patients[1])); ASSERT_EQ(p, patients[4]);
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[1]);
  index_->DeleteResource(p);
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[4]);
  index_->DeleteResource(p);
  ASSERT_FALSE(index_->SelectPatientToRecycle(p, patients[2]));
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[2]);
  index_->DeleteResource(p);
  // "patients[3]" is still protected
  ASSERT_FALSE(index_->SelectPatientToRecycle(p));

  ASSERT_EQ(4u, listener_->deletedFiles_.size());
  ASSERT_EQ(1u, index_->GetTableRecordCount("Resources")); 
  ASSERT_EQ(0u, index_->GetTableRecordCount("PatientRecyclingOrder")); 

  index_->SetProtectedPatient(patients[3], false);
  ASSERT_EQ(1u, index_->GetTableRecordCount("PatientRecyclingOrder")); 
  ASSERT_FALSE(index_->SelectPatientToRecycle(p, patients[3]));
  ASSERT_TRUE(index_->SelectPatientToRecycle(p, patients[2]));
  ASSERT_TRUE(index_->SelectPatientToRecycle(p)); ASSERT_EQ(p, patients[3]);
  index_->DeleteResource(p);

  ASSERT_EQ(5u, listener_->deletedFiles_.size());
  ASSERT_EQ(0u, index_->GetTableRecordCount("Resources")); 
  ASSERT_EQ(0u, index_->GetTableRecordCount("PatientRecyclingOrder")); 
}



TEST_P(DatabaseWrapperTest, Sequence)
{
  ASSERT_EQ(1u, index_->IncrementGlobalSequence(GlobalProperty_AnonymizationSequence));
  ASSERT_EQ(2u, index_->IncrementGlobalSequence(GlobalProperty_AnonymizationSequence));
  ASSERT_EQ(3u, index_->IncrementGlobalSequence(GlobalProperty_AnonymizationSequence));
  ASSERT_EQ(4u, index_->IncrementGlobalSequence(GlobalProperty_AnonymizationSequence));
}



TEST_P(DatabaseWrapperTest, LookupTagValue)
{
  int64_t a[] = {
    index_->CreateResource("a", ResourceType_Study),   // 0
    index_->CreateResource("b", ResourceType_Study),   // 1
    index_->CreateResource("c", ResourceType_Study),   // 2
    index_->CreateResource("d", ResourceType_Series)   // 3
  };

  DicomMap m;
  m.Clear(); m.SetValue(DICOM_TAG_STUDY_INSTANCE_UID, "0"); index_->SetMainDicomTags(a[0], m);
  m.Clear(); m.SetValue(DICOM_TAG_STUDY_INSTANCE_UID, "1"); index_->SetMainDicomTags(a[1], m);
  m.Clear(); m.SetValue(DICOM_TAG_STUDY_INSTANCE_UID, "0"); index_->SetMainDicomTags(a[2], m);
  m.Clear(); m.SetValue(DICOM_TAG_SERIES_INSTANCE_UID, "0"); index_->SetMainDicomTags(a[3], m);

  std::list<int64_t> s;

  index_->LookupTagValue(s, DICOM_TAG_STUDY_INSTANCE_UID, "0");
  ASSERT_EQ(2u, s.size());
  ASSERT_TRUE(std::find(s.begin(), s.end(), a[0]) != s.end());
  ASSERT_TRUE(std::find(s.begin(), s.end(), a[2]) != s.end());

  index_->LookupTagValue(s, "0");
  ASSERT_EQ(3u, s.size());
  ASSERT_TRUE(std::find(s.begin(), s.end(), a[0]) != s.end());
  ASSERT_TRUE(std::find(s.begin(), s.end(), a[2]) != s.end());
  ASSERT_TRUE(std::find(s.begin(), s.end(), a[3]) != s.end());

  index_->LookupTagValue(s, DICOM_TAG_STUDY_INSTANCE_UID, "1");
  ASSERT_EQ(1u, s.size());
  ASSERT_TRUE(std::find(s.begin(), s.end(), a[1]) != s.end());

  index_->LookupTagValue(s, "1");
  ASSERT_EQ(1u, s.size());
  ASSERT_TRUE(std::find(s.begin(), s.end(), a[1]) != s.end());


  /*{
      std::list<std::string> s;
      context.GetIndex().LookupTagValue(s, DICOM_TAG_STUDY_INSTANCE_UID, "1.2.250.1.74.20130819132500.29000036381059");
      for (std::list<std::string>::iterator i = s.begin(); i != s.end(); i++)
      {
        std::cout << "*** " << *i << std::endl;;
      }      
      }*/
}



TEST(ServerIndex, AttachmentRecycling)
{
  const std::string path = "OrthancStorageUnitTests";
  Toolbox::RemoveFile(path + "/index");
  ServerContext context(path, ":memory:");   // The SQLite DB is in memory
  ServerIndex& index = context.GetIndex();

  index.SetMaximumStorageSize(10);

  Json::Value tmp;
  index.ComputeStatistics(tmp);
  ASSERT_EQ(0, tmp["CountPatients"].asInt());
  ASSERT_EQ(0, boost::lexical_cast<int>(tmp["TotalDiskSize"].asString()));

  ServerIndex::Attachments attachments;

  std::vector<std::string> ids;
  for (int i = 0; i < 10; i++)
  {
    std::string id = boost::lexical_cast<std::string>(i);
    DicomMap instance;
    instance.SetValue(DICOM_TAG_PATIENT_ID, "patient-" + id);
    instance.SetValue(DICOM_TAG_STUDY_INSTANCE_UID, "study-" + id);
    instance.SetValue(DICOM_TAG_SERIES_INSTANCE_UID, "series-" + id);
    instance.SetValue(DICOM_TAG_SOP_INSTANCE_UID, "instance-" + id);
    ASSERT_EQ(StoreStatus_Success, index.Store(instance, attachments, ""));

    DicomInstanceHasher hasher(instance);
    ids.push_back(hasher.HashPatient());
    ids.push_back(hasher.HashStudy());
    ids.push_back(hasher.HashSeries());
    ids.push_back(hasher.HashInstance());
  }

  index.ComputeStatistics(tmp);
  ASSERT_EQ(10, tmp["CountPatients"].asInt());
  ASSERT_EQ(0, boost::lexical_cast<int>(tmp["TotalDiskSize"].asString()));

  for (size_t i = 0; i < ids.size(); i++)
  {
    FileInfo info(Toolbox::GenerateUuid(), FileContentType_Dicom, 1, "md5");
    index.AddAttachment(info, ids[i]);

    index.ComputeStatistics(tmp);
    ASSERT_GE(10, boost::lexical_cast<int>(tmp["TotalDiskSize"].asString()));
  }

  // Because the DB is in memory, the SQLite index must not have been created
  ASSERT_THROW(Toolbox::GetFileSize(path + "/index"), OrthancException);  
}
