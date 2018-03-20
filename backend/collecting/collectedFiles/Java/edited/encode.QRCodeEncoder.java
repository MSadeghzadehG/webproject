

package com.google.zxing.client.android.encode;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.Result;
import com.google.zxing.WriterException;
import com.google.zxing.client.android.Contents;
import com.google.zxing.client.android.Intents;
import com.google.zxing.client.android.R;
import com.google.zxing.client.result.AddressBookParsedResult;
import com.google.zxing.client.result.ParsedResult;
import com.google.zxing.client.result.ResultParser;
import com.google.zxing.common.BitMatrix;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;


final class QRCodeEncoder {

  private static final String TAG = QRCodeEncoder.class.getSimpleName();

  private static final int WHITE = 0xFFFFFFFF;
  private static final int BLACK = 0xFF000000;

  private final Context activity;
  private String contents;
  private String displayContents;
  private String title;
  private BarcodeFormat format;
  private final int dimension;
  private final boolean useVCard;

  QRCodeEncoder(Context activity, Intent intent, int dimension, boolean useVCard) throws WriterException {
    this.activity = activity;
    this.dimension = dimension;
    this.useVCard = useVCard;
    String action = intent.getAction();
    if (Intents.Encode.ACTION.equals(action)) {
      encodeContentsFromZXingIntent(intent);
    } else if (Intent.ACTION_SEND.equals(action)) {
      encodeContentsFromShareIntent(intent);
    }
  }

  String getContents() {
    return contents;
  }

  String getDisplayContents() {
    return displayContents;
  }

  String getTitle() {
    return title;
  }

  boolean isUseVCard() {
    return useVCard;
  }

      private void encodeContentsFromZXingIntent(Intent intent) {
         String formatString = intent.getStringExtra(Intents.Encode.FORMAT);
    format = null;
    if (formatString != null) {
      try {
        format = BarcodeFormat.valueOf(formatString);
      } catch (IllegalArgumentException iae) {
              }
    }
    if (format == null || format == BarcodeFormat.QR_CODE) {
      String type = intent.getStringExtra(Intents.Encode.TYPE);
      if (type != null && !type.isEmpty()) {
        this.format = BarcodeFormat.QR_CODE;
        encodeQRCodeContents(intent, type);
      }
    } else {
      String data = intent.getStringExtra(Intents.Encode.DATA);
      if (data != null && !data.isEmpty()) {
        contents = data;
        displayContents = data;
        title = activity.getString(R.string.contents_text);
      }
    }
  }

    private void encodeContentsFromShareIntent(Intent intent) throws WriterException {
        if (intent.hasExtra(Intent.EXTRA_STREAM)) {
      encodeFromStreamExtra(intent);
    } else {
      encodeFromTextExtras(intent);
    }
  }

  private void encodeFromTextExtras(Intent intent) throws WriterException {
        String theContents = ContactEncoder.trim(intent.getStringExtra(Intent.EXTRA_TEXT));
    if (theContents == null) {
      theContents = ContactEncoder.trim(intent.getStringExtra("android.intent.extra.HTML_TEXT"));
            if (theContents == null) {
        theContents = ContactEncoder.trim(intent.getStringExtra(Intent.EXTRA_SUBJECT));
        if (theContents == null) {
          String[] emails = intent.getStringArrayExtra(Intent.EXTRA_EMAIL);
          if (emails != null) {
            theContents = ContactEncoder.trim(emails[0]);
          } else {
            theContents = "?";
          }
        }
      }
    }

        if (theContents == null || theContents.isEmpty()) {
      throw new WriterException("Empty EXTRA_TEXT");
    }
    contents = theContents;
        format = BarcodeFormat.QR_CODE;
    if (intent.hasExtra(Intent.EXTRA_SUBJECT)) {
      displayContents = intent.getStringExtra(Intent.EXTRA_SUBJECT);
    } else if (intent.hasExtra(Intent.EXTRA_TITLE)) {
      displayContents = intent.getStringExtra(Intent.EXTRA_TITLE);
    } else {
      displayContents = contents;
    }
    title = activity.getString(R.string.contents_text);
  }

    private void encodeFromStreamExtra(Intent intent) throws WriterException {
    format = BarcodeFormat.QR_CODE;
    Bundle bundle = intent.getExtras();
    if (bundle == null) {
      throw new WriterException("No extras");
    }
    Uri uri = bundle.getParcelable(Intent.EXTRA_STREAM);
    if (uri == null) {
      throw new WriterException("No EXTRA_STREAM");
    }
    byte[] vcard;
    String vcardString;
    try (InputStream stream = activity.getContentResolver().openInputStream(uri)) {
      if (stream == null) {
        throw new WriterException("Can't open stream for " + uri);
      }
      ByteArrayOutputStream baos = new ByteArrayOutputStream();
      byte[] buffer = new byte[2048];
      int bytesRead;
      while ((bytesRead = stream.read(buffer)) > 0) {
        baos.write(buffer, 0, bytesRead);
      }
      vcard = baos.toByteArray();
      vcardString = new String(vcard, 0, vcard.length, "UTF-8");
    } catch (IOException ioe) {
      throw new WriterException(ioe);
    }
    Log.d(TAG, "Encoding share intent content:");
    Log.d(TAG, vcardString);
    Result result = new Result(vcardString, vcard, null, BarcodeFormat.QR_CODE);
    ParsedResult parsedResult = ResultParser.parseResult(result);
    if (!(parsedResult instanceof AddressBookParsedResult)) {
      throw new WriterException("Result was not an address");
    }
    encodeQRCodeContents((AddressBookParsedResult) parsedResult);
    if (contents == null || contents.isEmpty()) {
      throw new WriterException("No content to encode");
    }
  }

