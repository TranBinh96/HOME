package com.tcic.tableeditor;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.util.Hashtable;
import java.util.List;
import java.util.concurrent.TimeUnit;

import javax.swing.JOptionPane;
import javax.swing.JTable;
import javax.swing.SwingUtilities;

import com.ebsolutions.lovpanel.lovproperty.ui.LovField;
import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uiapplication.InterfaceCATIATableModel;
import com.ebsolutions.uiapplication.toolbox.BooleanField;
import com.ebsolutions.uiapplication.ui.AbstractSaveModule;
import com.ebsolutions.uimanager.AbstractUIManagerApplication;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.ebsolutions.uimanager.ItemTypeListProperty;
import com.ebsolutions.uimanager.Property;
import com.ebsolutions.uimanager.TCTypeInfo;
import com.ebsolutions.uimanager.common.UIPreferencesManager;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.ebsolutions.uimanager.modules.TableEditor;
import com.ebsolutions.uimanager.modules.UIDescriptor;
import com.tcic.actions.ComponentAttribute;
import com.tcic.actions.JFPartCreation;
import com.tcic.actions.MPDesignCreation;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;

public class CustomizedTableEditor extends TableEditor implements ActionListener, ComponentAttribute {
	/**
	 * @author bangpq - VinFast Custom
	 */
	private static final long serialVersionUID = -6041257553720814775L;

	private SaveModule module = null;
	private UIDescriptor descriptor = null;
	private AbstractUIManagerApplication app = null;
	private static final int SLEEP_INTERVAL = 1; // count in second
	private String[] uneditableProps = null;

	public CustomizedTableEditor() {
		super();
	}

	@Override
	protected void performOKAction(ActionEvent e) {
		try {

			module = (SaveModule) getModule();
			app = (AbstractUIManagerApplication) module.getApplication();
			descriptor = getDescriptor();

			InterfaceCATIAComponent[] components = module.getSelectedComponents();

//			module.saveSelection();

			List<?> comps = getSelectedComponents();

			/* Loop on all the CATIA Components */
			if (comps == null) {
				JOptionPane.showMessageDialog(app, "Comp null.");
				return;
			}

			for (int i = 0; i < comps.size(); i++) {

				InterfaceCATIAComponent component = (InterfaceCATIAComponent) comps.get(i);
				String currType = getType(component);

				if (currType.equals(JF_PART)) {
					if (validAtt(JF_PART))
						continue;
					else {
						JOptionPane.showMessageDialog(app,
								component.getProperty(ITEM_ID).toString() + " contains invalid Attribute for Join File Part", "Error",
								JOptionPane.ERROR_MESSAGE, null);
						return;
					}
				} else if (currType.equals(MP_DESIGN)) {
					continue;
				}
			}

			super.performOKAction(e);

			try {
				TimeUnit.SECONDS.sleep(SLEEP_INTERVAL);
			} catch (InterruptedException e1) {
				e1.printStackTrace();
			}

			for (int i = 0; i < comps.size(); i++) {
				InterfaceCATIAComponent component = (InterfaceCATIAComponent) comps.get(i);
				String currType = getType(component);

				if (currType.equals(JF_PART)) {
					uneditableProps = JFPartCreation.getUnwantedProps();
					uneditableProps(component, uneditableProps);
				} else if (currType.equals(MP_DESIGN)) {
					uneditableProps = MPDesignCreation.getUnwantedProps();
					uneditableProps(component, uneditableProps);
				}
			}
		} catch (Exception e2) {
			System.out.println("updateSelection " + e2.getMessage().toString());
		}
	}

	private String getType(InterfaceCATIAComponent component) {

		Object[] typeLOV = ((ItemTypeListProperty) ((AbstractUIManagerComponent) component)
				.getComponentProperty(ITEM_TYPE)).getPropertiesList();

		int typeIndex = (int) ((AbstractUIManagerComponent) component).getComponentProperty(ITEM_TYPE).getValue();
		String currType = ((TCTypeInfo) typeLOV[typeIndex]).get_real_name();
		return currType;

	}

	private void uneditableProps(InterfaceCATIAComponent comp, String[] unwantedProps) {

		Thread appThread = new Thread() {
			@Override
			public void run() {
				try {
					SwingUtilities.invokeAndWait(new Runnable() {

						@Override
						public void run() {
							Property[] compProps = ((AbstractUIManagerComponent) comp)
									.getComponentProperties(unwantedProps);

							for (Property property : compProps) {
								if (property.isEditable()) 
									property.setEditable(false);
							}
							
							updateLayout();
						}
					});
				} catch (Exception e) {
					System.out.println("e2" + e.getMessage().toString());
				}
			}
		};

		appThread.start();
		
	}

	private boolean validAtt(String partType) {

		String validIMDS = "false";
		String validCategory = "NONE";
		String validMakeBuy = "Information";
		boolean rs = false;

		switch (partType) {
		case JF_PART:
			if (checkIMDS(validIMDS) && checkCategory(validCategory) && checkMakeBuy(validMakeBuy))
				rs = true;
			break;

		case MP_DESIGN:
			rs = true;
			break;

		default:
			rs = true;
			break;
		}

		return rs;
	}

	private boolean checkIMDS(String validValue) {
		boolean rs = false;

		Hashtable<?, ?> descComps = descriptor.getComponents();
		Hashtable<?, ?> uiComps = (Hashtable<?, ?>) descComps.get(CAT_PART);

		BooleanField imds = (BooleanField) uiComps.get(IMDS_OBJECT);
		String imdsVal = imds.getSelected();

		if (imdsVal.equals(validValue))
			rs = true;

		return rs;
	}

	private boolean checkCategory(String validValue) {
		boolean rs = false;

		Hashtable<?, ?> descComps = descriptor.getComponents();
		Hashtable<?, ?> uiComps = (Hashtable<?, ?>) descComps.get(CAT_PART);

		LovField category = (LovField) uiComps.get(PART_CATEGORY);
		String categoryVal = category.get_display_value();

		if (categoryVal.equals(validValue))
			rs = true;

		return rs;
	}

	private boolean checkMakeBuy(String validValue) {
		boolean rs = false;

		Hashtable<?, ?> descComps = descriptor.getComponents();
		Hashtable<?, ?> uiComps = (Hashtable<?, ?>) descComps.get(CAT_PART);

		LovField makeBuy = (LovField) uiComps.get(PART_MAKE_BUY);
		String makeBuyVal = makeBuy.get_display_value();

		if (makeBuyVal.equals(validValue))
			rs = true;

		return rs;
	}

}
