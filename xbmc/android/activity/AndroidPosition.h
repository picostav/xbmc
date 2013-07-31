#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include <android/input.h>
#include "windowing/XBMC_events.h"

#include <string>
#include <vector>

typedef struct {
  float     flat;
  float     fuzz;
  float     min;
  float     max;
  float     range;
  float     value;
  float     oldvalue;
} APP_InputPositionAxis;

class DllAxis;
class CAndroidPosition
{
public:
  CAndroidPosition();
 ~CAndroidPosition();

  bool  onPositionEvent(AInputEvent *event);

private:
  bool  ProcessHat( AInputEvent *event, APP_InputPositionAxis &hat,  int id, int keymap_axis, int android_axis);
  bool  ProcessAxis(AInputEvent *event, APP_InputPositionAxis &axis, int id, int keymap_axis, int android_axis);
  float DeadZoneClamp(float value);
  bool  isValueInDeadZone(float value);
  void  XBMC_Key(XBMC_Event &last_Event,
          uint8_t code, XBMCKey sym, XBMCMod modifiers, unsigned char type);
  void  XBMC_JoyAxis(uint8_t id, uint8_t axis, float value);

  DllAxis              *m_dll;
  APP_InputPositionAxis m_x_hat;
  APP_InputPositionAxis m_y_hat;
  APP_InputPositionAxis m_x_axis;
  APP_InputPositionAxis m_y_axis;
  APP_InputPositionAxis m_z_axis;
  APP_InputPositionAxis m_rz_axis;

};
