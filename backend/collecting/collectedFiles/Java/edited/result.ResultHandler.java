

package com.google.zxing.client.android.result;

import android.telephony.PhoneNumberUtils;
import com.google.zxing.Result;
import com.google.zxing.client.android.Contents;
import com.google.zxing.client.android.Intents;
import com.google.zxing.client.android.LocaleManager;
import com.google.zxing.client.android.PreferencesActivity;
import com.google.zxing.client.android.R;
import com.google.zxing.client.android.book.SearchBookContentsActivity;
import com.google.zxing.client.result.ParsedResult;
import com.google.zxing.client.result.ParsedResultType;
import com.google.zxing.client.result.ResultParser;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentValues;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.provider.ContactsContract;
import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.Locale;
import java.util.ArrayList;


public abstract class ResultHandler {

  private static final String TAG = ResultHandler.class.getSimpleName();

  private static final String[] EMAIL_TYPE_STRINGS = {"home", "work", "mobile"};
  private static final String[] PHONE_TYPE_STRINGS = {"home", "work", "mobile", "fax", "pager", "main"};
  private static final String[] ADDRESS_TYPE_STRINGS = {"home", "work"};
  private static final int[] EMAIL_TYPE_VALUES = {
      ContactsContract.CommonDataKinds.Email.TYPE_HOME,
      ContactsContract.CommonDataKinds.Email.TYPE_WORK,
      ContactsContract.CommonDataKinds.Email.TYPE_MOBILE,
  };
  private static final int[] PHONE_TYPE_VALUES = {
      ContactsContract.CommonDataKinds.Phone.TYPE_HOME,
      ContactsContract.CommonDataKinds.Phone.TYPE_WORK,
      ContactsContract.CommonDataKinds.Phone.TYPE_MOBILE,
      ContactsContract.CommonDataKinds.Phone.TYPE_FAX_WORK,
      ContactsContract.CommonDataKinds.Phone.TYPE_PAGER,
      ContactsContract.CommonDataKinds.Phone.TYPE_MAIN,
  };
  private static final int[] ADDRESS_TYPE_VALUES = {
      ContactsContract.CommonDataKinds.StructuredPostal.TYPE_HOME,
      ContactsContract.CommonDataKinds.StructuredPostal.TYPE_WORK,
  };
  private static final int NO_TYPE = -1;

  public static final int MAX_BUTTON_COUNT = 4;

  private final ParsedResult result;
  private final Activity activity;
  private final Result rawResult;
  private final String customProductSearch;

  ResultHandler(Activity activity, ParsedResult result) {
    this(activity, result, null);
  }

  ResultHandler(Activity activity, ParsedResult result, Result rawResult) {
    this.result = result;
    this.activity = activity;
    this.rawResult = rawResult;
    this.customProductSearch = parseCustomSearchURL();
  }

  public final ParsedResult getResult() {
    return result;
  }

  final boolean hasCustomProductSearch() {
    return customProductSearch != null;
  }

  final Activity getActivity() {
    return activity;
  }

  
  public abstract int getButtonCount();

  
  public abstract int getButtonText(int index);

  public Integer getDefaultButtonID() {
    return null;
  }

  
  public abstract void handleButtonPress(int index);

  
  public boolean areContentsSecure() {
    return false;
  }

  
  public CharSequence getDisplayContents() {
    String contents = result.getDisplayResult();
    return contents.replace("\r", "");
  }

  
  public abstract int getDisplayTitle();

  
  public final ParsedResultType getType() {
    return result.getType();
  }

  final void addPhoneOnlyContact(String[] phoneNumbers,String[] phoneTypes) {
    addContact(null, null, null, phoneNumbers, phoneTypes, null, null, null, null, null, null, null, null, null, null, null);
  }

  final void addEmailOnlyContact(String[] emails, String[] emailTypes) {
    addContact(null, null, null, null, null, emails, emailTypes, null, null, null, null, null, null, null, null, null);
  }

