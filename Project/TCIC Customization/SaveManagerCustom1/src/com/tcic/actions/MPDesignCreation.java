package com.tcic.actions;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;

import javax.swing.JOptionPane;

import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uimanager.AbstractUIManagerApplication;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.ebsolutions.uimanager.AbstractUIManagerSession;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.ebsolutions.uimanager.modules.TableEditor;
import com.ebsolutions.utility.io.Tools;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;
import com.teamcenter.soa.client.model.ModelObject;

public class MPDesignCreation extends AbstractCustomAction implements ComponentAttribute, ActionListener {
	/**
	 * @author bangpq - VinFast Custom
	 */
	private final static String[] UNWANTED_PROPS = { ITEM_TYPE, ITEM_ID, DONOR_VEHICLE, ITEM_NAME };

	public static String[] getUnwantedProps() {
		return UNWANTED_PROPS;
	}

	public synchronized void actionPerformed(ActionEvent e) {
		super.actionPerformed(e);

		/* Save the table selection */
		module.saveSelection();

		InterfaceCATIAComponent[] comps = module.getSelectedComponents();

//		Loop on all the CATIA Components

		if (comps == null)
			return;

//			session.addSessionListener(module);

		List<InterfaceCATIAComponent> validComps = new ArrayList<>();

		List<InterfaceCATIAComponent> invalidComps = new ArrayList<>();

		for (int i = 0; i < comps.length; i++) {

			AbstractUIManagerComponent component = (AbstractUIManagerComponent) comps[i];

			if (matchType(component, MP_DESIGN) && !component.getComponentProperty(ITEM_ID).isEditable())
				continue;

			String inputID = getUserInputID();

			if (typeFromID(inputID) != null && !inputID.equals("Multiple values")
					&& typeFromID(inputID).equals(MP_DESIGN))

				component.setComponentProperty(ITEM_ID, inputID);

			if (typeFromID(component) != null && typeFromID(component).equals(MP_DESIGN)) {

				validComps.add((InterfaceCATIAComponent) component);

				// Set component to save mode
				component.setSaveMode(AbstractUIManagerComponent.SAVE_AS);

				setType(component);

				session.checkWithResetProps(component);

				if (!inputID.equals("Multiple values") && typeFromID(inputID).equals(MP_DESIGN))
					component.setComponentProperty(ITEM_ID, inputID);

				if (!isInputExist(inputID, MP_DESIGN_DN)) {
					initRev(component);
				}

				if (getParentDonor(inputID) != "") {
					String parentDonor = getParentDonor(inputID);
					initDonor(component, parentDonor);
				}

				uneditableProps(component, UNWANTED_PROPS);

			} else {

				invalidComps.add((InterfaceCATIAComponent) component);
			}
		}

// 		Warning
		if (invalidComps.size() > 0 && validComps.size() > 0) {

			warning(app, validComps, invalidComps);

		} else if (invalidComps.size() > 0 && validComps.size() == 0) {

			warning(app, invalidComps);

		} else if (invalidComps.size() == 0 && validComps.size() > 0) {

			JOptionPane.showMessageDialog(app, "The process has successfully completed");

		}

//		Reselect the table lines are they were before the set property 
		module.updateSelection();

	}

	private String getUserInputID() {

		TableEditor tableEditor = (TableEditor) module.getTableEditor();

//		String idText = (String) tableEditor.getKeyList().get("item_id");
//
//		return idText;
		return "";
	}
}