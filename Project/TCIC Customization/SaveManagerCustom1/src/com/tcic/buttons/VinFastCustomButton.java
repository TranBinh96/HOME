package com.tcic.buttons;

import java.awt.Color;
import java.awt.event.ActionListener;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JMenuItem;
import javax.swing.SwingConstants;

import org.xml.sax.Attributes;

import com.ebsolutions.uiapplication.ui.AbstractDisplayElement;

/*Custom button for Join File Creation*/

public class VinFastCustomButton extends AbstractDisplayElement {

	JMenuItem button = null;
	Color color = null;
	ActionListener listener = null;
	protected String category = null;

	public JComponent createComponent() {
		JMenuItem jMenuItem = new JMenuItem();
		button = jMenuItem;
		button.setBorder(null);
		button.setBackground(color);
		button.setIcon(new ImageIcon(getIcon()));
		button.setText(name);

		button.addActionListener(getAction());
		button.setHorizontalTextPosition(SwingConstants.TRAILING);
		button.setOpaque(false);
		button.setContentAreaFilled(false);
		button.setBorderPainted(false);
		
		
		return jMenuItem;
	}

	@Override
	public void initialize(Attributes attributes) {
		super.initialize(attributes);
		category = attributes.getValue("category");

	}

	@Override
	public String getID() {
		// TODO Auto-generated method stub
		return name;
	}

	public String getCategory() {
		// TODO Auto-generated method stub
		return category;
	}
}
