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

#pragma once

#ifndef WINDOW_EVENTS_ANDROID_H
#define WINDOW_EVENTS_ANDROID_H

#include "windowing/WinEvents.h"
#include "threads/Timer.h"
#include "threads/CriticalSection.h"
#include "input/MouseStat.h"

#include <map>
#include <string>
#include <vector>

typedef struct {
  int32_t id;
  std::string name;
} APP_InputDevice;

class CWinEventsAndroid : public CWinEventsBase, private ITimerCallback
{
public:
  CWinEventsAndroid();
  virtual ~CWinEventsAndroid();

  static CWinEventsAndroid &Get();

  static void MessagePush(XBMC_Event *newEvent);
  static bool MessagePump();

private:
  // implementation of ITimerCallback
  virtual void OnTimeout();
  void         HandleItemRepeat(XBMC_Event *event);

  static CCriticalSection             m_inputCond;
  static std::vector<XBMC_Event>      m_events;
  static std::vector<APP_InputDevice> m_inputs_devices;
  static std::map<int, std::map<int, XBMC_Event> > m_lastJoyAxisMap;
  static std::map<int, std::map<int, XBMC_Event> > m_lastJoyButtonMap;

  CCriticalSection                    m_holdCond;
  int32_t                             m_holdTimeout;
  CTimer                             *m_holdTimer;
};

#endif // WINDOW_EVENTS_ANDROID_H
