

package com.google.zxing.client.android.wifi;

import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiEnterpriseConfig;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.util.Log;

import java.util.regex.Pattern;

import com.google.zxing.client.result.WifiParsedResult;


public final class WifiConfigManager extends AsyncTask<WifiParsedResult,Object,Object> {

  private static final String TAG = WifiConfigManager.class.getSimpleName();

  private static final Pattern HEX_DIGITS = Pattern.compile("[0-9A-Fa-f]+");

  private final WifiManager wifiManager;

  public WifiConfigManager(WifiManager wifiManager) {
    this.wifiManager = wifiManager;
  }

  @Override
  protected Object doInBackground(WifiParsedResult... args) {
    WifiParsedResult theWifiResult = args[0];
        if (!wifiManager.isWifiEnabled()) {
      Log.i(TAG, "Enabling wi-fi...");
      if (wifiManager.setWifiEnabled(true)) {
        Log.i(TAG, "Wi-fi enabled");
      } else {
        Log.w(TAG, "Wi-fi could not be enabled!");
        return null;
      }
            int count = 0;
      while (!wifiManager.isWifiEnabled()) {
        if (count >= 10) {
          Log.i(TAG, "Took too long to enable wi-fi, quitting");
          return null;
        }
        Log.i(TAG, "Still waiting for wi-fi to enable...");
        try {
          Thread.sleep(1000L);
        } catch (InterruptedException ie) {
                  }
        count++;
      }
    }
    String networkTypeString = theWifiResult.getNetworkEncryption();
    NetworkType networkType;
    try {
      networkType = NetworkType.forIntentValue(networkTypeString);
    } catch (IllegalArgumentException ignored) {
      Log.w(TAG, "Bad network type; see NetworkType values: " + networkTypeString);
      return null;
    }
    if (networkType == NetworkType.NO_PASSWORD) {
      changeNetworkUnEncrypted(wifiManager, theWifiResult);
    } else {
      String password = theWifiResult.getPassword();
      if (password != null && !password.isEmpty()) {
        switch (networkType) {
          case WEP:
            changeNetworkWEP(wifiManager, theWifiResult);
            break;
          case WPA:
            changeNetworkWPA(wifiManager, theWifiResult);
            break;
          case WPA2_EAP:
            changeNetworkWPA2EAP(wifiManager, theWifiResult);
            break;
        }
      }
    }
    return null;
  }

  
  private static void updateNetwork(WifiManager wifiManager, WifiConfiguration config) {
    Integer foundNetworkID = findNetworkInExistingConfig(wifiManager, config.SSID);
    if (foundNetworkID != null) {
      Log.i(TAG, "Removing old configuration for network " + config.SSID);
      wifiManager.removeNetwork(foundNetworkID);
      wifiManager.saveConfiguration();
    }
    int networkId = wifiManager.addNetwork(config);
    if (networkId >= 0) {
            if (wifiManager.enableNetwork(networkId, true)) {
        Log.i(TAG, "Associating to network " + config.SSID);
        wifiManager.saveConfiguration();
      } else {
        Log.w(TAG, "Failed to enable network " + config.SSID);
      }
    } else {
      Log.w(TAG, "Unable to add network " + config.SSID);
    }
  }

  private static WifiConfiguration changeNetworkCommon(WifiParsedResult wifiResult) {
    WifiConfiguration config = new WifiConfiguration();
    config.allowedAuthAlgorithms.clear();
    config.allowedGroupCiphers.clear();
    config.allowedKeyManagement.clear();
    config.allowedPairwiseCiphers.clear();
    config.allowedProtocols.clear();
        config.SSID = quoteNonHex(wifiResult.getSsid());
    config.hiddenSSID = wifiResult.isHidden();
    return config;
  }

