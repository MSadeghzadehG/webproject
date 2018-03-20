

package com.google.zxing.client.android.share;

import java.util.ArrayList;
import java.util.List;

import android.app.ListActivity;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import com.google.zxing.client.android.Intents;


public final class BookmarkPickerActivity extends ListActivity {

  private static final String TAG = BookmarkPickerActivity.class.getSimpleName();

  private static final String[] BOOKMARK_PROJECTION = {
      "title",       "url",   };
    private static final Uri BOOKMARKS_URI = Uri.parse("content:
  private static final String BOOKMARK_SELECTION = "bookmark = 1 AND url IS NOT NULL";

  private final List<String[]> titleURLs = new ArrayList<>();

  @Override
  protected void onResume() {
    super.onResume();
    titleURLs.clear();
    try (Cursor cursor = getContentResolver().query(BOOKMARKS_URI, BOOKMARK_PROJECTION,
             BOOKMARK_SELECTION, null, null)) {
      if (cursor == null) {
        Log.w(TAG, "No cursor returned for bookmark query");
        finish();
        return;
      }
      while (cursor.moveToNext()) {
        titleURLs.add(new String[] { cursor.getString(0), cursor.getString(1) });
      }
    }
    setListAdapter(new BookmarkAdapter(this, titleURLs));
  }


  @Override
  protected void onListItemClick(ListView l, View view, int position, long id) {
    String[] titleURL = titleURLs.get(position);
    Intent intent = new Intent();
    intent.addFlags(Intents.FLAG_NEW_DOC);
    intent.putExtra("title", titleURL[0]);     intent.putExtra("url", titleURL[1]);     setResult(RESULT_OK, intent);
    finish();
  }
}
