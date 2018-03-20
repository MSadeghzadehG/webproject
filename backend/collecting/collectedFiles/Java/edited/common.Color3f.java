


package org.jbox2d.common;


public class Color3f {
	
	public static final Color3f WHITE = new Color3f(1, 1, 1);
	public static final Color3f BLACK = new Color3f(0, 0, 0);
	public static final Color3f BLUE = new Color3f(0, 0, 1);
	public static final Color3f GREEN = new Color3f(0, 1, 0);
	public static final Color3f RED = new Color3f(1, 0, 0);
	
	public float x;
	public float y;
	public float z;

	
	public Color3f(){
		x = y = z = 0;
	}
	public Color3f(float r, float g, float b) {
		x = r;
		y = g;
		z = b;
	}
	
	public void set(float r, float g, float b){
		x = r;
		y = g;
		z = b;
	}
	
	public void set(Color3f argColor){
		x = argColor.x;
		y = argColor.y;
		z = argColor.z;
	}
}
