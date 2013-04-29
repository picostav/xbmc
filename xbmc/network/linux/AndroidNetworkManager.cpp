/*
 *      Copyright (C) 2005-2010 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "AndroidNetworkManager.h"
#include "AndroidConnection.h"
#include "android/jni/ConnectivityManager.h"
#include "android/jni/WifiManager.h"
#include "android/jni/ScanResult.h"
#include "android/jni/List.h"
#include "android/jni/NetworkInfo.h"
#include "android/jni/Context.h"
#include "android/jni/DhcpInfo.h"
#include "android/jni/WifiInfo.h"
#include "android/jni/WifiConfiguration.h"
#include "utils/log.h"
#include "settings/AdvancedSettings.h"

CAndroidNetworkManager::CAndroidNetworkManager()
{
  CLog::Log(LOGDEBUG, "NetworkManager: AndroidNetworkManager created");
}

CAndroidNetworkManager::~CAndroidNetworkManager()
{
}

bool CAndroidNetworkManager::CanManageConnections()
{
  return true;
}

ConnectionList CAndroidNetworkManager::GetConnections()
{
  ConnectionList connections;
  CJNIConnectivityManager connectivityManager(CJNIContext::getSystemService("connectivity"));
  std::vector<CJNINetworkInfo> networks = connectivityManager.getAllNetworkInfo();
  for (std::vector<CJNINetworkInfo>::const_iterator i = networks.begin(); i != networks.end(); i++)
  {
    // ppp, vpn, 3g, etc. when they're unavailable (no service, for example)
    if (!i->isAvailable())
      continue;

    CLog::Log(LOGDEBUG, "NetworkManager: has connection(%s)", i->getTypeName().c_str());

    // Skip Wifi, access points will be picked up by the WifiManager below
    if (i->getType() == CJNIConnectivityManager::TYPE_WIFI)
      continue;

    CLog::Log(LOGDEBUG, "NetworkManager: new connection(%s)", i->getTypeName().c_str());
    CConnectionPtr network(new CAndroidConnection(*i));
    connections.push_back(network);
  }

  CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
  if(wifiManager.isWifiEnabled())
  {
    // Current wifi connection, if any
    CJNIWifiInfo wifiInfo = wifiManager.getConnectionInfo();
    std::string currentBSSID = wifiInfo.getBSSID();
    connections.push_back(CConnectionPtr(new CAndroidConnection(wifiInfo)));

    // Available access points
    CJNIList<CJNIScanResult> survey = wifiManager.getScanResults();
    int numAccessPoints = survey.size();
    for (int i = 0; i < numAccessPoints; i++)
    {
      CJNIScanResult accessPoint = survey.get(i);
      if (accessPoint.BSSID != currentBSSID);
        connections.push_back(CConnectionPtr(new CAndroidConnection(accessPoint)));
    }
  }
  return connections;
}

bool CAndroidNetworkManager::PumpNetworkEvents(INetworkEventsCallback *callback)
{
  bool result = false;

  // check for a failed startup connection
  // pump  our connection
  for (size_t i = 0; i < m_connections.size(); i++)
  {
    if (((CAndroidConnection*)m_connections[i].get())->PumpNetworkEvents())
    {
      //some connection state changed (connected or disconnected)
      if (((CAndroidConnection*)m_connections[i].get())->GetState() == NETWORK_CONNECTION_STATE_CONNECTED)
      {
        // callback to CNetworkManager to setup the
        // m_defaultConnection and update GUI state if showing.
        callback->OnConnectionChange(m_connections[i]);
        // callback to start services
        callback->OnConnectionStateChange(NETWORK_CONNECTION_STATE_CONNECTED);
        CLog::Log(LOGDEBUG, "NetworkManager: connected(%s)",
          ((CAndroidConnection*)m_connections[i].get())->GetName().c_str());
        result = true;
      }
    }
  }

  return result;
}

bool CAndroidNetworkManager::SendWakeOnLan(const char *mac)
{
  return false;
}
