

package sample.jooq.util;

import org.jooq.util.DefaultGeneratorStrategy;
import org.jooq.util.Definition;


public class TangleFreeGeneratorStrategy extends DefaultGeneratorStrategy {

	@Override
	public String getJavaPackageName(Definition definition, Mode mode) {
		return getTargetPackage();
	}

}
