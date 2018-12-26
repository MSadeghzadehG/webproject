

package com.badlogic.gdx.jnigen;

import java.util.ArrayList;

import com.badlogic.gdx.jnigen.BuildTarget.TargetOs;
import com.badlogic.gdx.jnigen.FileDescriptor.FileType;


public class AntScriptGenerator {
	
	public void generate (BuildConfig config, BuildTarget... targets) {
				if (!config.libsDir.exists()) {
			if (!config.libsDir.mkdirs())
				throw new RuntimeException("Couldn't create directory for shared library files in '" + config.libsDir + "'");
		}
		if (!config.jniDir.exists()) {
			if (!config.jniDir.mkdirs())
				throw new RuntimeException("Couldn't create native code directory '" + config.jniDir + "'");
		}

				copyJniHeaders(config.jniDir.path());

				if (config.jniDir.child("memcpy_wrap.c").exists() == false) {
			new FileDescriptor("com/badlogic/gdx/jnigen/resources/scripts/memcpy_wrap.c", FileType.Classpath).copyTo(config.jniDir
				.child("memcpy_wrap.c"));
		}

		ArrayList<String> buildFiles = new ArrayList<String>();
		ArrayList<String> libsDirs = new ArrayList<String>();
		ArrayList<String> sharedLibFiles = new ArrayList<String>();

				for (BuildTarget target : targets) {
			String buildFile = generateBuildTargetTemplate(config, target);
			FileDescriptor libsDir = new FileDescriptor(getLibsDirectory(config, target));

			if (!libsDir.exists()) {
				if (!libsDir.mkdirs()) throw new RuntimeException("Couldn't create libs directory '" + libsDir + "'");
			}

			String buildFileName = "build-" + target.os.toString().toLowerCase() + (target.is64Bit ? "64" : "32") + ".xml";
			if (target.buildFileName != null) buildFileName = target.buildFileName;
			config.jniDir.child(buildFileName).writeString(buildFile, false);
			System.out.println("Wrote target '" + target.os + (target.is64Bit ? "64" : "") + "' build script '"
				+ config.jniDir.child(buildFileName) + "'");

			if (!target.excludeFromMasterBuildFile) {
				if (target.os != TargetOs.MacOsX && target.os != TargetOs.IOS) {
					buildFiles.add(buildFileName);
				}

				String sharedLibFilename = target.libName;
				if (sharedLibFilename == null)
					sharedLibFilename = getSharedLibFilename(target.os, target.is64Bit, config.sharedLibName);
				
				sharedLibFiles.add(sharedLibFilename);
				if (target.os != TargetOs.Android && target.os != TargetOs.IOS) {
					libsDirs.add("../" + libsDir.path().replace('\\', '/'));
				}
			}
		}

				String template = new FileDescriptor("com/badlogic/gdx/jnigen/resources/scripts/build.xml.template", FileType.Classpath)
			.readString();
		StringBuffer clean = new StringBuffer();
		StringBuffer compile = new StringBuffer();
		StringBuffer pack = new StringBuffer();

		for (int i = 0; i < buildFiles.size(); i++) {
			clean.append("\t\t<ant antfile=\"" + buildFiles.get(i) + "\" target=\"clean\"/>\n");
			compile.append("\t\t<ant antfile=\"" + buildFiles.get(i) + "\"/>\n");
		}
		for (int i = 0; i < libsDirs.size(); i++) {
			pack.append("\t\t\t<fileset dir=\"" + libsDirs.get(i) + "\" includes=\"" + sharedLibFiles.get(i) + "\"/>\n");
		}

		if (config.sharedLibs != null) {
			for (String sharedLib : config.sharedLibs) {
				pack.append("\t\t\t<fileset dir=\"" + sharedLib + "\"/>\n");
			}
		}

		template = template.replace("%projectName%", config.sharedLibName + "-natives");
		template = template.replace("<clean/>", clean.toString());
		template = template.replace("<compile/>", compile.toString());
		template = template.replace("%packFile%", "../" + config.libsDir.path().replace('\\', '/') + "/" + config.sharedLibName
			+ "-natives.jar");
		template = template.replace("<pack/>", pack);

		config.jniDir.child("build.xml").writeString(template, false);
		System.out.println("Wrote master build script '" + config.jniDir.child("build.xml") + "'");
	}

	private void copyJniHeaders (String jniDir) {
		final String pack = "com/badlogic/gdx/jnigen/resources/headers";
		String files[] = {"classfile_constants.h", "jawt.h", "jdwpTransport.h", "jni.h", "linux/jawt_md.h", "linux/jni_md.h",
			"mac/jni_md.h", "win32/jawt_md.h", "win32/jni_md.h"};

		for (String file : files) {
			new FileDescriptor(pack, FileType.Classpath).child(file).copyTo(
				new FileDescriptor(jniDir).child("jni-headers").child(file));
		}
	}

