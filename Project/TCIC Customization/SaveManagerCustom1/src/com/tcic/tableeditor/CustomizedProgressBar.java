package com.tcic.tableeditor;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Frame;

import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JProgressBar;

public class CustomizedProgressBar {

	private JProgressBar progressBar = null;
	private JDialog jDialog = null;

	protected void progressBar(Frame parent) {

		jDialog = new JDialog(parent, true);
		JPanel pane = new JPanel();
		progressBar = new JProgressBar();

		progressBar.setValue(0);
		progressBar.setStringPainted(true);
		progressBar.setIndeterminate(true);
		progressBar.setPreferredSize(new Dimension(parent.getWidth() / 3, 25));

		pane.add(progressBar, BorderLayout.CENTER);
		pane.setPreferredSize(new Dimension(parent.getWidth() / 3 + 20, 25 + 10));
//		dialog.setUndecorated(true);
//		dialog.setMinimumSize(new Dimension(parent.getWidth() / 3 + 10, 30));
		jDialog.add(pane);
		jDialog.pack();
//		dialog.getRootPane().setOpaque(false);
//		dialog.getContentPane().setBackground(new Color(0, 0, 0));
//		dialog.setPreferredSize(new Dimension(parent.getWidth(), parent.getHeight()));
		jDialog.setLocationRelativeTo(parent);

		jDialog.setVisible(true);
	}

	protected void progressDone() {

		jDialog.setVisible(false);
	}

	protected JComponent glassPane(Frame parent) {

		JComponent glassPane = new JComponent() {

			/**
			 * 
			 */
			private static final long serialVersionUID = 1L;
		};
		glassPane.setOpaque(false);
		glassPane.setBackground(new Color(0, 0, 0, 50));
		glassPane.setPreferredSize(new Dimension(parent.getWidth(), parent.getHeight()));

		return glassPane;
	}
}
