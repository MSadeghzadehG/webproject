

package io.reactivex.internal.util;

import java.lang.annotation.*;


@Retention(RetentionPolicy.CLASS)
@Documented
@Target({ElementType.METHOD, ElementType.CONSTRUCTOR, ElementType.TYPE})
public @interface SuppressAnimalSniffer {

}
