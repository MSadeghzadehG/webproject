

package com.badlogic.gdx.backends.lwjgl3;

import java.awt.Toolkit;

import org.lwjgl.glfw.GLFW;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.utils.Clipboard;


public class Lwjgl3Clipboard implements Clipboard {
	@Override
	public String getContents () {		
		return GLFW.glfwGetClipboardString(((Lwjgl3Graphics)Gdx.graphics).getWindow().getWindowHandle());
	}

	@Override
	public void setContents (String content) {
		GLFW.glfwSetClipboardString(((Lwjgl3Graphics)Gdx.graphics).getWindow().getWindowHandle(), content);
	}
}
