package com.tcic.displayelements;

import com.ebsolutions.uiapplication.ui.AbstractDisplayElement;
import com.tcic.utils.XMLReader;

import javax.swing.*;
import java.awt.*;

public class SaveModeLabel extends AbstractDisplayElement {
    JLabel label = null;

    @Override
    public String getID() {
        return name;
    }

    @Override
    public JComponent createComponent() {
        label = new JLabel();
        label.setText("SAVE MODE: " + XMLReader.getSaveMode());
        label.setFont(new Font(Font.SANS_SERIF, Font.BOLD, 13));
        return label;
    }
}
