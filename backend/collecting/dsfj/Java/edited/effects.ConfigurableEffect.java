

package com.badlogic.gdx.tools.hiero.unicodefont.effects;

import java.util.List;


public interface ConfigurableEffect extends Effect {
	
	public List getValues ();

	
	public void setValues (List values);

	
	static public interface Value {
		
		public String getName ();

		
		public void setString (String value);

		
		public String getString ();

		
		public Object getObject ();

		
		public void showDialog ();
	}
}
