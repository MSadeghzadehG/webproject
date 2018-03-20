

package org.springframework.boot.cli.compiler;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.codehaus.groovy.ast.AnnotatedNode;
import org.codehaus.groovy.ast.AnnotationNode;
import org.codehaus.groovy.ast.ClassNode;
import org.codehaus.groovy.ast.FieldNode;
import org.codehaus.groovy.ast.MethodNode;
import org.codehaus.groovy.ast.expr.ArgumentListExpression;
import org.codehaus.groovy.ast.expr.ClosureExpression;
import org.codehaus.groovy.ast.expr.ConstantExpression;
import org.codehaus.groovy.ast.expr.Expression;
import org.codehaus.groovy.ast.expr.MethodCallExpression;
import org.codehaus.groovy.ast.stmt.BlockStatement;
import org.codehaus.groovy.ast.stmt.ExpressionStatement;
import org.codehaus.groovy.ast.stmt.Statement;

import org.springframework.util.PatternMatchUtils;


public abstract class AstUtils {

	
	public static boolean hasAtLeastOneAnnotation(ClassNode node, String... annotations) {
		if (hasAtLeastOneAnnotation((AnnotatedNode) node, annotations)) {
			return true;
		}
		for (MethodNode method : node.getMethods()) {
			if (hasAtLeastOneAnnotation(method, annotations)) {
				return true;
			}
		}
		return false;
	}

	
	public static boolean hasAtLeastOneAnnotation(AnnotatedNode node,
			String... annotations) {
		for (AnnotationNode annotationNode : node.getAnnotations()) {
			for (String annotation : annotations) {
				if (PatternMatchUtils.simpleMatch(annotation,
						annotationNode.getClassNode().getName())) {
					return true;
				}
			}
		}
		return false;
	}

	
	public static boolean hasAtLeastOneFieldOrMethod(ClassNode node, String... types) {
		Set<String> typesSet = new HashSet<>(Arrays.asList(types));
		for (FieldNode field : node.getFields()) {
			if (typesSet.contains(field.getType().getName())) {
				return true;
			}
		}
		for (MethodNode method : node.getMethods()) {
			if (typesSet.contains(method.getReturnType().getName())) {
				return true;
			}
		}
		return false;
	}

	
	public static boolean subclasses(ClassNode node, String... types) {
		for (String type : types) {
			if (node.getSuperClass().getName().equals(type)) {
				return true;
			}
		}
		return false;
	}

	public static boolean hasAtLeastOneInterface(ClassNode classNode, String... types) {
		Set<String> typesSet = new HashSet<>(Arrays.asList(types));
		for (ClassNode inter : classNode.getInterfaces()) {
			if (typesSet.contains(inter.getName())) {
				return true;
			}
		}
		return false;
	}

	
	public static ClosureExpression getClosure(BlockStatement block, String name,
			boolean remove) {
		for (ExpressionStatement statement : getExpressionStatements(block)) {
			Expression expression = statement.getExpression();
			if (expression instanceof MethodCallExpression) {
				ClosureExpression closure = getClosure(name,
						(MethodCallExpression) expression);
				if (closure != null) {
					if (remove) {
						block.getStatements().remove(statement);
					}
					return closure;
				}
			}
		}
		return null;
	}

	private static List<ExpressionStatement> getExpressionStatements(
			BlockStatement block) {
		List<ExpressionStatement> statements = new ArrayList<>();
		for (Statement statement : block.getStatements()) {
			if (statement instanceof ExpressionStatement) {
				statements.add((ExpressionStatement) statement);
			}
		}
		return statements;
	}

	private static ClosureExpression getClosure(String name,
			MethodCallExpression expression) {
		Expression method = expression.getMethod();
		if (method instanceof ConstantExpression
				&& name.equals(((ConstantExpression) method).getValue())) {
			return (ClosureExpression) ((ArgumentListExpression) expression
					.getArguments()).getExpression(0);
		}
		return null;
	}

	public static ClosureExpression getClosure(BlockStatement block, String name) {
		return getClosure(block, name, false);
	}

}