  final void addContact(String[] names,
                        String[] nicknames,
                        String pronunciation,
                        String[] phoneNumbers,
                        String[] phoneTypes,
                        String[] emails,
                        String[] emailTypes,
                        String note,
                        String instantMessenger,
                        String address,
                        String addressType,
                        String org,
                        String title,
                        String[] urls,
                        String birthday,
                        String[] geo) {

        Intent intent = new Intent(Intent.ACTION_INSERT_OR_EDIT, ContactsContract.Contacts.CONTENT_URI);
    intent.setType(ContactsContract.Contacts.CONTENT_ITEM_TYPE);
    putExtra(intent, ContactsContract.Intents.Insert.NAME, names != null && names.length > 0 ? names[0] : null);

    putExtra(intent, ContactsContract.Intents.Insert.PHONETIC_NAME, pronunciation);

    int phoneCount = Math.min(phoneNumbers != null ? phoneNumbers.length : 0, Contents.PHONE_KEYS.length);
    for (int x = 0; x < phoneCount; x++) {
      putExtra(intent, Contents.PHONE_KEYS[x], phoneNumbers[x]);
      if (phoneTypes != null && x < phoneTypes.length) {
        int type = toPhoneContractType(phoneTypes[x]);
        if (type >= 0) {
          intent.putExtra(Contents.PHONE_TYPE_KEYS[x], type);
        }
      }
    }

    int emailCount = Math.min(emails != null ? emails.length : 0, Contents.EMAIL_KEYS.length);
    for (int x = 0; x < emailCount; x++) {
      putExtra(intent, Contents.EMAIL_KEYS[x], emails[x]);
      if (emailTypes != null && x < emailTypes.length) {
        int type = toEmailContractType(emailTypes[x]);
        if (type >= 0) {
          intent.putExtra(Contents.EMAIL_TYPE_KEYS[x], type);
        }
      }
    }

    ArrayList<ContentValues> data = new ArrayList<>();
    if (urls != null) {
      for (String url : urls) {
        if (url != null && !url.isEmpty()) {
          ContentValues row = new ContentValues(2);
          row.put(ContactsContract.Data.MIMETYPE, ContactsContract.CommonDataKinds.Website.CONTENT_ITEM_TYPE);
          row.put(ContactsContract.CommonDataKinds.Website.URL, url);
          data.add(row);
          break;
        }
      }
    }

    if (birthday != null) {
      ContentValues row = new ContentValues(3);
      row.put(ContactsContract.Data.MIMETYPE, ContactsContract.CommonDataKinds.Event.CONTENT_ITEM_TYPE);
      row.put(ContactsContract.CommonDataKinds.Event.TYPE, ContactsContract.CommonDataKinds.Event.TYPE_BIRTHDAY);
      row.put(ContactsContract.CommonDataKinds.Event.START_DATE, birthday);
      data.add(row);
    }

    if (nicknames != null) {
      for (String nickname : nicknames) {
        if (nickname != null && !nickname.isEmpty()) {
          ContentValues row = new ContentValues(3);
          row.put(ContactsContract.Data.MIMETYPE, ContactsContract.CommonDataKinds.Nickname.CONTENT_ITEM_TYPE);
          row.put(ContactsContract.CommonDataKinds.Nickname.TYPE,
                  ContactsContract.CommonDataKinds.Nickname.TYPE_DEFAULT);
          row.put(ContactsContract.CommonDataKinds.Nickname.NAME, nickname);
          data.add(row);
          break;
        }
      }
    }

    if (!data.isEmpty()) {
      intent.putParcelableArrayListExtra(ContactsContract.Intents.Insert.DATA, data);
    }

    StringBuilder aggregatedNotes = new StringBuilder();
    if (note != null) {
      aggregatedNotes.append('\n').append(note);
    }
    if (geo != null && geo.length >= 2) {
      aggregatedNotes.append('\n').append(geo[0]).append(',').append(geo[1]);
    }

    if (aggregatedNotes.length() > 0) {
            putExtra(intent, ContactsContract.Intents.Insert.NOTES, aggregatedNotes.substring(1));
    }
    
    putExtra(intent, ContactsContract.Intents.Insert.IM_HANDLE, instantMessenger);
    putExtra(intent, ContactsContract.Intents.Insert.POSTAL, address);
    if (addressType != null) {
      int type = toAddressContractType(addressType);
      if (type >= 0) {
        intent.putExtra(ContactsContract.Intents.Insert.POSTAL_TYPE, type);
      }
    }
    putExtra(intent, ContactsContract.Intents.Insert.COMPANY, org);
    putExtra(intent, ContactsContract.Intents.Insert.JOB_TITLE, title);
    launchIntent(intent);
  }

