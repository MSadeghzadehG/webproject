

package com.badlogic.gdx;

import java.util.Map;


public interface Preferences {
	public Preferences putBoolean (String key, boolean val);

	public Preferences putInteger (String key, int val);

	public Preferences putLong (String key, long val);

	public Preferences putFloat (String key, float val);

	public Preferences putString (String key, String val);

	public Preferences put (Map<String, ?> vals);

	public boolean getBoolean (String key);

	public int getInteger (String key);

	public long getLong (String key);

	public float getFloat (String key);

	public String getString (String key);

	public boolean getBoolean (String key, boolean defValue);

	public int getInteger (String key, int defValue);

	public long getLong (String key, long defValue);

	public float getFloat (String key, float defValue);

	public String getString (String key, String defValue);

	
	public Map<String, ?> get ();

	public boolean contains (String key);

	public void clear ();

	public void remove (String key);

	
	public void flush ();
}
