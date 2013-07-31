#pragma once

/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif
#include "DynamicDll.h"
class DllAxisInterface
{
public:
  virtual ~DllAxisInterface() {}
  virtual float AMotionEvent_getAxisValue(const AInputEvent* event,  int32_t axis, size_t pointer_index)=0;
  virtual float AMotionEvent_getHistoricalAxisValue(const AInputEvent* motion_event, int32_t axis, size_t pointer_index, size_t history_index)=0;

float AMotionEvent_getHistoricalSize(AInputEvent* motion_event, size_t pointer_index,
        size_t history_index);
};

class DllAxis : public DllDynamic, DllAxisInterface
{
  DECLARE_DLL_WRAPPER(DllAxis, "libandroid.so")
  DEFINE_METHOD3(float, AMotionEvent_getAxisValue, (const AInputEvent* p1,  int32_t p2, size_t p3))
  DEFINE_METHOD4(float, AMotionEvent_getHistoricalAxisValue, (const AInputEvent* p1,  int32_t p2, size_t p3, size_t p4))

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(AMotionEvent_getAxisValue)
    RESOLVE_METHOD(AMotionEvent_getHistoricalAxisValue)
  END_METHOD_RESOLVE()
};