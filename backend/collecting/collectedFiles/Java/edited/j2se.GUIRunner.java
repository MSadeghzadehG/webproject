

package com.google.zxing.client.j2se;

import com.google.zxing.BinaryBitmap;
import com.google.zxing.LuminanceSource;
import com.google.zxing.MultiFormatReader;
import com.google.zxing.ReaderException;
import com.google.zxing.Result;
import com.google.zxing.common.HybridBinarizer;

import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.net.MalformedURLException;
import java.nio.file.Path;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.WindowConstants;
import javax.swing.text.JTextComponent;


public final class GUIRunner extends JFrame {

  private final JLabel imageLabel;
  private final JTextComponent textArea;

  private GUIRunner() {
    imageLabel = new JLabel();
    textArea = new JTextArea();
    textArea.setEditable(false);
    textArea.setMaximumSize(new Dimension(400, 200));
    Container panel = new JPanel();
    panel.setLayout(new FlowLayout());
    panel.add(imageLabel);
    panel.add(textArea);
    setTitle("ZXing");
    setSize(400, 400);
    setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
    setContentPane(panel);
    setLocationRelativeTo(null);
  }

  public static void main(String[] args) throws MalformedURLException {
    GUIRunner runner = new GUIRunner();
    runner.setVisible(true);
    runner.chooseImage();
  }

  private void chooseImage() throws MalformedURLException {
    JFileChooser fileChooser = new JFileChooser();
    fileChooser.showOpenDialog(this);
    Path file = fileChooser.getSelectedFile().toPath();
    Icon imageIcon = new ImageIcon(file.toUri().toURL());
    setSize(imageIcon.getIconWidth(), imageIcon.getIconHeight() + 100);
    imageLabel.setIcon(imageIcon);
    String decodeText = getDecodeText(file);
    textArea.setText(decodeText);
  }

  private static String getDecodeText(Path file) {
    BufferedImage image;
    try {
      image = ImageReader.readImage(file.toUri());
    } catch (IOException ioe) {
      return ioe.toString();
    }
    LuminanceSource source = new BufferedImageLuminanceSource(image);
    BinaryBitmap bitmap = new BinaryBitmap(new HybridBinarizer(source));
    Result result;
    try {
      result = new MultiFormatReader().decode(bitmap);
    } catch (ReaderException re) {
      return re.toString();
    }
    return String.valueOf(result.getText());
  }

}
