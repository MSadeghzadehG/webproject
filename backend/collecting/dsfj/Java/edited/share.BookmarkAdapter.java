

package com.google.zxing.client.android.share;

import java.util.List;

import com.google.zxing.client.android.R;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.TextView;


final class BookmarkAdapter extends BaseAdapter {

  private final Context context;
  private final List<String[]> titleURLs;

  BookmarkAdapter(Context context, List<String[]> titleURLs) {
    this.context = context;
    this.titleURLs = titleURLs;
  }

  @Override
  public int getCount() {
    return titleURLs.size();
  }

  @Override
  public Object getItem(int index) {
    return titleURLs.get(index);
  }

  @Override
  public long getItemId(int index) {
    return index;
  }

  @Override
  public View getView(int index, View view, ViewGroup viewGroup) {
    View layout;
    if (view instanceof LinearLayout) {
      layout = view;
    } else {
      LayoutInflater factory = LayoutInflater.from(context);
      layout = factory.inflate(R.layout.bookmark_picker_list_item, viewGroup, false);
    }
    String[] titleURL = titleURLs.get(index);
    ((TextView) layout.findViewById(R.id.bookmark_title)).setText(titleURL[0]);
    ((TextView) layout.findViewById(R.id.bookmark_url)).setText(titleURL[1]);
    return layout;
  }
}
