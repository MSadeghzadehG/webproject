

package org.springframework.boot.configurationprocessor.fieldvalues.javac;

import java.util.Collections;
import java.util.Set;

import javax.lang.model.element.Modifier;


class VariableTree extends ReflectionWrapper {

	VariableTree(Object instance) {
		super("com.sun.source.tree.VariableTree", instance);
	}

	public String getName() throws Exception {
		return findMethod("getName").invoke(getInstance()).toString();
	}

	public String getType() throws Exception {
		return findMethod("getType").invoke(getInstance()).toString();
	}

	public ExpressionTree getInitializer() throws Exception {
		Object instance = findMethod("getInitializer").invoke(getInstance());
		return (instance == null ? null : new ExpressionTree(instance));
	}

	@SuppressWarnings("unchecked")
	public Set<Modifier> getModifierFlags() throws Exception {
		Object modifiers = findMethod("getModifiers").invoke(getInstance());
		if (modifiers == null) {
			return Collections.emptySet();
		}
		return (Set<Modifier>) findMethod(findClass("com.sun.source.tree.ModifiersTree"),
				"getFlags").invoke(modifiers);
	}

}
