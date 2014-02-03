/**
 * Orthanc - A Lightweight, RESTful DICOM Store
 * Copyright (C) 2012-2014 Medical Physics Department, CHU of Liege,
 * Belgium
 *
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *    * Neither the name of Google Inc., the name of the CHU of Liege,
 * nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/


#include "Transaction.h"

namespace Orthanc
{
  namespace SQLite
  {
    Transaction::Transaction(Connection& connection) :
      connection_(connection),
      isOpen_(false)
    {
    }

    Transaction::~Transaction()
    {
      if (isOpen_)
      {
        connection_.RollbackTransaction();
      }
    }

    void Transaction::Begin()
    {
      if (isOpen_) 
      {
        throw OrthancException("SQLite: Beginning a transaction twice!");
      }

      isOpen_ = connection_.BeginTransaction();
      if (!isOpen_)
      {
        throw OrthancException("SQLite: Unable to create a transaction");
      }
    }

    void Transaction::Rollback() 
    {
      if (!isOpen_) 
      {
        throw OrthancException("SQLite: Attempting to roll back a nonexistent transaction. "
                                "Did you remember to call Begin()?");
      }

      isOpen_ = false;

      connection_.RollbackTransaction();
    }

    void Transaction::Commit() 
    {
      if (!isOpen_) 
      {
        throw OrthancException("SQLite: Attempting to roll back a nonexistent transaction. "
                                "Did you remember to call Begin()?");
      }

      isOpen_ = false;

      if (!connection_.CommitTransaction())
      {
        throw OrthancException("SQLite: Failure when committing the transaction");
      }
    }
  }
}
