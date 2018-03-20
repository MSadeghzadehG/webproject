

package com.google.zxing.integration.android;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Fragment;
import android.content.ActivityNotFoundException;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;


public class IntentIntegrator {

  public static final int REQUEST_CODE = 0x0000c0de;   private static final String TAG = IntentIntegrator.class.getSimpleName();

  public static final String DEFAULT_TITLE = "Install Barcode Scanner?";
  public static final String DEFAULT_MESSAGE =
      "This application requires Barcode Scanner. Would you like to install it?";
  public static final String DEFAULT_YES = "Yes";
  public static final String DEFAULT_NO = "No";

  private static final String BS_PACKAGE = "com.google.zxing.client.android";
  private static final String BSPLUS_PACKAGE = "com.srowen.bs.android";

    public static final Collection<String> PRODUCT_CODE_TYPES = list("UPC_A", "UPC_E", "EAN_8", "EAN_13", "RSS_14");
  public static final Collection<String> ONE_D_CODE_TYPES =
      list("UPC_A", "UPC_E", "EAN_8", "EAN_13", "CODE_39", "CODE_93", "CODE_128",
           "ITF", "RSS_14", "RSS_EXPANDED");
  public static final Collection<String> QR_CODE_TYPES = Collections.singleton("QR_CODE");
  public static final Collection<String> DATA_MATRIX_TYPES = Collections.singleton("DATA_MATRIX");

  public static final Collection<String> ALL_CODE_TYPES = null;
  
  public static final List<String> TARGET_BARCODE_SCANNER_ONLY = Collections.singletonList(BS_PACKAGE);
  public static final List<String> TARGET_ALL_KNOWN = list(
          BSPLUS_PACKAGE,                       BSPLUS_PACKAGE + ".simple",           BS_PACKAGE                                  );

      private static final int FLAG_NEW_DOC = Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET;
  
  private final Activity activity;
  private final Fragment fragment;

  private String title;
  private String message;
  private String buttonYes;
  private String buttonNo;
  private List<String> targetApplications;
  private final Map<String,Object> moreExtras = new HashMap<String,Object>(3);

  
  public IntentIntegrator(Activity activity) {
    this.activity = activity;
    this.fragment = null;
    initializeConfiguration();
  }

  
  public IntentIntegrator(Fragment fragment) {
    this.activity = fragment.getActivity();
    this.fragment = fragment;
    initializeConfiguration();
  }

  private void initializeConfiguration() {
    title = DEFAULT_TITLE;
    message = DEFAULT_MESSAGE;
    buttonYes = DEFAULT_YES;
    buttonNo = DEFAULT_NO;
    targetApplications = TARGET_ALL_KNOWN;
  }
  
  public String getTitle() {
    return title;
  }
  
  public void setTitle(String title) {
    this.title = title;
  }

  public void setTitleByID(int titleID) {
    title = activity.getString(titleID);
  }

  public String getMessage() {
    return message;
  }

  public void setMessage(String message) {
    this.message = message;
  }

  public void setMessageByID(int messageID) {
    message = activity.getString(messageID);
  }

  public String getButtonYes() {
    return buttonYes;
  }

  public void setButtonYes(String buttonYes) {
    this.buttonYes = buttonYes;
  }

  public void setButtonYesByID(int buttonYesID) {
    buttonYes = activity.getString(buttonYesID);
  }

  public String getButtonNo() {
    return buttonNo;
  }

  public void setButtonNo(String buttonNo) {
    this.buttonNo = buttonNo;
  }

  public void setButtonNoByID(int buttonNoID) {
    buttonNo = activity.getString(buttonNoID);
  }
  
  public Collection<String> getTargetApplications() {
    return targetApplications;
  }
  
  public final void setTargetApplications(List<String> targetApplications) {
    if (targetApplications.isEmpty()) {
      throw new IllegalArgumentException("No target applications");
    }
    this.targetApplications = targetApplications;
  }
  
  public void setSingleTargetApplication(String targetApplication) {
    this.targetApplications = Collections.singletonList(targetApplication);
  }

  public Map<String,?> getMoreExtras() {
    return moreExtras;
  }

  public final void addExtra(String key, Object value) {
    moreExtras.put(key, value);
  }

  
  public final AlertDialog initiateScan() {
    return initiateScan(ALL_CODE_TYPES, -1);
  }
  
  
  public final AlertDialog initiateScan(int cameraId) {
    return initiateScan(ALL_CODE_TYPES, cameraId);
  }

  
  public final AlertDialog initiateScan(Collection<String> desiredBarcodeFormats) {
    return initiateScan(desiredBarcodeFormats, -1);
  }
  
  
  public final AlertDialog initiateScan(Collection<String> desiredBarcodeFormats, int cameraId) {
    Intent intentScan = new Intent(BS_PACKAGE + ".SCAN");
    intentScan.addCategory(Intent.CATEGORY_DEFAULT);

        if (desiredBarcodeFormats != null) {
            StringBuilder joinedByComma = new StringBuilder();
      for (String format : desiredBarcodeFormats) {
        if (joinedByComma.length() > 0) {
          joinedByComma.append(',');
        }
        joinedByComma.append(format);
      }
      intentScan.putExtra("SCAN_FORMATS", joinedByComma.toString());
    }

        if (cameraId >= 0) {
      intentScan.putExtra("SCAN_CAMERA_ID", cameraId);
    }

    String targetAppPackage = findTargetAppPackage(intentScan);
    if (targetAppPackage == null) {
      return showDownloadDialog();
    }
    intentScan.setPackage(targetAppPackage);
    intentScan.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
    intentScan.addFlags(FLAG_NEW_DOC);
    attachMoreExtras(intentScan);
    startActivityForResult(intentScan, REQUEST_CODE);
    return null;
  }

  
  protected void startActivityForResult(Intent intent, int code) {
    if (fragment == null) {
      activity.startActivityForResult(intent, code);
    } else {
      fragment.startActivityForResult(intent, code);
    }
  }
  
