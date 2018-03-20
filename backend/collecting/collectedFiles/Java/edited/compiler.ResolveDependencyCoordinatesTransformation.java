

package org.springframework.boot.cli.compiler;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import groovy.lang.Grab;
import org.codehaus.groovy.ast.AnnotationNode;
import org.codehaus.groovy.ast.expr.ConstantExpression;
import org.codehaus.groovy.ast.expr.Expression;
import org.codehaus.groovy.transform.ASTTransformation;

import org.springframework.boot.cli.compiler.grape.DependencyResolutionContext;
import org.springframework.core.annotation.Order;


@Order(ResolveDependencyCoordinatesTransformation.ORDER)
public class ResolveDependencyCoordinatesTransformation
		extends AnnotatedNodeASTTransformation {

	
	public static final int ORDER = DependencyManagementBomTransformation.ORDER + 300;

	private static final Set<String> GRAB_ANNOTATION_NAMES = Collections
			.unmodifiableSet(new HashSet<>(
					Arrays.asList(Grab.class.getName(), Grab.class.getSimpleName())));

	private final DependencyResolutionContext resolutionContext;

	public ResolveDependencyCoordinatesTransformation(
			DependencyResolutionContext resolutionContext) {
		super(GRAB_ANNOTATION_NAMES, false);
		this.resolutionContext = resolutionContext;
	}

	@Override
	protected void processAnnotationNodes(List<AnnotationNode> annotationNodes) {
		for (AnnotationNode annotationNode : annotationNodes) {
			transformGrabAnnotation(annotationNode);
		}
	}

	private void transformGrabAnnotation(AnnotationNode grabAnnotation) {
		grabAnnotation.setMember("initClass", new ConstantExpression(false));
		String value = getValue(grabAnnotation);
		if (value != null && !isConvenienceForm(value)) {
			applyGroupAndVersion(grabAnnotation, value);
		}
	}

	private String getValue(AnnotationNode annotation) {
		Expression expression = annotation.getMember("value");
		if (expression instanceof ConstantExpression) {
			Object value = ((ConstantExpression) expression).getValue();
			return (value instanceof String ? (String) value : null);
		}
		return null;
	}

	private boolean isConvenienceForm(String value) {
		return value.contains(":") || value.contains("#");
	}

	private void applyGroupAndVersion(AnnotationNode annotation, String module) {
		if (module != null) {
			setMember(annotation, "module", module);
		}
		else {
			Expression expression = annotation.getMembers().get("module");
			module = (String) ((ConstantExpression) expression).getValue();
		}
		if (annotation.getMember("group") == null) {
			setMember(annotation, "group", this.resolutionContext
					.getArtifactCoordinatesResolver().getGroupId(module));
		}
		if (annotation.getMember("version") == null) {
			setMember(annotation, "version", this.resolutionContext
					.getArtifactCoordinatesResolver().getVersion(module));
		}
	}

	private void setMember(AnnotationNode annotation, String name, String value) {
		ConstantExpression expression = new ConstantExpression(value);
		annotation.setMember(name, expression);
	}

}
