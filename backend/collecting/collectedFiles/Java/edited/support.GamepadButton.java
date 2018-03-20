

package com.badlogic.gdx.controllers.gwt.support;

import com.google.gwt.core.client.JavaScriptObject;

public final class GamepadButton extends JavaScriptObject {

    protected GamepadButton() {
            }

    public native boolean getPressed() ;

    public native double getTouched() ;

    public native double getValue() ;

}
