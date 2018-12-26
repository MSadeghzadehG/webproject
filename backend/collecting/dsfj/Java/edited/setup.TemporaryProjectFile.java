

package com.badlogic.gdx.setup;

import java.io.File;


public class TemporaryProjectFile extends ProjectFile {

	
	public File file;

	public TemporaryProjectFile(File file, String outputString, boolean isTemplate) {
		super(outputString, isTemplate);
		this.file = file;
	}

}