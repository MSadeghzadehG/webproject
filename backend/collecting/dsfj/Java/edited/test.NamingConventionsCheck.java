

package org.elasticsearch.test;

import java.io.IOException;
import java.lang.reflect.Modifier;
import java.nio.file.FileVisitResult;
import java.nio.file.FileVisitor;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.HashSet;
import java.util.Objects;
import java.util.Set;


public class NamingConventionsCheck {
    public static void main(String[] args) throws IOException {
        Class<?> testClass = null;
        Class<?> integTestClass = null;
        Path rootPath = null;
        boolean skipIntegTestsInDisguise = false;
        boolean selfTest = false;
        boolean checkMainClasses = false;
        for (int i = 0; i < args.length; i++) {
            String arg = args[i];
            switch (arg) {
                case "--test-class":
                    testClass = loadClassWithoutInitializing(args[++i]);
                    break;
                case "--integ-test-class":
                    integTestClass = loadClassWithoutInitializing(args[++i]);
                    break;
                case "--skip-integ-tests-in-disguise":
                    skipIntegTestsInDisguise = true;
                    break;
                case "--self-test":
                    selfTest = true;
                    break;
                case "--main":
                    checkMainClasses = true;
                    break;
                case "--":
                    rootPath = Paths.get(args[++i]);
                    break;
                default:
                    fail("unsupported argument '" + arg + "'");
            }
        }

        NamingConventionsCheck check = new NamingConventionsCheck(testClass, integTestClass);
        if (checkMainClasses) {
            check.checkMain(rootPath);
        } else {
            check.checkTests(rootPath, skipIntegTestsInDisguise);
        }

        if (selfTest) {
            if (checkMainClasses) {
                assertViolation(NamingConventionsCheckInMainTests.class.getName(), check.testsInMain);
                assertViolation(NamingConventionsCheckInMainIT.class.getName(), check.testsInMain);
            } else {
                assertViolation("WrongName", check.missingSuffix);
                assertViolation("WrongNameTheSecond", check.missingSuffix);
                assertViolation("DummyAbstractTests", check.notRunnable);
                assertViolation("DummyInterfaceTests", check.notRunnable);
                assertViolation("InnerTests", check.innerClasses);
                assertViolation("NotImplementingTests", check.notImplementing);
                assertViolation("PlainUnit", check.pureUnitTest);
            }
        }

                assertNoViolations(
                "Not all subclasses of " + check.testClass.getSimpleName()
                    + " match the naming convention. Concrete classes must end with [Tests]",
                check.missingSuffix);
        assertNoViolations("Classes ending with [Tests] are abstract or interfaces", check.notRunnable);
        assertNoViolations("Found inner classes that are tests, which are excluded from the test runner", check.innerClasses);
        assertNoViolations("Pure Unit-Test found must subclass [" + check.testClass.getSimpleName() + "]", check.pureUnitTest);
        assertNoViolations("Classes ending with [Tests] must subclass [" + check.testClass.getSimpleName() + "]", check.notImplementing);
        assertNoViolations(
                "Classes ending with [Tests] or [IT] or extending [" + check.testClass.getSimpleName() + "] must be in src/test/java",
                check.testsInMain);
        if (skipIntegTestsInDisguise == false) {
            assertNoViolations(
                    "Subclasses of " + check.integTestClass.getSimpleName() + " should end with IT as they are integration tests",
                    check.integTestsInDisguise);
        }
    }

    private final Set<Class<?>> notImplementing = new HashSet<>();
    private final Set<Class<?>> pureUnitTest = new HashSet<>();
    private final Set<Class<?>> missingSuffix = new HashSet<>();
    private final Set<Class<?>> integTestsInDisguise = new HashSet<>();
    private final Set<Class<?>> notRunnable = new HashSet<>();
    private final Set<Class<?>> innerClasses = new HashSet<>();
    private final Set<Class<?>> testsInMain = new HashSet<>();

    private final Class<?> testClass;
    private final Class<?> integTestClass;

    public NamingConventionsCheck(Class<?> testClass, Class<?> integTestClass) {
        this.testClass = Objects.requireNonNull(testClass, "--test-class is required");
        this.integTestClass = integTestClass;
    }

