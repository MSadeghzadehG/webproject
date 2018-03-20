

package com.badlogic.gdx.jnigen;

import java.util.ArrayList;

import com.badlogic.gdx.jnigen.BuildTarget.TargetOs;
import com.badlogic.gdx.jnigen.FileDescriptor.FileType;

public class AndroidNdkScriptGenerator {
	public void generate (BuildConfig config, BuildTarget target) {
		if (target.os != TargetOs.Android) throw new IllegalArgumentException("target os must be Android");

				if (!config.libsDir.exists()) {
			if (!config.libsDir.mkdirs())
				throw new RuntimeException("Couldn't create directory for shared library files in '" + config.libsDir + "'");
		}
		if (!config.jniDir.exists()) {
			if (!config.jniDir.mkdirs())
				throw new RuntimeException("Couldn't create native code directory '" + config.jniDir + "'");
		}

		ArrayList<FileDescriptor> files = new ArrayList<FileDescriptor>();

		int idx = 0;
		String[] includes = new String[target.cIncludes.length + target.cppIncludes.length];
		for (String include : target.cIncludes)
			includes[idx++] = config.jniDir + "/" + include;
		for (String include : target.cppIncludes)
			includes[idx++] = config.jniDir + "/" + include;

		idx = 0;
		String[] excludes = new String[target.cExcludes.length + target.cppExcludes.length + 1];
		for (String exclude : target.cExcludes)
			excludes[idx++] = config.jniDir + "/" + exclude;
		for (String exclude : target.cppExcludes)
			excludes[idx++] = config.jniDir + "/" + exclude;
		excludes[idx] = "**/target