	private String getSharedLibFilename (TargetOs os, boolean is64Bit, String sharedLibName) {
				String libPrefix = "";
		String libSuffix = "";
		if (os == TargetOs.Windows) {
			libSuffix = (is64Bit ? "64" : "") + ".dll";
		}
		if (os == TargetOs.Linux || os == TargetOs.Android) {
			libPrefix = "lib";
			libSuffix = (is64Bit ? "64" : "") + ".so";
		}
		if (os == TargetOs.MacOsX) {
			libPrefix = "lib";
			libSuffix = (is64Bit ? "64" : "") + ".dylib";
		}
		if (os == TargetOs.IOS) {
			libPrefix = "lib";
			libSuffix = ".a";
		}
		return libPrefix + sharedLibName + libSuffix;
	}

	private String getJniPlatform (TargetOs os) {
		if (os == TargetOs.Windows) return "win32";
		if (os == TargetOs.Linux) return "linux";
		if (os == TargetOs.MacOsX) return "mac";
		return "";
	}

	private String getLibsDirectory (BuildConfig config, BuildTarget target) {
		String targetName = target.osFileName;
		if (targetName == null) targetName = target.os.toString().toLowerCase() + (target.is64Bit ? "64" : "32");
		return config.libsDir.child(targetName).path().replace('\\', '/');
	}

	private String generateBuildTargetTemplate (BuildConfig config, BuildTarget target) {
				if (target.os == TargetOs.Android) {
			new AndroidNdkScriptGenerator().generate(config, target);
			String template = new FileDescriptor("com/badlogic/gdx/jnigen/resources/scripts/build-android.xml.template",
				FileType.Classpath).readString();
			template = template.replace("%precompile%", target.preCompileTask == null ? "" : target.preCompileTask);
			template = template.replace("%postcompile%", target.postCompileTask == null ? "" : target.postCompileTask);
			return template;
		}

				String template = null;
		if (target.os == TargetOs.IOS) {
			template = new FileDescriptor("com/badlogic/gdx/jnigen/resources/scripts/build-ios.xml.template", FileType.Classpath)
				.readString();
		} else {
			template = new FileDescriptor("com/badlogic/gdx/jnigen/resources/scripts/build-target.xml.template", FileType.Classpath)
				.readString();
		}

				String libName = target.libName;
		if (libName == null) libName = getSharedLibFilename(target.os, target.is64Bit, config.sharedLibName);
		String jniPlatform = getJniPlatform(target.os);

						StringBuffer cIncludes = new StringBuffer();
		cIncludes.append("\t\t<include name=\"memcpy_wrap.c\"/>\n");
		for (String cInclude : target.cIncludes) {
			cIncludes.append("\t\t<include name=\"" + cInclude + "\"/>\n");
		}
		StringBuffer cppIncludes = new StringBuffer();
		for (String cppInclude : target.cppIncludes) {
			cppIncludes.append("\t\t<include name=\"" + cppInclude + "\"/>\n");
		}
		StringBuffer cExcludes = new StringBuffer();
		for (String cExclude : target.cExcludes) {
			cExcludes.append("\t\t<exclude name=\"" + cExclude + "\"/>\n");
		}
		StringBuffer cppExcludes = new StringBuffer();
		for (String cppExclude : target.cppExcludes) {
			cppExcludes.append("\t\t<exclude name=\"" + cppExclude + "\"/>\n");
		}

				StringBuffer headerDirs = new StringBuffer();
		for (String headerDir : target.headerDirs) {
			headerDirs.append("\t\t\t<arg value=\"-I" + headerDir + "\"/>\n");
		}

		String targetFolder = target.osFileName;
		if (targetFolder == null) targetFolder = target.os.toString().toLowerCase() + (target.is64Bit ? "64" : "32");

				template = template.replace("%projectName%", config.sharedLibName + "-" + target.os + "-" + (target.is64Bit ? "64" : "32"));
		template = template.replace("%buildDir%", config.buildDir.child(targetFolder).path().replace('\\', '/'));
		template = template.replace("%libsDir%", "../" + getLibsDirectory(config, target));
		template = template.replace("%libName%", libName);
		template = template.replace("%jniPlatform%", jniPlatform);
		template = template.replace("%compilerPrefix%", target.compilerPrefix);
		template = template.replace("%cFlags%", target.cFlags);
		template = template.replace("%cppFlags%", target.cppFlags);
		template = template.replace("%linkerFlags%", target.linkerFlags);
		template = template.replace("%libraries%", target.libraries);
		template = template.replace("%cIncludes%", cIncludes);
		template = template.replace("%cExcludes%", cExcludes);
		template = template.replace("%cppIncludes%", cppIncludes);
		template = template.replace("%cppExcludes%", cppExcludes);
		template = template.replace("%headerDirs%", headerDirs);
		template = template.replace("%precompile%", target.preCompileTask == null ? "" : target.preCompileTask);
		template = template.replace("%postcompile%", target.postCompileTask == null ? "" : target.postCompileTask);

		return template;
	}
}