    public void checkTests(Path rootPath, boolean skipTestsInDisguised) throws IOException {
        Files.walkFileTree(rootPath, new TestClassVisitor() {
            @Override
            protected void visitTestClass(Class<?> clazz) {
                if (skipTestsInDisguised == false && integTestClass.isAssignableFrom(clazz)) {
                    integTestsInDisguise.add(clazz);
                }
                if (Modifier.isAbstract(clazz.getModifiers()) || Modifier.isInterface(clazz.getModifiers())) {
                    notRunnable.add(clazz);
                } else if (isTestCase(clazz) == false) {
                    notImplementing.add(clazz);
                } else if (Modifier.isStatic(clazz.getModifiers())) {
                    innerClasses.add(clazz);
                }
            }

            @Override
            protected void visitIntegrationTestClass(Class<?> clazz) {
                if (isTestCase(clazz) == false) {
                    notImplementing.add(clazz);
                }
            }

            @Override
            protected void visitOtherClass(Class<?> clazz) {
                if (Modifier.isAbstract(clazz.getModifiers()) || Modifier.isInterface(clazz.getModifiers())) {
                    return;
                }
                if (isTestCase(clazz)) {
                    missingSuffix.add(clazz);
                } else if (junit.framework.Test.class.isAssignableFrom(clazz)) {
                    pureUnitTest.add(clazz);
                }
            }
        });
    }

    public void checkMain(Path rootPath) throws IOException {
        Files.walkFileTree(rootPath, new TestClassVisitor() {
            @Override
            protected void visitTestClass(Class<?> clazz) {
                testsInMain.add(clazz);
            }

            @Override
            protected void visitIntegrationTestClass(Class<?> clazz) {
                testsInMain.add(clazz);
            }

            @Override
            protected void visitOtherClass(Class<?> clazz) {
                if (Modifier.isAbstract(clazz.getModifiers()) || Modifier.isInterface(clazz.getModifiers())) {
                    return;
                }
                if (isTestCase(clazz)) {
                    testsInMain.add(clazz);
                }
            }
        });

    }

    
    private static void assertNoViolations(String message, Set<Class<?>> set) {
        if (false == set.isEmpty()) {
            System.err.println(message + ":");
            for (Class<?> bad : set) {
                System.err.println(" * " + bad.getName());
            }
            System.exit(1);
        }
    }

    
    private static void assertViolation(String className, Set<Class<?>> set) {
        className = className.startsWith("org") ? className : "org.elasticsearch.test.NamingConventionsCheckBadClasses$" + className;
        if (false == set.remove(loadClassWithoutInitializing(className))) {
            System.err.println("Error in NamingConventionsCheck! Expected [" + className + "] to be a violation but wasn't.");
            System.exit(1);
        }
    }

    
    private static void fail(String reason) {
        System.err.println(reason);
        System.exit(1);
    }

    static Class<?> loadClassWithoutInitializing(String name) {
        try {
            return Class.forName(name,
                                        false,
                                        NamingConventionsCheck.class.getClassLoader());
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        }
    }

    abstract class TestClassVisitor implements FileVisitor<Path> {
        
        private String packageName;

        
        protected abstract void visitTestClass(Class<?> clazz);
        
        protected abstract void visitIntegrationTestClass(Class<?> clazz);
        
        protected abstract void visitOtherClass(Class<?> clazz);

        @Override
        public final FileVisitResult preVisitDirectory(Path dir, BasicFileAttributes attrs) throws IOException {
                        if (packageName == null) {
                                packageName = "";
            } else {
                packageName += dir.getFileName() + ".";
            }
            return FileVisitResult.CONTINUE;
        }

        @Override
        public final FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
                        packageName = packageName.substring(0, 1 + packageName.lastIndexOf('.', packageName.length() - 2));
            return FileVisitResult.CONTINUE;
        }

        @Override
        public final FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
            String filename = file.getFileName().toString();
            if (filename.endsWith(".class")) {
                String className = filename.substring(0, filename.length() - ".class".length());
                Class<?> clazz = loadClassWithoutInitializing(packageName + className);
                if (clazz.getName().endsWith("Tests")) {
                    visitTestClass(clazz);
                } else if (clazz.getName().endsWith("IT")) {
                    visitIntegrationTestClass(clazz);
                } else {
                    visitOtherClass(clazz);
                }
            }
            return FileVisitResult.CONTINUE;
        }

        
        protected boolean isTestCase(Class<?> clazz) {
            return testClass.isAssignableFrom(clazz);
        }

        @Override
        public final FileVisitResult visitFileFailed(Path file, IOException exc) throws IOException {
            throw exc;
        }
    }
}
