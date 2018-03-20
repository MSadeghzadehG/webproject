

package com.google.zxing.client.android.book;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;

import java.util.List;

import com.google.zxing.client.android.R;


final class SearchBookContentsAdapter extends ArrayAdapter<SearchBookContentsResult> {

  SearchBookContentsAdapter(Context context, List<SearchBookContentsResult> items) {
    super(context, R.layout.search_book_contents_list_item, 0, items);
  }

  @Override
  public View getView(int position, View view, ViewGroup viewGroup) {
    SearchBookContentsListItem listItem;

    if (view == null) {
      LayoutInflater factory = LayoutInflater.from(getContext());
      listItem = (SearchBookContentsListItem) factory.inflate(
          R.layout.search_book_contents_list_item, viewGroup, false);
    } else {
      if (view instanceof SearchBookContentsListItem) {
        listItem = (SearchBookContentsListItem) view;
      } else {
        return view;
      }
    }

    SearchBookContentsResult result = getItem(position);
    listItem.set(result);
    return listItem;
  }
}
