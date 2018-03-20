

package org.springframework.boot.groovy;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import org.springframework.boot.cli.compiler.autoconfigure.GroovyTemplatesCompilerAutoConfiguration;


@Target(ElementType.TYPE)
@Documented
@Retention(RetentionPolicy.RUNTIME)
public @interface EnableGroovyTemplates {

}
