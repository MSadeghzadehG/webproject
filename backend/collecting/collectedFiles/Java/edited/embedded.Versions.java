

package org.springframework.boot.context.embedded;

import java.io.File;
import java.io.FileReader;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathFactory;

import org.xml.sax.InputSource;

import org.springframework.util.StringUtils;


final class Versions {

	private static final String PROPERTIES = "