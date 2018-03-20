

package com.badlogic.gdx.tests.lwjgl;

import java.awt.BorderLayout;
import java.awt.HeadlessException;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.DefaultListSelectionModel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ListSelectionModel;
import javax.swing.UIManager;

import com.badlogic.gdx.Preferences;
import com.badlogic.gdx.backends.lwjgl.LwjglApplication;
import com.badlogic.gdx.backends.lwjgl.LwjglApplicationConfiguration;
import com.badlogic.gdx.backends.lwjgl.LwjglFiles;
import com.badlogic.gdx.backends.lwjgl.LwjglPreferences;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.graphics.glutils.ShaderProgram;
import com.badlogic.gdx.tests.utils.GdxTest;
import com.badlogic.gdx.tests.utils.GdxTests;

public class LwjglTestStarter extends JFrame {
	public LwjglTestStarter () throws HeadlessException {
		super("libgdx Tests");
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setContentPane(new TestList());
		pack();
		setSize(getWidth(), 600);
		setLocationRelativeTo(null);
		setVisible(true);
	}

	
	public static boolean runTest (String testName) {
		boolean useGL30 = false;
		GdxTest test = GdxTests.newTest(testName);
		if (test == null) {
			return false;
		}
		LwjglApplicationConfiguration config = new LwjglApplicationConfiguration();
		config.width = 640;
		config.height = 480;
		config.title = testName;
		config.forceExit = false;
		if (useGL30) {
			config.useGL30 = true;
			ShaderProgram.prependVertexCode = "#version 140\n#define varying out\n#define attribute in\n";
			ShaderProgram.prependFragmentCode = "#version 140\n#define varying in\n#define texture2D texture\n#define gl_FragColor fragColor\nout vec4 fragColor;\n";
		} else {
			config.useGL30 = false;
			ShaderProgram.prependVertexCode = "";
			ShaderProgram.prependFragmentCode = "";			
		}
		new LwjglApplication(test, config);
		return true;
	}

	class TestList extends JPanel {
		public TestList () {
			setLayout(new BorderLayout());

			final JButton button = new JButton("Run Test");

			final JList list = new JList(GdxTests.getNames().toArray());
			JScrollPane pane = new JScrollPane(list);

			DefaultListSelectionModel m = new DefaultListSelectionModel();
			m.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
			m.setLeadAnchorNotificationEnabled(false);
			list.setSelectionModel(m);

			list.addMouseListener(new MouseAdapter() {
				public void mouseClicked (MouseEvent event) {
					if (event.getClickCount() == 2) button.doClick();
				}
			});

			list.addKeyListener(new KeyAdapter() {
				public void keyPressed (KeyEvent e) {
					if (e.getKeyCode() == KeyEvent.VK_ENTER) button.doClick();
				}
			});

			final Preferences prefs = new LwjglPreferences(new FileHandle(new LwjglFiles().getExternalStoragePath()
				+ ".prefs/lwjgl-tests"));
			list.setSelectedValue(prefs.getString("last", null), true);

			button.addActionListener(new ActionListener() {
				@Override
				public void actionPerformed (ActionEvent e) {
					String testName = (String)list.getSelectedValue();
					prefs.putString("last", testName);
					prefs.flush();
					dispose();
					runTest(testName);
				}
			});

			add(pane, BorderLayout.CENTER);
			add(button, BorderLayout.SOUTH);

								}
	}

	
	public static void main (String[] argv) throws Exception {
		if (argv.length > 0) {
			if (runTest(argv[0])) {
				return;
							}
		}
		UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		new LwjglTestStarter();
	}
}
