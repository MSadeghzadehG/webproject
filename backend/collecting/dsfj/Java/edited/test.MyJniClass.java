

package com.badlogic.gdx.jnigen.test;

import java.nio.Buffer;
import java.nio.ByteBuffer;

import com.badlogic.gdx.jnigen.AntScriptGenerator;
import com.badlogic.gdx.jnigen.BuildConfig;
import com.badlogic.gdx.jnigen.BuildExecutor;
import com.badlogic.gdx.jnigen.BuildTarget;
import com.badlogic.gdx.jnigen.BuildTarget.TargetOs;
import com.badlogic.gdx.jnigen.JniGenSharedLibraryLoader;
import com.badlogic.gdx.jnigen.NativeCodeGenerator;

public class MyJniClass {
	public static native void test (boolean boolArg, byte byteArg, char charArg, short shortArg, int intArg, long longArg,
		float floatArg, double doubleArg, Buffer byteBuffer, boolean[] boolArray, char[] charArray, short[] shortArray,
		int[] intArray, long[] longArray, float[] floatArray, double[] doubleArray, double[][] multidim, 
		String string, Class classy, Throwable thr, Object obj ); 
	
		
	
	
	public static void main(String[] args) throws Exception {
				new NativeCodeGenerator().generate("src", "bin", "jni", new String[] { "**/MyJniClass.java" }, null);
		
				BuildConfig buildConfig = new BuildConfig("test");
		BuildTarget win32 = BuildTarget.newDefaultTarget(TargetOs.Windows, false);
		win32.compilerPrefix = "";
		win32.cppFlags += " -g";
		BuildTarget lin64 = BuildTarget.newDefaultTarget(TargetOs.Linux, true);
		new AntScriptGenerator().generate(buildConfig, win32, lin64);
		
				BuildExecutor.executeAnt("jni/build-linux64.xml", "-v -Dhas-compiler=true clean postcompile");
		BuildExecutor.executeAnt("jni/build.xml", "-v pack-natives");
		
				new JniGenSharedLibraryLoader("libs/test-natives.jar").load("test");
		ByteBuffer buffer = ByteBuffer.allocateDirect(1);
		buffer.put(0, (byte)8);
		MyJniClass.test(
			true, (byte)1, (char)2, (short)3, 4, 5, 6, 7, 
			buffer, new boolean[] { false }, new char[] { 9 },
			new short[] { 10 }, new int[] { 11 }, new long[] { 12 },
			new float[] { 13 }, new double[] { 14 }, 
			null, "Hurray", MyJniClass.class, new RuntimeException(), new MyJniClass());
	}
}