    private static void changeNetworkWEP(WifiManager wifiManager, WifiParsedResult wifiResult) {
    WifiConfiguration config = changeNetworkCommon(wifiResult);
    config.wepKeys[0] = quoteNonHex(wifiResult.getPassword(), 10, 26, 58);
    config.wepTxKeyIndex = 0;
    config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.SHARED);
    config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
    config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
    config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
    config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
    config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP104);
    updateNetwork(wifiManager, config);
  }

    private static void changeNetworkWPA(WifiManager wifiManager, WifiParsedResult wifiResult) {
    WifiConfiguration config = changeNetworkCommon(wifiResult);
        config.preSharedKey = quoteNonHex(wifiResult.getPassword(), 64);
    config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
    config.allowedProtocols.set(WifiConfiguration.Protocol.WPA);     config.allowedProtocols.set(WifiConfiguration.Protocol.RSN);     config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
    config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
    config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
    config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
    config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
    config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
    updateNetwork(wifiManager, config);
  }

    private static void changeNetworkWPA2EAP(WifiManager wifiManager, WifiParsedResult wifiResult) {
    WifiConfiguration config = changeNetworkCommon(wifiResult);
        config.preSharedKey = quoteNonHex(wifiResult.getPassword(), 64);
    config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
    config.allowedProtocols.set(WifiConfiguration.Protocol.RSN);     config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_EAP);
    config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
    config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
    config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
    config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
    config.enterpriseConfig.setIdentity(wifiResult.getIdentity());
    config.enterpriseConfig.setAnonymousIdentity(wifiResult.getAnonymousIdentity());
    config.enterpriseConfig.setPassword(wifiResult.getPassword());
    config.enterpriseConfig.setEapMethod(parseEap(wifiResult.getEapMethod()));
    config.enterpriseConfig.setPhase2Method(parsePhase2(wifiResult.getPhase2Method()));
    updateNetwork(wifiManager, config);
  }

    private static void changeNetworkUnEncrypted(WifiManager wifiManager, WifiParsedResult wifiResult) {
    WifiConfiguration config = changeNetworkCommon(wifiResult);
    config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
    updateNetwork(wifiManager, config);
  }

  private static Integer findNetworkInExistingConfig(WifiManager wifiManager, String ssid) {
    Iterable<WifiConfiguration> existingConfigs = wifiManager.getConfiguredNetworks();
    if (existingConfigs != null) {
      for (WifiConfiguration existingConfig : existingConfigs) {
        String existingSSID = existingConfig.SSID;
        if (existingSSID != null && existingSSID.equals(ssid)) {
          return existingConfig.networkId;
        }
      }
    }
    return null;
  }

  private static String quoteNonHex(String value, int... allowedLengths) {
    return isHexOfLength(value, allowedLengths) ? value : convertToQuotedString(value);
  }

  
  private static String convertToQuotedString(String s) {
    if (s == null || s.isEmpty()) {
      return null;
    }
        if (s.charAt(0) == '"' && s.charAt(s.length() - 1) == '"') {
      return s;
    }
    return '\"' + s + '\"';
  }

  
  private static boolean isHexOfLength(CharSequence value, int... allowedLengths) {
    if (value == null || !HEX_DIGITS.matcher(value).matches()) {
      return false;
    }
    if (allowedLengths.length == 0) {
      return true;
    }
    for (int length : allowedLengths) {
      if (value.length() == length) {
        return true;
      }
    }
    return false;
  }

  private static int parseEap(String eapString) {
    if (eapString == null) {
      return WifiEnterpriseConfig.Eap.NONE;
    }
    switch (eapString) {
    case "NONE":
      return WifiEnterpriseConfig.Eap.NONE;
    case "PEAP":
      return WifiEnterpriseConfig.Eap.PEAP;
    case "PWD":
      return WifiEnterpriseConfig.Eap.PWD;
    case "TLS":
      return WifiEnterpriseConfig.Eap.TLS;
    case "TTLS":
      return WifiEnterpriseConfig.Eap.TTLS;
    default:
      throw new IllegalArgumentException("Unknown value for EAP method: " + eapString);
    }
  }

  private static int parsePhase2(String phase2String) {
    if (phase2String == null) {
      return WifiEnterpriseConfig.Phase2.NONE;
    }
    switch (phase2String) {
    case "GTC":
      return WifiEnterpriseConfig.Phase2.GTC;
    case "MSCHAP":
      return WifiEnterpriseConfig.Phase2.MSCHAP;
    case "MSCHAPV2":
      return WifiEnterpriseConfig.Phase2.MSCHAPV2;
    case "NONE":
      return WifiEnterpriseConfig.Phase2.NONE;
    case "PAP":
      return WifiEnterpriseConfig.Phase2.PAP;
    default:
      throw new IllegalArgumentException("Unknown value for phase 2 method: " + phase2String);
    }
  }

}
