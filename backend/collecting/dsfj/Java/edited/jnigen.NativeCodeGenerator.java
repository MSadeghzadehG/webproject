

package com.badlogic.gdx.jnigen;

import java.io.InputStream;
import java.nio.Buffer;
import java.util.ArrayList;

import com.badlogic.gdx.jnigen.parsing.CMethodParser;
import com.badlogic.gdx.jnigen.parsing.CMethodParser.CMethod;
import com.badlogic.gdx.jnigen.parsing.CMethodParser.CMethodParserResult;
import com.badlogic.gdx.jnigen.parsing.JavaMethodParser;
import com.badlogic.gdx.jnigen.parsing.JavaMethodParser.Argument;
import com.badlogic.gdx.jnigen.parsing.JavaMethodParser.JavaMethod;
import com.badlogic.gdx.jnigen.parsing.JavaMethodParser.JavaSegment;
import com.badlogic.gdx.jnigen.parsing.JavaMethodParser.JniSection;
import com.badlogic.gdx.jnigen.parsing.JniHeaderCMethodParser;
import com.badlogic.gdx.jnigen.parsing.RobustJavaMethodParser;


public class NativeCodeGenerator {
	private static final String JNI_METHOD_MARKER = "native";
	private static final String JNI_ARG_PREFIX = "obj_";
	private static final String JNI_RETURN_VALUE = "JNI_returnValue";
	private static final String JNI_WRAPPER_PREFIX = "wrapped_";
	FileDescriptor sourceDir;
	String classpath;
	FileDescriptor jniDir;
	String[] includes;
	String[] excludes;
	AntPathMatcher matcher = new AntPathMatcher();
	JavaMethodParser javaMethodParser = new RobustJavaMethodParser();
	CMethodParser cMethodParser = new JniHeaderCMethodParser();
	CMethodParserResult cResult;

	
	public void generate () throws Exception {
		generate("src", "bin", "jni", null, null);
	}

	
	public void generate (String sourceDir, String classpath, String jniDir) throws Exception {
		generate(sourceDir, classpath, jniDir, null, null);
	}

	
	public void generate (String sourceDir, String classpath, String jniDir, String[] includes, String[] excludes)
		throws Exception {
		this.sourceDir = new FileDescriptor(sourceDir);
		this.jniDir = new FileDescriptor(jniDir);
		this.classpath = classpath;
		this.includes = includes;
		this.excludes = excludes;

				if (!this.sourceDir.exists()) {
			throw new Exception("Java source directory '" + sourceDir + "' does not exist");
		}

				if (!this.jniDir.exists()) {
			if (!this.jniDir.mkdirs()) {
				throw new Exception("Couldn't create JNI directory '" + jniDir + "'");
			}
		}

				processDirectory(this.sourceDir);
	}

	private void processDirectory (FileDescriptor dir) throws Exception {
		FileDescriptor[] files = dir.list();
		for (FileDescriptor file : files) {
			if (file.isDirectory()) {
				if (file.path().contains(".svn")) continue;
				if (excludes != null && matcher.match(file.path(), excludes)) continue;
				processDirectory(file);
			} else {
				if (file.extension().equals("java")) {
					if (file.name().contains("NativeCodeGenerator")) continue;
					if (includes != null && !matcher.match(file.path(), includes)) continue;
					if (excludes != null && matcher.match(file.path(), excludes)) continue;
					String className = getFullyQualifiedClassName(file);
					FileDescriptor hFile = new FileDescriptor(jniDir.path() + "/" + className + ".h");
					FileDescriptor cppFile = new FileDescriptor(jniDir + "/" + className + ".cpp");
					if (file.lastModified() < cppFile.lastModified()) {
						System.out.println("C/C++ for '" + file.path() + "' up to date");
						continue;
					}
					String javaContent = file.readString();
					if (javaContent.contains(JNI_METHOD_MARKER)) {
						ArrayList<JavaSegment> javaSegments = javaMethodParser.parse(javaContent);
						if (javaSegments.size() == 0) {
							System.out.println("Skipping '" + file + "', no JNI code found.");
							continue;
						}
						System.out.print("Generating C/C++ for '" + file + "'...");
						generateHFile(file);
						generateCppFile(javaSegments, hFile, cppFile);
						System.out.println("done");
					}
				}
			}
		}
	}

