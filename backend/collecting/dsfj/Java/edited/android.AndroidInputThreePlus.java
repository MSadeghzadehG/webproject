

package com.badlogic.gdx.backends.android;

import java.util.ArrayList;

import android.content.Context;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnGenericMotionListener;

import com.badlogic.gdx.Application;


public class AndroidInputThreePlus extends AndroidInput implements OnGenericMotionListener {
	ArrayList<OnGenericMotionListener> genericMotionListeners = new ArrayList();
	private final AndroidMouseHandler mouseHandler;

	public AndroidInputThreePlus (Application activity, Context context, Object view, AndroidApplicationConfiguration config) {
		super(activity, context, view, config);
						if (view instanceof View) {
			View v = (View)view;
			v.setOnGenericMotionListener(this);
		}
		mouseHandler = new AndroidMouseHandler();
	}

	@Override
	public boolean onGenericMotion (View view, MotionEvent event) {
		if (mouseHandler.onGenericMotion(event, this)) return true;
		for (int i = 0, n = genericMotionListeners.size(); i < n; i++)
			if (genericMotionListeners.get(i).onGenericMotion(view, event)) return true;
		return false;
	}

	public void addGenericMotionListener (OnGenericMotionListener listener) {
		genericMotionListeners.add(listener);
	}
}
