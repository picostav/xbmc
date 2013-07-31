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

#include "AndroidPosition.h"
#include "android/jni/View.h"
#include "DllAxis.h"
#include "utils/log.h"
#include "windowing/WinEvents.h"

#include <math.h>

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 11
#define AXIS_RX 12
#define AXIS_RY 13
#define AXIS_RZ 14
#define AXIS_HAT_X 15
#define AXIS_HAT_Y 16

CAndroidPosition::CAndroidPosition()
{
  m_dll = NULL;
}

CAndroidPosition::~CAndroidPosition()
{
  if (m_dll)
    m_dll->Unload();
  delete m_dll;
}

bool CAndroidPosition::onPositionEvent(AInputEvent *event)
{
  if (!m_dll)
  {
    m_dll = new DllAxis;
    if (m_dll)
      m_dll->Load();
  }

  if (m_dll)
  {
    int32_t device_id = AInputEvent_getDeviceId(event);

    // Left joystick
    if (!ProcessAxis(event, m_y_axis, device_id, 1, AXIS_Y))
      ProcessAxis(event, m_x_axis, device_id, 2, AXIS_X);
    // Right joystick
    if (!ProcessAxis(event, m_z_axis, device_id, 3, AXIS_Z))
      ProcessAxis(event, m_rz_axis,device_id, 4, AXIS_RZ);
    // Dpad
    if (!ProcessHat(event,  m_y_hat,  device_id, 5, AXIS_HAT_Y))
      ProcessHat(event,  m_x_hat,  device_id, 6, AXIS_HAT_X);

    //CLog::Log(LOGDEBUG, "joystick event. x(%f),  y(%f)", m_x_axis.value, m_y_axis.value);
    //CLog::Log(LOGDEBUG, "joystick event. z(%f), rz(%f)", m_z_axis.value, m_rz_axis.value);
    //CLog::Log(LOGDEBUG, "joystick event. xhat(%f), xhat(%f)", m_x_hat.value, m_y_hat.value);
  }

  return true;
}

bool CAndroidPosition::ProcessHat(AInputEvent *event,
  APP_InputPositionAxis &hat, int id, int keymap_axis, int android_axis)
{
  bool rtn = false;
  // Dpad (quantized to -1.0, 0.0 and 1.0)
  hat.value = m_dll->AMotionEvent_getAxisValue(event, android_axis, 0);
  if (!isValueInDeadZone(hat.value) && hat.value != hat.oldvalue)
    XBMC_JoyAxis(id, keymap_axis, hat.value), rtn = true;
  hat.oldvalue = hat.value;

  return rtn;
}

bool CAndroidPosition::ProcessAxis(AInputEvent *event,
  APP_InputPositionAxis &axis, int id, int keymap_axis, int android_axis)
{
  bool rtn = false;
  axis.value = DeadZoneClamp(m_dll->AMotionEvent_getAxisValue(event, android_axis, 0));
  if (fabs(axis.value) > 0.5 && axis.value != axis.oldvalue)
    XBMC_JoyAxis(id, keymap_axis, axis.value), rtn = true;
  axis.oldvalue = axis.value;

  return rtn;
}

float CAndroidPosition::DeadZoneClamp(float value)
{
  // The 1e-2 here should really be range.getFlat() + range.getFuzz() (where range is
  // event.getDevice().getMotionRange(axis)), but the values those functions return
  // on the Ouya are zero so we're just hard-coding it for now.
  if (fabs(value) < 1e-2)
    return 0.0;
  else // convert axis range to be -1 or 1.
    return value < 0.0 ? -1.0:1.0;
}

bool CAndroidPosition::isValueInDeadZone(float value)
{
  //float value = event.getAxisValue(axis);
  return (fabs(value) < 1e-2);
}

void CAndroidPosition::XBMC_JoyAxis(uint8_t id, uint8_t axis, float value)
{
  XBMC_Event newEvent = {0};

  newEvent.type       = XBMC_JOYAXISMOTION;
  newEvent.jaxis.type = XBMC_JOYAXISMOTION;
  newEvent.jaxis.which  = id;
  newEvent.jaxis.axis   = axis;
  newEvent.jaxis.fvalue = value;

  //CLog::Log(LOGDEBUG, "XBMC_Axis(%u, %u, %u, %f)", type, id, axis, value);
  CWinEvents::MessagePush(&newEvent);
}
