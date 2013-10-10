/*
*      Copyright (C) 2010-2012 Team XBMC
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

#include "system.h"
#include "Application.h"
#include "WinEventsAndroid.h"
#include "android/jni/View.h"
#include "guilib/GUIWindowManager.h"
#include "input/XBMC_vkeys.h"
#include "threads/CriticalSection.h"
#include "utils/log.h"
#include "windowing/WinEvents.h"
#include "windowing/WindowingFactory.h"

CCriticalSection              CWinEventsAndroid::m_inputCond;
std::vector<XBMC_Event>       CWinEventsAndroid::m_events;
std::vector<APP_InputDevice>  CWinEventsAndroid::m_inputs_devices;
std::map<int, std::map<int, XBMC_Event> > CWinEventsAndroid::m_lastJoyAxisMap;
std::map<int, std::map<int, XBMC_Event> > CWinEventsAndroid::m_lastJoyButtonMap;

CWinEventsAndroid::CWinEventsAndroid()
{
  CLog::Log(LOGDEBUG, "CWinEventsAndroid::CWinEventsAndroid");
  m_holdTimeout = 250;
  m_holdTimer = new CTimer(this);
}

CWinEventsAndroid::~CWinEventsAndroid()
{
  delete m_holdTimer;
}

CWinEventsAndroid &CWinEventsAndroid::Get()
{
  static CWinEventsAndroid sWinEventsAndroid;
  return sWinEventsAndroid;
}

void CWinEventsAndroid::OnTimeout()
{
  bool restart = false;
  CSingleLock lock(m_holdCond);

  //CLog::Log(LOGDEBUG, "CWinEventsAndroid::OnTimeout");

  if (!m_lastJoyAxisMap.empty())
  {
    // Process all the stored axis.
    for (map<int, map<int, XBMC_Event> >::iterator iter = m_lastJoyAxisMap.begin(); iter != m_lastJoyAxisMap.end(); ++iter)
    {
      for (map<int, XBMC_Event>::iterator iterAxis = (*iter).second.begin(); iterAxis != (*iter).second.end(); ++iterAxis)
      {
        restart = true;
        //CWinEvents::MessagePush(&(*iterAxis).second);
      }
    }
  }
  if (!m_lastJoyButtonMap.empty())
  {
    // Process all the stored axis.
    for (map<int, map<int, XBMC_Event> >::iterator iter = m_lastJoyButtonMap.begin(); iter != m_lastJoyButtonMap.end(); ++iter)
    {
      for (map<int, XBMC_Event>::iterator iterButton = (*iter).second.begin(); iterButton != (*iter).second.end(); ++iterButton)
      {
        restart = true;
        //CWinEvents::MessagePush(&(*iterButton).second);
      }
    }
  }

  if (restart)
    m_holdTimer->Restart();
}

void CWinEventsAndroid::HandleItemRepeat(XBMC_Event *event)
{
  CSingleLock lock(m_holdCond);

  switch(event->type)
  {
    case XBMC_JOYAXISMOTION:
      if (fabs(event->jaxis.fvalue) >= 0.08)
      {
        m_lastJoyAxisMap[event->jaxis.which][event->jaxis.axis] = *event;
        m_holdTimer->Stop(true);
        m_holdTimer->Start(m_holdTimeout);
      }
      else
      {
        m_lastJoyAxisMap[event->jaxis.which].erase(event->jaxis.axis);
        m_holdTimer->Stop(false);
      }
      break;
    case XBMC_JOYBUTTONDOWN:
      m_lastJoyButtonMap[event->jbutton.which][event->jbutton.button] = *event;
      m_holdTimer->Stop(true);
      m_holdTimer->Start(m_holdTimeout);
      break;
    case XBMC_JOYBUTTONUP:
      m_lastJoyButtonMap[event->jbutton.which].erase(event->jbutton.button);
      m_holdTimer->Stop(false);
      break;
  }
}

void CWinEventsAndroid::MessagePush(XBMC_Event *newEvent)
{
  CSingleLock lock(m_inputCond);
  m_events.push_back(*newEvent);
}

bool CWinEventsAndroid::MessagePump()
{
  bool ret = false;
  std::vector<XBMC_Event> copy_events;
  { // double-buffered events to avoid constant locking for OnEvent().
    CSingleLock lock(m_inputCond);
    copy_events = m_events;
    m_events.clear();
  }

  for (std::vector<XBMC_Event>::iterator iter = copy_events.begin(); iter != copy_events.end(); ++iter)
  {
    XBMC_Event *pumpEvent = (XBMC_Event*)&*iter;
    if ((pumpEvent->type == XBMC_JOYBUTTONUP)   ||
        (pumpEvent->type == XBMC_JOYBUTTONDOWN) ||
        (pumpEvent->type == XBMC_JOYAXISMOTION))
    {
      int             item;
      int             type;
      uint32_t        holdTime;
      APP_InputDevice input_device;

      type = pumpEvent->type;
      if (type == XBMC_JOYAXISMOTION)
      {
        // The typical joystick keymap xml has the following where 'id' is the axis
        //  and 'limit' is which action to choose (ie. Up or Down).
        //  <axis id="5" limit="-1">Up</axis>
        //  <axis id="5" limit="+1">Down</axis>
        // One would think that limits is in reference to fvalue but
        // it is really in reference to id :) The sign of item passed
        // into ProcessJoystickEvent indicates the action mapping.
        item = pumpEvent->jaxis.axis;
        if (pumpEvent->jaxis.fvalue < 0.0f)
          item = -item;
        holdTime = 0;
        input_device.id = pumpEvent->jaxis.which;
        //CLog::Log(LOGDEBUG, "CWinEventsAndroid::MessagePump:"
        //  "item(%d), fvalue(%f)", item, pumpEvent->jaxis.fvalue);
      }
      else
      {
        item = pumpEvent->jbutton.button;
        holdTime = pumpEvent->jbutton.holdTime;
        input_device.id = pumpEvent->jbutton.which;
      }

      // look for device name in our inputdevice cache.
      for (size_t i = 0; i < m_inputs_devices.size(); i++)
      {
        if (m_inputs_devices[i].id == input_device.id)
          input_device.name = m_inputs_devices[i].name;
      }
      if (input_device.name.empty())
      {
        // not in inputdevice cache, fetch and cache it.
        CJNIViewInputDevice view_input_device = CJNIViewInputDevice::getDevice(input_device.id);
        input_device.name = view_input_device.getName();
        CLog::Log(LOGDEBUG, "CWinEventsAndroid::MessagePump:caching  id(%d), device(%s)",
          input_device.id, input_device.name.c_str());
        m_inputs_devices.push_back(input_device);
      }

      // handle repeating of items. We are in a static routine
      // so we have to create/get the singleton and call it.
      // non-working, review this later.
      //CWinEventsAndroid::Get().HandleItemRepeat(pumpEvent);

      ret |= g_application.ProcessJoystickEvent(input_device.name,
        item, type == XBMC_JOYAXISMOTION, 1.0f, holdTime);
    }
    else
    {
      ret |= g_application.OnEvent(*iter);
    }

    if (iter->type == XBMC_MOUSEBUTTONUP)
      g_windowManager.SendMessage(GUI_MSG_UNFOCUS_ALL, 0, 0, 0, 0);
  }

  return ret;
}
