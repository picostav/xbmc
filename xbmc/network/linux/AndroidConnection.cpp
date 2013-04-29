/*
 *      Copyright (C) 2011-2012 Team XBMC
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

#include "AndroidConnection.h"
#include "android/jni/NetworkInfo.h"
#include "android/jni/WifiInfo.h"
#include "android/jni/DhcpInfo.h"
#include "android/jni/ScanResult.h"
#include "android/jni/ConnectivityManager.h"
#include "android/jni/WifiManager.h"
#include "android/jni/WifiConfiguration.h"
#include "android/jni/Context.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include <arpa/inet.h>

#include <sys/socket.h>
#include <net/if_arp.h>

// Non-Wifi interfaces
CAndroidConnection::CAndroidConnection(const CJNINetworkInfo &networkInfo)
: m_type (NETWORK_CONNECTION_TYPE_UNKNOWN)
, m_state(NETWORK_CONNECTION_STATE_UNKNOWN)
, m_method(IP_CONFIG_DHCP)
, m_encryption(NETWORK_CONNECTION_ENCRYPTION_UNKNOWN)
, m_signal(0)
, m_speed(0)
, m_networkId(0)
{
  m_name = networkInfo.getTypeName();
  m_type = GetType(networkInfo.getType());
  m_state = GetState(networkInfo.getState().name());
  // hack, hack
  if (m_type == NETWORK_CONNECTION_TYPE_WIRED)
  {
    std::string tmp;
    struct ifreq ifr;
    strcpy(ifr.ifr_name, "eth0");
    ifr.ifr_addr.sa_family = AF_INET;

    int asocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (ioctl(asocket, SIOCGIFADDR, &ifr) >= 0)
      m_address = inet_ntoa((*((struct sockaddr_in*)&ifr.ifr_addr)).sin_addr);

    if (ioctl(asocket, SIOCGIFNETMASK, &ifr) >= 0)
      m_netmask = inet_ntoa((*((struct sockaddr_in*)&ifr.ifr_addr)).sin_addr);

    char macaddress[1024] = {0};
    if (ioctl(asocket, SIOCGIFHWADDR, &ifr) >= 0)
    {
      sprintf(macaddress, "%02X:%02X:%02X:%02X:%02X:%02X",
        ifr.ifr_hwaddr.sa_data[0], ifr.ifr_hwaddr.sa_data[1],
        ifr.ifr_hwaddr.sa_data[2], ifr.ifr_hwaddr.sa_data[3],
        ifr.ifr_hwaddr.sa_data[4], ifr.ifr_hwaddr.sa_data[5]);
      m_macaddress = macaddress;
    }

    close(asocket);
  }
}

// Current Wifi Connection
CAndroidConnection::CAndroidConnection(const CJNIWifiInfo &wifiInfo)
: m_type (NETWORK_CONNECTION_TYPE_WIFI)
, m_state(NETWORK_CONNECTION_STATE_DISCONNECTED)
, m_method(IP_CONFIG_DHCP)
, m_encryption(NETWORK_CONNECTION_ENCRYPTION_UNKNOWN)
, m_speed(0)
, m_networkId(0)
{
    m_name = wifiInfo.getSSID();
    m_signal = CJNIWifiManager::calculateSignalLevel(wifiInfo.getRssi(), 100);
    m_address = GetAddress(wifiInfo.getIpAddress());
    m_macaddress = wifiInfo.getMacAddress();
    StringUtils::ToUpper(m_macaddress);
    std::string state = CJNIConnectivityManager(CJNIContext::getSystemService("connectivity")).getNetworkInfo(CJNIConnectivityManager::TYPE_WIFI).getState().name();
    m_state = GetState(state);
}

// Available access points
CAndroidConnection::CAndroidConnection(const CJNIScanResult &scanResult)
: m_name(scanResult.SSID)
, m_type (NETWORK_CONNECTION_TYPE_WIFI)
, m_state(NETWORK_CONNECTION_STATE_DISCONNECTED)
, m_method(IP_CONFIG_DHCP)
, m_encryption(NETWORK_CONNECTION_ENCRYPTION_UNKNOWN)
, m_speed(0)
, m_networkId(0)
{
  m_signal = CJNIWifiManager::calculateSignalLevel(scanResult.level, 100);
  m_encryption = GetEncryption(scanResult.capabilities);
}

CAndroidConnection::CAndroidConnection::~CAndroidConnection()
{
}

std::string CAndroidConnection::GetName() const
{
  return m_name;
}

std::string CAndroidConnection::GetAddress() const
{
  return m_address;
}

std::string CAndroidConnection::GetNetmask() const
{
  return m_netmask;
}

std::string CAndroidConnection::GetGateway() const
{
  return m_gateway;
}

std::string CAndroidConnection::GetNameServer() const
{
  return m_nameserver;
}

std::string CAndroidConnection::GetMacAddress() const
{
  return m_macaddress;
}

ConnectionType CAndroidConnection::GetType() const
{
  return m_type;
}

ConnectionState CAndroidConnection::GetState() const
{
  return m_state;
}

unsigned int CAndroidConnection::GetSpeed() const
{
  return m_speed;
}

IPConfigMethod CAndroidConnection::GetMethod() const
{
  return m_method;
}

unsigned int CAndroidConnection::GetStrength() const
{
  return m_signal;
}

EncryptionType CAndroidConnection::GetEncryption() const
{
  return m_encryption;
}

bool CAndroidConnection::Connect(IPassphraseStorage *storage, const CIPConfig &ipconfig)
{
  if (m_type == NETWORK_CONNECTION_TYPE_WIFI && m_networkId)
  {
    CJNIWifiManager wifiManager(CJNIContext::getSystemService("wifi"));
    wifiManager.enableNetwork(m_networkId, true);
  }
  return true;
}

//-----------------------------------------------------------------------
bool CAndroidConnection::PumpNetworkEvents()
{
  return false;
}

EncryptionType CAndroidConnection::GetEncryption(const std::string &capabilities)
{
  if (capabilities.find("WPA2"))
     return NETWORK_CONNECTION_ENCRYPTION_WPA2;
  else if(capabilities.find("WPA"))
     return NETWORK_CONNECTION_ENCRYPTION_WPA;
  else if(capabilities.find("WEP"))
     return NETWORK_CONNECTION_ENCRYPTION_WEP;
  else
    return NETWORK_CONNECTION_ENCRYPTION_UNKNOWN;
}

ConnectionState CAndroidConnection::GetState(const std::string &state)
{
  if (state == "CONNECTED")
    return NETWORK_CONNECTION_STATE_CONNECTED;
  else if (state == "CONNECTING"           || \
           state == "AUTHENTICATING"       || \
           state == "OBTAINING_IPADDR")
    return NETWORK_CONNECTION_STATE_CONNECTING;
  else if (state == "BLOCKED"              || \
             state == "FAILED")
    return NETWORK_CONNECTION_STATE_FAILURE;

  return NETWORK_CONNECTION_STATE_DISCONNECTED;
}

ConnectionType CAndroidConnection::GetType(int type)
{
  if (type == CJNIConnectivityManager::TYPE_ETHERNET)
    return NETWORK_CONNECTION_TYPE_WIRED;
  else if (type == CJNIConnectivityManager::TYPE_WIFI)
    return NETWORK_CONNECTION_TYPE_WIFI;
  return NETWORK_CONNECTION_TYPE_UNKNOWN;
}

std::string CAndroidConnection::GetAddress(int ip)
{
  char str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &ip, str, INET_ADDRSTRLEN);
  return str;
}
