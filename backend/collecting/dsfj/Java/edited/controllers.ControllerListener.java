

package com.badlogic.gdx.controllers;

import java.util.ResourceBundle.Control;

import com.badlogic.gdx.math.Vector3;


public interface ControllerListener {
	
	public void connected (Controller controller);

	
	public void disconnected (Controller controller);

	
	public boolean buttonDown (Controller controller, int buttonCode);

	
	public boolean buttonUp (Controller controller, int buttonCode);

	
	public boolean axisMoved (Controller controller, int axisCode, float value);

	
	public boolean povMoved (Controller controller, int povCode, PovDirection value);

	
	public boolean xSliderMoved (Controller controller, int sliderCode, boolean value);

	
	public boolean ySliderMoved (Controller controller, int sliderCode, boolean value);

	
	public boolean accelerometerMoved (Controller controller, int accelerometerCode, Vector3 value);
}
