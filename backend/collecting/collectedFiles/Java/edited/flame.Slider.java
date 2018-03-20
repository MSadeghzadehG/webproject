

package com.badlogic.gdx.tools.flame;

import java.awt.BorderLayout;
import java.awt.Dimension;

import javax.swing.JPanel;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.event.ChangeListener;


public class Slider extends JPanel {
	public JSpinner spinner;

	public Slider (float initialValue, final float min, final float max, float stepSize) {
		spinner = new JSpinner(new SpinnerNumberModel(initialValue, min, max, stepSize));
		setLayout(new BorderLayout());
		add(spinner);
	}

	public void setValue (float value) {
		spinner.setValue((double)value);
	}

	public float getValue () {
		return ((Double)spinner.getValue()).floatValue();
	}

	public void addChangeListener (ChangeListener listener) {
		spinner.addChangeListener(listener);
	}

	public Dimension getPreferredSize () {
		Dimension size = super.getPreferredSize();
		size.width = 75;
		size.height = 26;
		return size;
	}
}