  private static int toEmailContractType(String typeString) {
    return doToContractType(typeString, EMAIL_TYPE_STRINGS, EMAIL_TYPE_VALUES);
  }

  private static int toPhoneContractType(String typeString) {
    return doToContractType(typeString, PHONE_TYPE_STRINGS, PHONE_TYPE_VALUES);
  }

  private static int toAddressContractType(String typeString) {
    return doToContractType(typeString, ADDRESS_TYPE_STRINGS, ADDRESS_TYPE_VALUES);
  }

  private static int doToContractType(String typeString, String[] types, int[] values) {
    if (typeString == null) {
      return NO_TYPE;
    }
    for (int i = 0; i < types.length; i++) {
      String type = types[i];
      if (typeString.startsWith(type) || typeString.startsWith(type.toUpperCase(Locale.ENGLISH))) {
        return values[i];
      }
    }
    return NO_TYPE;
  }

  final void shareByEmail(String contents) {
    sendEmail(null, null, null, null, contents);
  }

  final void sendEmail(String[] to,
                       String[] cc,
                       String[] bcc,
                       String subject,
                       String body) {
    Intent intent = new Intent(Intent.ACTION_SEND, Uri.parse("mailto:"));
    if (to != null && to.length != 0) {
      intent.putExtra(Intent.EXTRA_EMAIL, to);
    }
    if (cc != null && cc.length != 0) {
      intent.putExtra(Intent.EXTRA_CC, cc);
    }
    if (bcc != null && bcc.length != 0) {
      intent.putExtra(Intent.EXTRA_BCC, bcc);
    }
    putExtra(intent, Intent.EXTRA_SUBJECT, subject);
    putExtra(intent, Intent.EXTRA_TEXT, body);
    intent.setType("text/plain");
    launchIntent(intent);
  }

  final void shareBySMS(String contents) {
    sendSMSFromUri("smsto:", contents);
  }

  final void sendSMS(String phoneNumber, String body) {
    sendSMSFromUri("smsto:" + phoneNumber, body);
  }

