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

#include "DicomMap.h"

#include <stdint.h>

namespace Orthanc
{
  class DicomIntegerPixelAccessor
  {
  private:
    unsigned int width_;
    unsigned int height_;
    unsigned int samplesPerPixel_;
    unsigned int numberOfFrames_;
    unsigned int planarConfiguration_;
    const void* pixelData_;
    size_t size_;

    uint8_t shift_;
    uint32_t signMask_;
    uint32_t mask_;
    size_t bytesPerPixel_;
    unsigned int frame_;

    size_t frameOffset_;
    size_t rowOffset_;

  public:
    DicomIntegerPixelAccessor(const DicomMap& values,
                              const void* pixelData,
                              size_t size);

    unsigned int GetWidth() const
    {
      return width_;
    }

    unsigned int GetHeight() const
    {
      return height_;
    }

    unsigned int GetNumberOfFrames() const
    {
      return numberOfFrames_;
    }

    unsigned int GetCurrentFrame() const
    {
      return frame_;
    }

    void SetCurrentFrame(unsigned int frame);

    void GetExtremeValues(int32_t& min, 
                          int32_t& max) const;

    unsigned int GetChannelCount() const
    {
      return samplesPerPixel_;
    }

    int32_t GetValue(unsigned int x, unsigned int y, unsigned int channel = 0) const;
  };
}