  private String findTargetAppPackage(Intent intent) {
    PackageManager pm = activity.getPackageManager();
    List<ResolveInfo> availableApps = pm.queryIntentActivities(intent, PackageManager.MATCH_DEFAULT_ONLY);
    if (availableApps != null) {
      for (String targetApp : targetApplications) {
        if (contains(availableApps, targetApp)) {
          return targetApp;
        }
      }
    }
    return null;
  }
  
  private static boolean contains(Iterable<ResolveInfo> availableApps, String targetApp) {
    for (ResolveInfo availableApp : availableApps) {
      String packageName = availableApp.activityInfo.packageName;
      if (targetApp.equals(packageName)) {
        return true;
      }
    }
    return false;
  }

  private AlertDialog showDownloadDialog() {
    AlertDialog.Builder downloadDialog = new AlertDialog.Builder(activity);
    downloadDialog.setTitle(title);
    downloadDialog.setMessage(message);
    downloadDialog.setPositiveButton(buttonYes, new DialogInterface.OnClickListener() {
      @Override
      public void onClick(DialogInterface dialogInterface, int i) {
        String packageName;
        if (targetApplications.contains(BS_PACKAGE)) {
                    packageName = BS_PACKAGE;
        } else {
                    packageName = targetApplications.get(0);
        }
        Uri uri = Uri.parse("market:        Intent intent = new Intent(Intent.ACTION_VIEW, uri);
        try {
          if (fragment == null) {
            activity.startActivity(intent);
          } else {
            fragment.startActivity(intent);
          }
        } catch (ActivityNotFoundException anfe) {
                    Log.w(TAG, "Google Play is not installed; cannot install " + packageName);
        }
      }
    });
    downloadDialog.setNegativeButton(buttonNo, null);
    downloadDialog.setCancelable(true);
    return downloadDialog.show();
  }


  
  public static IntentResult parseActivityResult(int requestCode, int resultCode, Intent intent) {
    if (requestCode == REQUEST_CODE) {
      if (resultCode == Activity.RESULT_OK) {
        String contents = intent.getStringExtra("SCAN_RESULT");
        String formatName = intent.getStringExtra("SCAN_RESULT_FORMAT");
        byte[] rawBytes = intent.getByteArrayExtra("SCAN_RESULT_BYTES");
        int intentOrientation = intent.getIntExtra("SCAN_RESULT_ORIENTATION", Integer.MIN_VALUE);
        Integer orientation = intentOrientation == Integer.MIN_VALUE ? null : intentOrientation;
        String errorCorrectionLevel = intent.getStringExtra("SCAN_RESULT_ERROR_CORRECTION_LEVEL");
        return new IntentResult(contents,
                                formatName,
                                rawBytes,
                                orientation,
                                errorCorrectionLevel);
      }
      return new IntentResult();
    }
    return null;
  }


  
  public final AlertDialog shareText(CharSequence text) {
    return shareText(text, "TEXT_TYPE");
  }

  
  public final AlertDialog shareText(CharSequence text, CharSequence type) {
    Intent intent = new Intent();
    intent.addCategory(Intent.CATEGORY_DEFAULT);
    intent.setAction(BS_PACKAGE + ".ENCODE");
    intent.putExtra("ENCODE_TYPE", type);
    intent.putExtra("ENCODE_DATA", text);
    String targetAppPackage = findTargetAppPackage(intent);
    if (targetAppPackage == null) {
      return showDownloadDialog();
    }
    intent.setPackage(targetAppPackage);
    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
    intent.addFlags(FLAG_NEW_DOC);
    attachMoreExtras(intent);
    if (fragment == null) {
      activity.startActivity(intent);
    } else {
      fragment.startActivity(intent);
    }
    return null;
  }
  
  private static List<String> list(String... values) {
    return Collections.unmodifiableList(Arrays.asList(values));
  }

  private void attachMoreExtras(Intent intent) {
    for (Map.Entry<String,Object> entry : moreExtras.entrySet()) {
      String key = entry.getKey();
      Object value = entry.getValue();
            if (value instanceof Integer) {
        intent.putExtra(key, (Integer) value);
      } else if (value instanceof Long) {
        intent.putExtra(key, (Long) value);
      } else if (value instanceof Boolean) {
        intent.putExtra(key, (Boolean) value);
      } else if (value instanceof Double) {
        intent.putExtra(key, (Double) value);
      } else if (value instanceof Float) {
        intent.putExtra(key, (Float) value);
      } else if (value instanceof Bundle) {
        intent.putExtra(key, (Bundle) value);
      } else {
        intent.putExtra(key, value.toString());
      }
    }
  }

}