  private void encodeQRCodeContents(Intent intent, String type) {
    switch (type) {
      case Contents.Type.TEXT:
        String textData = intent.getStringExtra(Intents.Encode.DATA);
        if (textData != null && !textData.isEmpty()) {
          contents = textData;
          displayContents = textData;
          title = activity.getString(R.string.contents_text);
        }
        break;

      case Contents.Type.EMAIL:
        String emailData = ContactEncoder.trim(intent.getStringExtra(Intents.Encode.DATA));
        if (emailData != null) {
          contents = "mailto:" + emailData;
          displayContents = emailData;
          title = activity.getString(R.string.contents_email);
        }
        break;

      case Contents.Type.PHONE:
        String phoneData = ContactEncoder.trim(intent.getStringExtra(Intents.Encode.DATA));
        if (phoneData != null) {
          contents = "tel:" + phoneData;
          displayContents = ContactEncoder.formatPhone(phoneData);
          title = activity.getString(R.string.contents_phone);
        }
        break;

      case Contents.Type.SMS:
        String smsData = ContactEncoder.trim(intent.getStringExtra(Intents.Encode.DATA));
        if (smsData != null) {
          contents = "sms:" + smsData;
          displayContents = ContactEncoder.formatPhone(smsData);
          title = activity.getString(R.string.contents_sms);
        }
        break;

      case Contents.Type.CONTACT:
        Bundle contactBundle = intent.getBundleExtra(Intents.Encode.DATA);
        if (contactBundle != null) {

          String name = contactBundle.getString(ContactsContract.Intents.Insert.NAME);
          String organization = contactBundle.getString(ContactsContract.Intents.Insert.COMPANY);
          String address = contactBundle.getString(ContactsContract.Intents.Insert.POSTAL);
          List<String> phones = getAllBundleValues(contactBundle, Contents.PHONE_KEYS);
          List<String> phoneTypes = getAllBundleValues(contactBundle, Contents.PHONE_TYPE_KEYS);
          List<String> emails = getAllBundleValues(contactBundle, Contents.EMAIL_KEYS);
          String url = contactBundle.getString(Contents.URL_KEY);
          List<String> urls = url == null ? null : Collections.singletonList(url);
          String note = contactBundle.getString(Contents.NOTE_KEY);

          ContactEncoder encoder = useVCard ? new VCardContactEncoder() : new MECARDContactEncoder();
          String[] encoded = encoder.encode(Collections.singletonList(name),
                                            organization,
                                            Collections.singletonList(address),
                                            phones,
                                            phoneTypes,
                                            emails,
                                            urls,
                                            note);
                    if (!encoded[1].isEmpty()) {
            contents = encoded[0];
            displayContents = encoded[1];
            title = activity.getString(R.string.contents_contact);
          }

        }
        break;

      case Contents.Type.LOCATION:
        Bundle locationBundle = intent.getBundleExtra(Intents.Encode.DATA);
        if (locationBundle != null) {
                    float latitude = locationBundle.getFloat("LAT", Float.MAX_VALUE);
          float longitude = locationBundle.getFloat("LONG", Float.MAX_VALUE);
          if (latitude != Float.MAX_VALUE && longitude != Float.MAX_VALUE) {
            contents = "geo:" + latitude + ',' + longitude;
            displayContents = latitude + "," + longitude;
            title = activity.getString(R.string.contents_location);
          }
        }
        break;
    }
  }

  private static List<String> getAllBundleValues(Bundle bundle, String[] keys) {
    List<String> values = new ArrayList<>(keys.length);
    for (String key : keys) {
      Object value = bundle.get(key);
      values.add(value == null ? null : value.toString());
    }
    return values;
  }

  private void encodeQRCodeContents(AddressBookParsedResult contact) {
    ContactEncoder encoder = useVCard ? new VCardContactEncoder() : new MECARDContactEncoder();
    String[] encoded = encoder.encode(toList(contact.getNames()),
                                      contact.getOrg(),
                                      toList(contact.getAddresses()),
                                      toList(contact.getPhoneNumbers()),
                                      null,
                                      toList(contact.getEmails()),
                                      toList(contact.getURLs()),
                                      null);
        if (!encoded[1].isEmpty()) {
      contents = encoded[0];
      displayContents = encoded[1];
      title = activity.getString(R.string.contents_contact);
    }
  }

  private static List<String> toList(String[] values) {
    return values == null ? null : Arrays.asList(values);
  }

  Bitmap encodeAsBitmap() throws WriterException {
    String contentsToEncode = contents;
    if (contentsToEncode == null) {
      return null;
    }
    Map<EncodeHintType,Object> hints = null;
    String encoding = guessAppropriateEncoding(contentsToEncode);
    if (encoding != null) {
      hints = new EnumMap<>(EncodeHintType.class);
      hints.put(EncodeHintType.CHARACTER_SET, encoding);
    }
    BitMatrix result;
    try {
      result = new MultiFormatWriter().encode(contentsToEncode, format, dimension, dimension, hints);
    } catch (IllegalArgumentException iae) {
            return null;
    }
    int width = result.getWidth();
    int height = result.getHeight();
    int[] pixels = new int[width * height];
    for (int y = 0; y < height; y++) {
      int offset = y * width;
      for (int x = 0; x < width; x++) {
        pixels[offset + x] = result.get(x, y) ? BLACK : WHITE;
      }
    }

    Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
    bitmap.setPixels(pixels, 0, width, 0, 0, width, height);
    return bitmap;
  }

  private static String guessAppropriateEncoding(CharSequence contents) {
        for (int i = 0; i < contents.length(); i++) {
      if (contents.charAt(i) > 0xFF) {
        return "UTF-8";
      }
    }
    return null;
  }

}