	private String getFullyQualifiedClassName (FileDescriptor file) {
		String className = file.path().replace(sourceDir.path(), "").replace('\\', '.').replace('/', '.').replace(".java", "");
		if (className.startsWith(".")) className = className.substring(1);
		return className;
	}

	private void generateHFile (FileDescriptor file) throws Exception {
		String className = getFullyQualifiedClassName(file);
		String command = "javah -classpath " + classpath + " -o " + jniDir.path() + "/" + className + ".h " + className;
		Process process = Runtime.getRuntime().exec(command);
		process.waitFor();
		if (process.exitValue() != 0) {
			System.out.println();
			System.out.println("Command: " + command);
			InputStream errorStream = process.getErrorStream();
			int c = 0;
			while ((c = errorStream.read()) != -1) {
				System.out.print((char)c);
			}
		}
	}

	protected void emitHeaderInclude (StringBuffer buffer, String fileName) {
		buffer.append("#include <" + fileName + ">\n");
	}

	private void generateCppFile (ArrayList<JavaSegment> javaSegments, FileDescriptor hFile, FileDescriptor cppFile)
		throws Exception {
		String headerFileContent = hFile.readString();
		ArrayList<CMethod> cMethods = cMethodParser.parse(headerFileContent).getMethods();

		StringBuffer buffer = new StringBuffer();
		emitHeaderInclude(buffer, hFile.name());

		for (JavaSegment segment : javaSegments) {
			if (segment instanceof JniSection) {
				emitJniSection(buffer, (JniSection)segment);
			}

			if (segment instanceof JavaMethod) {
				JavaMethod javaMethod = (JavaMethod)segment;
				if (javaMethod.getNativeCode() == null) {
					throw new RuntimeException("Method '" + javaMethod.getName() + "' has no body");
				}
				CMethod cMethod = findCMethod(javaMethod, cMethods);
				if (cMethod == null)
					throw new RuntimeException("Couldn't find C method for Java method '" + javaMethod.getClassName() + "#"
						+ javaMethod.getName() + "'");
				emitJavaMethod(buffer, javaMethod, cMethod);
			}
		}
		cppFile.writeString(buffer.toString(), false, "UTF-8");
	}

	private CMethod findCMethod (JavaMethod javaMethod, ArrayList<CMethod> cMethods) {
		for (CMethod cMethod : cMethods) {
			String javaMethodName = javaMethod.getName().replace("_", "_1");
			String javaClassName = javaMethod.getClassName().toString().replace("_", "_1");
			if (cMethod.getHead().endsWith(javaClassName + "_" + javaMethodName)
				|| cMethod.getHead().contains(javaClassName + "_" + javaMethodName + "__")) {
												if (cMethod.getArgumentTypes().length - 2 == javaMethod.getArguments().size()) {
					boolean match = true;
					for (int i = 2; i < cMethod.getArgumentTypes().length; i++) {
						String cType = cMethod.getArgumentTypes()[i];
						String javaType = javaMethod.getArguments().get(i - 2).getType().getJniType();
						if (!cType.equals(javaType)) {
							match = false;
							break;
						}
					}

					if (match) {
						return cMethod;
					}
				}
			}
		}
		return null;
	}

