

package com.badlogic.gdx.controllers;

import java.util.Collection;

import com.badlogic.gdx.Application;
import com.badlogic.gdx.ApplicationListener;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.Graphics.GraphicsType;
import com.badlogic.gdx.LifecycleListener;
import com.badlogic.gdx.Application.ApplicationType;
import com.badlogic.gdx.utils.Array;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.ObjectMap;
import com.badlogic.gdx.utils.reflect.ClassReflection;


public class Controllers {
	private static final String TAG = "Controllers";
	static final ObjectMap<Application, ControllerManager> managers = new ObjectMap<Application, ControllerManager>();

	
	static public Array<Controller> getControllers () {
		initialize();
		return getManager().getControllers();
	}

	
	static public void addListener (ControllerListener listener) {
		initialize();
		getManager().addListener(listener);
	}

	
	static public void removeListener (ControllerListener listener) {
		initialize();
		getManager().removeListener(listener);
	}
	
	
	static public void clearListeners () {
		initialize();
		getManager().clearListeners();
	}
	
	
	static public Array<ControllerListener> getListeners() {
		initialize();
		return getManager().getListeners();
	}

	static private ControllerManager getManager () {
		return managers.get(Gdx.app);
	}

	static private void initialize () {
		if (managers.containsKey(Gdx.app)) return;

		String className = null;
		ApplicationType type = Gdx.app.getType();
		ControllerManager manager = null;

		if (type == ApplicationType.Android) {
			if (Gdx.app.getVersion() >= 12) {
				className = "com.badlogic.gdx.controllers.android.AndroidControllers";
			} else {
				Gdx.app.log(TAG, "No controller manager is available for Android versions < API level 12");
				manager = new ControllerManagerStub();
			}
		} else if (type == ApplicationType.Desktop) {
			if(Gdx.graphics.getType() == GraphicsType.LWJGL3) {
				className = "com.badlogic.gdx.controllers.lwjgl3.Lwjgl3ControllerManager";
			} else {
				className = "com.badlogic.gdx.controllers.desktop.DesktopControllerManager";
			}
		} else if (type == ApplicationType.WebGL) {
			className = "com.badlogic.gdx.controllers.gwt.GwtControllers";
		} else {
			Gdx.app.log(TAG, "No controller manager is available for: " + Gdx.app.getType());
			manager = new ControllerManagerStub();
		}

		if (manager == null) {
			try {
				Class controllerManagerClass = ClassReflection.forName(className);
				manager = (ControllerManager)ClassReflection.newInstance(controllerManagerClass);
			} catch (Throwable ex) {
				throw new GdxRuntimeException("Error creating controller manager: " + className, ex);
			}
		}

		managers.put(Gdx.app, manager);
		final Application app = Gdx.app;
		Gdx.app.addLifecycleListener(new LifecycleListener() {
			@Override
			public void resume () {
			}

			@Override
			public void pause () {
			}

			@Override
			public void dispose () {
				managers.remove(app);
				Gdx.app.log(TAG, "removed manager for application, " + managers.size + " managers active");

			}
		});
		Gdx.app.log(TAG, "added manager for application, " + managers.size + " managers active");
	}
}
