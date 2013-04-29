#pragma once
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

#include "xbmc/network/IConnection.h"

class CJNINetworkInfo;
class CJNIScanResult;
class CJNIWifiInfo;
class CJNIWifiConfiguration;
class CAndroidConnection : public IConnection
{
public:
  CAndroidConnection(const CJNINetworkInfo &networkInfo);
  CAndroidConnection(const CJNIWifiInfo &wifiInfo);
//, const CJNIScanResult &scanResult);
  CAndroidConnection(const CJNIScanResult &scanResult);
//, const CJNIWifiConfiguration &wifiConfig);
  virtual ~CAndroidConnection();

  virtual std::string     GetName()       const;
  virtual std::string     GetAddress()    const;
  virtual std::string     GetNetmask()    const;
  virtual std::string     GetGateway()    const;
  virtual std::string     GetNameServer() const;
  virtual std::string     GetMacAddress() const;

  virtual ConnectionType  GetType()       const;
  virtual ConnectionState GetState()      const;
  virtual unsigned int    GetSpeed()      const;
  virtual IPConfigMethod  GetMethod()     const;
  virtual unsigned int    GetStrength()   const;
  virtual EncryptionType  GetEncryption() const;

  virtual bool            Connect(IPassphraseStorage *storage, const CIPConfig &ipconfig);

  bool PumpNetworkEvents();

  void SetCanManage(bool can_manange);
private:
  EncryptionType GetEncryption(const std::string &capabilities);
  ConnectionState GetState(const std::string &state);
  ConnectionType  GetType(int type);
  std::string GetAddress(int ip);
  bool            m_managed;

  std::string     m_name;
  std::string     m_address;
  std::string     m_netmask;
  std::string     m_gateway;
  std::string     m_macaddress;
  std::string     m_nameserver;

  ConnectionType  m_type;
  ConnectionState m_state;
  IPConfigMethod  m_method;
  EncryptionType  m_encryption;

  int             m_signal;
  int             m_speed;
  std::string     m_interface;

  int             m_networkId;
};
