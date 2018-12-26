

package com.google.zxing.client.android.share;

import android.app.ListActivity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageItemInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListAdapter;
import com.google.zxing.client.android.R;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


final class LoadPackagesAsyncTask extends AsyncTask<Object,Object,List<AppInfo>> {

  private static final String[] PKG_PREFIX_WHITELIST = {
      "com.google.android.apps.",
  };
  private static final String[] PKG_PREFIX_BLACKLIST = {
      "com.android.",
      "android",
      "com.google.android.",
      "com.htc",
  };

  private final ListActivity activity;

  LoadPackagesAsyncTask(ListActivity activity) {
    this.activity = activity;
  }

  @Override
  protected List<AppInfo> doInBackground(Object... objects) {
    List<AppInfo> labelsPackages = new ArrayList<>();
    PackageManager packageManager = activity.getPackageManager();
    Iterable<ApplicationInfo> appInfos = packageManager.getInstalledApplications(0);
    for (PackageItemInfo appInfo : appInfos) {
      String packageName = appInfo.packageName;
      if (!isHidden(packageName)) {
        CharSequence label = appInfo.loadLabel(packageManager);
        Drawable icon = appInfo.loadIcon(packageManager);        
        if (label != null) {
          labelsPackages.add(new AppInfo(packageName, label.toString(), icon));
        }
      }
    }
    Collections.sort(labelsPackages);
    return labelsPackages;
  }

  private static boolean isHidden(String packageName) {
    if (packageName == null) {
      return true;
    }
    for (String prefix : PKG_PREFIX_WHITELIST) {
      if (packageName.startsWith(prefix)) {
        return false;
      }
    }
    for (String prefix : PKG_PREFIX_BLACKLIST) {
      if (packageName.startsWith(prefix)) {
        return true;
      }
    }
    return false;
  }

  @Override
  protected void onPostExecute(final List<AppInfo> results) {    
    ListAdapter listAdapter = new ArrayAdapter<AppInfo>(activity, 
                                                        R.layout.app_picker_list_item, 
                                                        R.id.app_picker_list_item_label, 
                                                        results) {
      @Override
      public View getView(int position, View convertView, ViewGroup parent) {
        View view = super.getView(position, convertView, parent);
        Drawable icon = results.get(position).getIcon();
        if (icon != null) {
          ((ImageView) view.findViewById(R.id.app_picker_list_item_icon)).setImageDrawable(icon);
        }
        return view;
      }
    };
    activity.setListAdapter(listAdapter);
  }

}
