

package com.badlogic.gdx.controllers;

import java.util.Collection;

import com.badlogic.gdx.utils.Array;


public interface ControllerManager {
	public Array<Controller> getControllers ();

	public void addListener (ControllerListener listener);

	public void removeListener (ControllerListener listener);
	
	public Array<ControllerListener> getListeners();

	
	public void clearListeners ();
}