  private void sendSMSFromUri(String uri, String body) {
    Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.parse(uri));
    putExtra(intent, "sms_body", body);
        intent.putExtra("compose_mode", true);
    launchIntent(intent);
  }

  final void sendMMS(String phoneNumber, String subject, String body) {
    sendMMSFromUri("mmsto:" + phoneNumber, subject, body);
  }

  private void sendMMSFromUri(String uri, String subject, String body) {
    Intent intent = new Intent(Intent.ACTION_SENDTO, Uri.parse(uri));
        if (subject == null || subject.isEmpty()) {
      putExtra(intent, "subject", activity.getString(R.string.msg_default_mms_subject));
    } else {
      putExtra(intent, "subject", subject);
    }
    putExtra(intent, "sms_body", body);
    intent.putExtra("compose_mode", true);
    launchIntent(intent);
  }

  final void dialPhone(String phoneNumber) {
    launchIntent(new Intent(Intent.ACTION_DIAL, Uri.parse("tel:" + phoneNumber)));
  }

  final void dialPhoneFromUri(String uri) {
    launchIntent(new Intent(Intent.ACTION_DIAL, Uri.parse(uri)));
  }

  final void openMap(String geoURI) {
    launchIntent(new Intent(Intent.ACTION_VIEW, Uri.parse(geoURI)));
  }

  
  final void searchMap(String address) {
    launchIntent(new Intent(Intent.ACTION_VIEW, Uri.parse("geo:0,0?q=" + Uri.encode(address))));
  }

  final void getDirections(double latitude, double longitude) {
    launchIntent(new Intent(Intent.ACTION_VIEW, Uri.parse("http:        LocaleManager.getCountryTLD(activity) + "/maps?f=d&daddr=" + latitude + ',' + longitude)));
  }

    final void openProductSearch(String upc) {
    Uri uri = Uri.parse("http:        "/m/products?q=" + upc + "&source=zxing");
    launchIntent(new Intent(Intent.ACTION_VIEW, uri));
  }

  final void openBookSearch(String isbn) {
    Uri uri = Uri.parse("http:        "/books?vid=isbn" + isbn);
    launchIntent(new Intent(Intent.ACTION_VIEW, uri));
  }

  final void searchBookContents(String isbnOrUrl) {
    Intent intent = new Intent(Intents.SearchBookContents.ACTION);
    intent.setClassName(activity, SearchBookContentsActivity.class.getName());
    putExtra(intent, Intents.SearchBookContents.ISBN, isbnOrUrl);
    launchIntent(intent);
  }

  final void openURL(String url) {
            if (url.startsWith("HTTP:      url = "http" + url.substring(4);
    } else if (url.startsWith("HTTPS:      url = "https" + url.substring(5);
    }
    Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
    try {
      launchIntent(intent);
    } catch (ActivityNotFoundException ignored) {
      Log.w(TAG, "Nothing available to handle " + intent);
    }
  }

  final void webSearch(String query) {
    Intent intent = new Intent(Intent.ACTION_WEB_SEARCH);
    intent.putExtra("query", query);
    launchIntent(intent);
  }

  
  final void rawLaunchIntent(Intent intent) {
    if (intent != null) {
      intent.addFlags(Intents.FLAG_NEW_DOC);
      Log.d(TAG, "Launching intent: " + intent + " with extras: " + intent.getExtras());
      activity.startActivity(intent);
    }
  }

  
  final void launchIntent(Intent intent) {
    try {
      rawLaunchIntent(intent);
    } catch (ActivityNotFoundException ignored) {
      AlertDialog.Builder builder = new AlertDialog.Builder(activity);
      builder.setTitle(R.string.app_name);
      builder.setMessage(R.string.msg_intent_failed);
      builder.setPositiveButton(R.string.button_ok, null);
      builder.show();
    }
  }

  private static void putExtra(Intent intent, String key, String value) {
    if (value != null && !value.isEmpty()) {
      intent.putExtra(key, value);
    }
  }

  private String parseCustomSearchURL() {
    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(activity);
    String customProductSearch = prefs.getString(PreferencesActivity.KEY_CUSTOM_PRODUCT_SEARCH,
        null);
    if (customProductSearch != null && customProductSearch.trim().isEmpty()) {
      return null;
    }
    return customProductSearch;
  }

  final String fillInCustomSearchURL(String text) {
    if (customProductSearch == null) {
      return text;     }
    try {
      text = URLEncoder.encode(text, "UTF-8");
    } catch (UnsupportedEncodingException e) {
          }
    String url = customProductSearch;
    if (rawResult != null) {
                  url = url.replaceFirst("%f(?![0-9a-f])", rawResult.getBarcodeFormat().toString());
      if (url.contains("%t")) {
        ParsedResult parsedResultAgain = ResultParser.parseResult(rawResult);
        url = url.replace("%t", parsedResultAgain.getType().toString());
      }
    }
        return url.replace("%s", text);
  }

  static String formatPhone(String phoneData) {
        return PhoneNumberUtils.formatNumber(phoneData);
  }

}
