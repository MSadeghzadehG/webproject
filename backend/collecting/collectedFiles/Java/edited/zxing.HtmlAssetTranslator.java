

package com.google.zxing;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.bootstrap.DOMImplementationRegistry;
import org.w3c.dom.ls.DOMImplementationLS;
import org.w3c.dom.ls.LSSerializer;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.regex.Pattern;


public final class HtmlAssetTranslator {

  private static final Pattern COMMA = Pattern.compile(",");

  private HtmlAssetTranslator() {}

  public static void main(String[] args) throws IOException {
    if (args.length < 3) {
      System.err.println("Usage: HtmlAssetTranslator android/assets/ " +
                         "(all|lang1[,lang2 ...]) (all|file1.html[ file2.html ...])");
      return;
    }
    Path assetsDir = Paths.get(args[0]);
    Collection<String> languagesToTranslate = parseLanguagesToTranslate(assetsDir, args[1]);
    List<String> restOfArgs = Arrays.asList(args).subList(2, args.length);
    Collection<String> fileNamesToTranslate = parseFileNamesToTranslate(assetsDir, restOfArgs);
    for (String language : languagesToTranslate) {
      translateOneLanguage(assetsDir, language, fileNamesToTranslate);
    }
  }

  private static Collection<String> parseLanguagesToTranslate(Path assetsDir,
                                                              String languageArg) throws IOException {
    if ("all".equals(languageArg)) {
      Collection<String> languages = new ArrayList<>();
      DirectoryStream.Filter<Path> fileFilter = new DirectoryStream.Filter<Path>() {
        @Override
        public boolean accept(Path entry) {
          String fileName = entry.getFileName().toString();
          return Files.isDirectory(entry) && !Files.isSymbolicLink(entry) &&
              fileName.startsWith("html-") && !"html-en".equals(fileName);
        }
      };
      try (DirectoryStream<Path> dirs = Files.newDirectoryStream(assetsDir, fileFilter)) {
        for (Path languageDir : dirs) {
          languages.add(languageDir.getFileName().toString().substring(5));
        }
      }
      return languages;
    } else {
      return Arrays.asList(COMMA.split(languageArg));
    }
  }

  private static Collection<String> parseFileNamesToTranslate(Path assetsDir,
                                                              List<String> restOfArgs) throws IOException {
    if ("all".equals(restOfArgs.get(0))) {
      Collection<String> fileNamesToTranslate = new ArrayList<>();
      Path htmlEnAssetDir = assetsDir.resolve("html-en");
      try (DirectoryStream<Path> files = Files.newDirectoryStream(htmlEnAssetDir, "*.html")) {
        for (Path file : files) {
          fileNamesToTranslate.add(file.getFileName().toString());
        }
      }
      return fileNamesToTranslate;
    } else {
      return restOfArgs;
    }
  }

  private static void translateOneLanguage(Path assetsDir,
                                           String language,
                                           final Collection<String> filesToTranslate) throws IOException {
    Path targetHtmlDir = assetsDir.resolve("html-" + language);
    Files.createDirectories(targetHtmlDir);
    Path englishHtmlDir = assetsDir.resolve("html-en");

    String translationTextTranslated =
        StringsResourceTranslator.translateString("Translated by Google Translate.", language);

    DirectoryStream.Filter<Path> filter = new DirectoryStream.Filter<Path>() {
      @Override
      public boolean accept(Path entry) {
        String name = entry.getFileName().toString();
        return name.endsWith(".html") && (filesToTranslate.isEmpty() || filesToTranslate.contains(name));
      }
    };
    try (DirectoryStream<Path> files = Files.newDirectoryStream(englishHtmlDir, filter)) {
      for (Path sourceFile : files) {
        translateOneFile(language, targetHtmlDir, sourceFile, translationTextTranslated);
      }
    }
  }

  private static void translateOneFile(String language,
                                       Path targetHtmlDir,
                                       Path sourceFile,
                                       String translationTextTranslated) throws IOException {

    Path destFile = targetHtmlDir.resolve(sourceFile.getFileName());

    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    Document document;
    try {
      DocumentBuilder builder = factory.newDocumentBuilder();
      document = builder.parse(sourceFile.toFile());
    } catch (ParserConfigurationException pce) {
      throw new IllegalStateException(pce);
    } catch (SAXException sae) {
      throw new IOException(sae);
    }

    Element rootElement = document.getDocumentElement();
    rootElement.normalize();

    Queue<Node> nodes = new LinkedList<>();
    nodes.add(rootElement);

    while (!nodes.isEmpty()) {
      Node node = nodes.poll();
      if (shouldTranslate(node)) {
        NodeList children = node.getChildNodes();
        for (int i = 0; i < children.getLength(); i++) {
          nodes.add(children.item(i));
        }
      }
      if (node.getNodeType() == Node.TEXT_NODE) {
        String text = node.getTextContent();
        if (!text.trim().isEmpty()) {
          text = StringsResourceTranslator.translateString(text, language);
          node.setTextContent(' ' + text + ' ');
        }
      }
    }

    Node translateText = document.createTextNode(translationTextTranslated);
    Node paragraph = document.createElement("p");
    paragraph.appendChild(translateText);
    Node body = rootElement.getElementsByTagName("body").item(0);
    body.appendChild(paragraph);

    DOMImplementationRegistry registry;
    try {
      registry = DOMImplementationRegistry.newInstance();
    } catch (ClassNotFoundException | InstantiationException | IllegalAccessException e) {
      throw new IllegalStateException(e);
    }

    DOMImplementationLS impl = (DOMImplementationLS) registry.getDOMImplementation("LS");
    LSSerializer writer = impl.createLSSerializer();
    String fileAsString = writer.writeToString(document);
        fileAsString = fileAsString.replaceAll("<\\?xml[^>]+>", "<!DOCTYPE HTML>");
    Files.write(destFile, Collections.singleton(fileAsString), StandardCharsets.UTF_8);
  }

  private static boolean shouldTranslate(Node node) {
        NamedNodeMap attributes = node.getAttributes();
    if (attributes != null) {
      Node classAttribute = attributes.getNamedItem("class");
      if (classAttribute != null) {
        String textContent = classAttribute.getTextContent();
        if (textContent != null && textContent.contains("notranslate")) {
          return false;
        }
      }
    }
    String nodeName = node.getNodeName();
    if ("script".equalsIgnoreCase(nodeName)) {
      return false;
    }
        String textContent = node.getTextContent();
    if (textContent != null) {
      for (int i = 0; i < textContent.length(); i++) {
        if (Character.isLetter(textContent.charAt(i))) {
          return true;
        }
      }
    }
    return false;
  }

}
