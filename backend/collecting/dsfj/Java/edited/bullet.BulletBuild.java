

package com.badlogic.gdx.physics.bullet;

import java.io.File;

import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.jnigen.AntScriptGenerator;
import com.badlogic.gdx.jnigen.BuildConfig;
import com.badlogic.gdx.jnigen.BuildExecutor;
import com.badlogic.gdx.jnigen.BuildTarget;
import com.badlogic.gdx.jnigen.BuildTarget.TargetOs;
import com.badlogic.gdx.jnigen.NativeCodeGenerator;

public class BulletBuild {
	public static void main (String[] args) throws Exception {
				new NativeCodeGenerator().generate("src", "bin", "jni");

				String cppFlags = "";

				cppFlags += " -fno-strict-aliasing";
												cppFlags += " -fno-rtti";
						cppFlags += " -DBT_NO_PROFILE";
				cppFlags += " -DBT_USE_INVERSE_DYNAMICS_WITH_BULLET2";

				String[] excludes = {"src/bullet/BulletMultiThreaded/GpuSoftBodySolvers