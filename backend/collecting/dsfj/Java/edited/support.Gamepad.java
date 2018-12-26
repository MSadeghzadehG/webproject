

package com.badlogic.gdx.controllers.gwt.support;

import com.google.gwt.core.client.JavaScriptObject;
import com.google.gwt.core.client.JsArray;
import com.google.gwt.core.client.JsArrayNumber;

public final class Gamepad extends JavaScriptObject {
	
	protected Gamepad() {
			}
	
	public native String getId() ;

	public native int getIndex() ;

	public native boolean getConnected() ;

	public native double getTimestamp() ;

	public native String getMapping() ;

	public native JsArrayNumber getAxes() ;
	
	public native JsArray<GamepadButton> getButtons() ;

	public native double getPreviousTimestamp() ;

	public native void setPreviousTimestamp(double previousTimestamp) ;

	public static Gamepad getGamepad(int index) {
		return GamepadSupport.getGamepad(index);		
	}

}