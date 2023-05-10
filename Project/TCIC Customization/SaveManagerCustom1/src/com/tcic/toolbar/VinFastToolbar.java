package com.tcic.toolbar;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;

import com.ebsolutions.uiapplication.ui.AbstractDisplayElement;
import com.ebsolutions.uiapplication.ui.AbstractToolBar;
import com.ebsolutions.uimanager.AbstractUIManagerApplication;
import com.tcic.actions.JFPartCreation;
import com.tcic.buttons.VinFastCustomButton;

/*
 * This sample is redefining a toolbar that creates a JMenu
 * Each display element is put in its category menu.
 * */
/**
 * @author ppo
 * 
 *         TODO To change the template for this generated type comment go to
 *         Window - Preferences - Java - Code Style - Code Templates
 */
public class VinFastToolbar extends AbstractToolBar {

	AbstractUIManagerApplication app = (AbstractUIManagerApplication) getApplication();

	JMenuBar bar = null;
	ArrayList<String> listMenu = null;

	protected Color darkColor = new Color(168, 167, 185);
	protected Color catiaColor = new Color(207, 207, 231);
	protected Color lightColor = new Color(241, 239, 226);
	protected Color background = new Color(225, 225, 225);

	/*
	 * Redefines the method that return the JComponent corresponding to our toolbar
	 * implementation
	 */
	public JComponent createComponent() {

		createToolBar();

		return bar;
	}

	public void createToolBar() {

		bar = new JMenuBar();
		bar.setBackground(catiaColor);
		listMenu = new ArrayList<String>();

		AbstractDisplayElement[] displayElement = getDisplayElements();
		/*
		 * This loop determines how many menus the toolbar will contain : There will be
		 * as many menus as the number of buttons categories
		 */
		for (int i = 0; i < displayElement.length; i++) {

			bar.add(displayElement[i].getDisplay());

			if (displayElement[i] instanceof VinFastCustomButton) {

				VinFastCustomButton elem = (VinFastCustomButton) displayElement[i];

				if (!listMenu.contains(elem.getCategory())) {

					listMenu.add(elem.getCategory());

				}
			}
		}

		/*
		 * We create as many menus as the number of categories and we add the buttons
		 * inside their corresponding menu
		 */
		for (int i = 0; i < listMenu.size(); i++) {

			JMenu menu = new JMenu(listMenu.get(i).toString()) {

				/**
				 * 
				 */
				protected void paintComponent(Graphics g) {
					super.paintComponent(g);

					int w = getWidth();
					int h = getHeight() + 2;
					int diviser = 6;
					Graphics2D g2d = (Graphics2D) g;
					g2d.setPaint(new GradientPaint(0, 0, lightColor, 0, h / diviser, catiaColor));
					g2d.fillRect(0, 0, w, h / diviser);

					g2d.setPaint(new GradientPaint(0, (diviser - 1) * h / diviser, catiaColor, 0, h + 1, darkColor));
					g2d.fillRect(0, (diviser - 1) * h / diviser, w, h);
				}
			};
			menu.setBorderPainted(false);
			bar.setBorderPainted(true);
			bar.setBorder(BorderFactory.createEmptyBorder());
			bar.setBackground(darkColor);

			bar.add(menu);

			for (int j = 0; j < displayElement.length; j++) {

				if (displayElement[j] instanceof VinFastCustomButton) {

					VinFastCustomButton elem = (VinFastCustomButton) displayElement[j];

					if (listMenu.get(i).toString().equals(elem.getCategory())) {
						menu.add(displayElement[j].getDisplay());
					}
				}
			}
		}
	}

	/*
	 * Returns an unique identifier of this toolbar
	 */

	public String getID() {
		return name;
	}

}