	private void emitLineMarker (StringBuffer buffer, int line) {
		buffer.append("\n		buffer.append(line);
		buffer.append("\n");
	}

	private void emitJniSection (StringBuffer buffer, JniSection section) {
		emitLineMarker(buffer, section.getStartIndex());
		buffer.append(section.getNativeCode().replace("\r", ""));
	}

	private void emitJavaMethod (StringBuffer buffer, JavaMethod javaMethod, CMethod cMethod) {
				StringBuffer jniSetupCode = new StringBuffer();
		StringBuffer jniCleanupCode = new StringBuffer();
		StringBuffer additionalArgs = new StringBuffer();
		StringBuffer wrapperArgs = new StringBuffer();
		emitJniSetupCode(jniSetupCode, javaMethod, additionalArgs, wrapperArgs);
		emitJniCleanupCode(jniCleanupCode, javaMethod, cMethod);

				boolean isManual = javaMethod.isManual();

						if (javaMethod.hasDisposableArgument() && javaMethod.getNativeCode().contains("return")) {
									if (isManual) {
				emitMethodSignature(buffer, javaMethod, cMethod, null, false);
				emitMethodBody(buffer, javaMethod);
				buffer.append("}\n\n");
			} else {
												String wrappedMethodName = emitMethodSignature(buffer, javaMethod, cMethod, additionalArgs.toString());
				emitMethodBody(buffer, javaMethod);
				buffer.append("}\n\n");

								emitMethodSignature(buffer, javaMethod, cMethod, null);
				if (!isManual) {
					buffer.append(jniSetupCode);
				}

				if (cMethod.getReturnType().equals("void")) {
					buffer.append("\t" + wrappedMethodName + "(" + wrapperArgs.toString() + ");\n\n");
					if (!isManual) {
						buffer.append(jniCleanupCode);
					}
					buffer.append("\treturn;\n");
				} else {
					buffer.append("\t" + cMethod.getReturnType() + " " + JNI_RETURN_VALUE + " = " + wrappedMethodName + "("
						+ wrapperArgs.toString() + ");\n\n");
					if (!isManual) {
						buffer.append(jniCleanupCode);
					}
					buffer.append("\treturn " + JNI_RETURN_VALUE + ";\n");
				}
				buffer.append("}\n\n");
			}
		} else {
			emitMethodSignature(buffer, javaMethod, cMethod, null);
			if (!isManual) {
				buffer.append(jniSetupCode);
			}
			emitMethodBody(buffer, javaMethod);
			if (!isManual) {
				buffer.append(jniCleanupCode);
			}
			buffer.append("}\n\n");
		}

	}

	protected void emitMethodBody (StringBuffer buffer, JavaMethod javaMethod) {
				emitLineMarker(buffer, javaMethod.getEndIndex());

				buffer.append(javaMethod.getNativeCode());
		buffer.append("\n");
	}

	private String emitMethodSignature (StringBuffer buffer, JavaMethod javaMethod, CMethod cMethod, String additionalArguments) {
		return emitMethodSignature(buffer, javaMethod, cMethod, additionalArguments, true);
	}

	private String emitMethodSignature (StringBuffer buffer, JavaMethod javaMethod, CMethod cMethod, String additionalArguments,
		boolean appendPrefix) {
						String wrappedMethodName = null;
		if (additionalArguments != null) {
			String[] tokens = cMethod.getHead().replace("\r\n", "").replace("\n", "").split(" ");
			wrappedMethodName = JNI_WRAPPER_PREFIX + tokens[3];
			buffer.append("static inline ");
			buffer.append(tokens[1]);
			buffer.append(" ");
			buffer.append(wrappedMethodName);
			buffer.append("\n");
		} else {
			buffer.append(cMethod.getHead());
		}

						if (javaMethod.isStatic()) {
			buffer.append("(JNIEnv* env, jclass clazz");
		} else {
			buffer.append("(JNIEnv* env, jobject object");
		}
		if (javaMethod.getArguments().size() > 0) buffer.append(", ");
		for (int i = 0; i < javaMethod.getArguments().size(); i++) {
						buffer.append(cMethod.getArgumentTypes()[i + 2]);
			buffer.append(" ");
												Argument javaArg = javaMethod.getArguments().get(i);
			if (!javaArg.getType().isPlainOldDataType() && !javaArg.getType().isObject() && appendPrefix) {
				buffer.append(JNI_ARG_PREFIX);
			}
						buffer.append(javaArg.getName());

						if (i < javaMethod.getArguments().size() - 1) buffer.append(", ");
		}

				if (additionalArguments != null) {
			buffer.append(additionalArguments);
		}

				buffer.append(") {\n");

				return wrappedMethodName;
	}

	private void emitJniSetupCode (StringBuffer buffer, JavaMethod javaMethod, StringBuffer additionalArgs,
		StringBuffer wrapperArgs) {
						if (javaMethod.isStatic()) {
			wrapperArgs.append("env, clazz, ");
		} else {
			wrapperArgs.append("env, object, ");
		}

				for (int i = 0; i < javaMethod.getArguments().size(); i++) {
			Argument arg = javaMethod.getArguments().get(i);
			if (!arg.getType().isPlainOldDataType() && !arg.getType().isObject()) {
				wrapperArgs.append(JNI_ARG_PREFIX);
			}
						wrapperArgs.append(arg.getName());
			if (i < javaMethod.getArguments().size() - 1) wrapperArgs.append(", ");
		}

				for (Argument arg : javaMethod.getArguments()) {
			if (arg.getType().isBuffer()) {
				String type = arg.getType().getBufferCType();
				buffer.append("\t" + type + " " + arg.getName() + " = (" + type + ")(" + JNI_ARG_PREFIX + arg.getName()
					+ "?env->GetDirectBufferAddress(" + JNI_ARG_PREFIX + arg.getName() + "):0);\n");
				additionalArgs.append(", ");
				additionalArgs.append(type);
				additionalArgs.append(" ");
				additionalArgs.append(arg.getName());
				wrapperArgs.append(", ");
				wrapperArgs.append(arg.getName());
			}
		}

				for (Argument arg : javaMethod.getArguments()) {
			if (arg.getType().isString()) {
				String type = "char*";
				buffer.append("\t" + type + " " + arg.getName() + " = (" + type + ")env->GetStringUTFChars(" + JNI_ARG_PREFIX
					+ arg.getName() + ", 0);\n");
				additionalArgs.append(", ");
				additionalArgs.append(type);
				additionalArgs.append(" ");
				additionalArgs.append(arg.getName());
				wrapperArgs.append(", ");
				wrapperArgs.append(arg.getName());
			}
		}

						for (Argument arg : javaMethod.getArguments()) {
			if (arg.getType().isPrimitiveArray()) {
				String type = arg.getType().getArrayCType();
				buffer.append("\t" + type + " " + arg.getName() + " = (" + type + ")env->GetPrimitiveArrayCritical(" + JNI_ARG_PREFIX
					+ arg.getName() + ", 0);\n");
				additionalArgs.append(", ");
				additionalArgs.append(type);
				additionalArgs.append(" ");
				additionalArgs.append(arg.getName());
				wrapperArgs.append(", ");
				wrapperArgs.append(arg.getName());
			}
		}

				buffer.append("\n");
	}

	private void emitJniCleanupCode (StringBuffer buffer, JavaMethod javaMethod, CMethod cMethod) {
				for (Argument arg : javaMethod.getArguments()) {
			if (arg.getType().isPrimitiveArray()) {
				buffer.append("\tenv->ReleasePrimitiveArrayCritical(" + JNI_ARG_PREFIX + arg.getName() + ", " + arg.getName()
					+ ", 0);\n");
			}
		}

				for (Argument arg : javaMethod.getArguments()) {
			if (arg.getType().isString()) {
				buffer.append("\tenv->ReleaseStringUTFChars(" + JNI_ARG_PREFIX + arg.getName() + ", " + arg.getName() + ");\n");
			}
		}

				buffer.append("\n");
	}
}
