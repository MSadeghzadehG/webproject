

package com.badlogic.gdx.controllers.desktop.ois;

import java.util.ArrayList;


public class Ois {
	private final long inputManagerPtr;
	private final ArrayList<OisJoystick> joysticks = new ArrayList();

	public Ois (long hwnd) {
		inputManagerPtr = createInputManager(hwnd);

		String[] names = getJoystickNames(inputManagerPtr);
		for (int i = 0, n = names.length; i < n; i++)
			joysticks.add(new OisJoystick(createJoystick(inputManagerPtr), names[i]));
	}

	public ArrayList<OisJoystick> getJoysticks () {
		return joysticks;
	}

	public void update () {
		for (int i = 0, n = joysticks.size(); i < n; i++)
			joysticks.get(i).update();
	}

	public int getVersionNumber () {
		return getVersionNumber(inputManagerPtr);
	}

	public String getVersionName () {
		return getVersionName(inputManagerPtr);
	}

	public String getInputSystemName () {
		return getInputSystemName(inputManagerPtr);
	}

		

	private native long createInputManager (long hwnd); 

	private native String[] getJoystickNames (long inputManagerPtr); 

	private native int getVersionNumber (long inputManagerPtr); 
	
	private native String getVersionName (long inputManagerPtr); 
	
	private native String getInputSystemName (long inputManagerPtr); 

	private native long createJoystick (long inputManagerPtr); 
}