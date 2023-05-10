package com.tcic.actions;

import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uiapplication.actions.AbstractAction;
import com.ebsolutions.uimanager.*;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.ebsolutions.uimanager.modules.TableEditor;
import com.ebsolutions.uimanager.modules.UIDescriptor;
import com.tcic.utils.Query;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.util.Hashtable;
import java.util.List;

public class VFTestNew extends AbstractAction {
	private UIDescriptor uiDescriptor;
	private static final String PART_MAKE_BUY = "TC Item vf4_item_make_buy";
	protected AbstractUIManagerApplication app = null;
	protected SaveModule module = null;
	//	protected UIApplicationSession session = null;
	List<InterfaceCATIAComponent> selComp;

	@Override
	public void actionPerformed(ActionEvent e) {
		 app = (AbstractUIManagerApplication) getApplication();
		SaveModule module = app.getSaveModule();
		module.saveSelection();
		TableEditor editor = (TableEditor) module.getTableEditor();
		UIDescriptor uiDescriptor = editor.getDescriptor();
		InterfaceCATIAComponent[] comps = app.getSaveModule().getSelectedComponents();

		boolean rs = preApplyAction(comps, uiDescriptor);

		module.updateSelection();
		if (!rs)
			JOptionPane.showMessageDialog(app,
					"Invalid Revision. Please assign revision", "Error", JOptionPane.ERROR_MESSAGE);
	}
	private boolean checkValidRev(InterfaceCATIAComponent[] comps, UIDescriptor uiDescriptor) {
		if (comps != null) {
			for (InterfaceCATIAComponent comp : comps) {
				AbstractUIManagerComponent component = (AbstractUIManagerComponent) comp;
				List<String> revList =
						Query.getRevList(component
								.getComponentProperty(AbstractUIManagerComponent.ITEM_ID)
								.getValue().toString(), getCurDisplayType(comp));

				int latestRev = 0;
				for (String rev : revList) {
					if (rev.matches("^\\d{2}$") && Integer.parseInt(rev) > latestRev)
						latestRev = Integer.parseInt(rev);
				}
				String possibleRev = (latestRev + 1 < 10 ? "0" : "") + (latestRev + 1);
				String curRev = getCurRev(comp, uiDescriptor);
				if (!revList.contains(curRev) && !curRev.equals(possibleRev)) {
					return false;
				}
			}
		}
		return true;
	}

	private boolean preApplyAction(InterfaceCATIAComponent[] comps, UIDescriptor uiDescriptor) {
		return checkValidRev(comps, uiDescriptor);
	}

	private String getCurRev(InterfaceCATIAComponent comp, UIDescriptor uiDescriptor) {
		Hashtable properties = uiDescriptor.getProperties(comp.getComponentType());
		return ((Property) properties.get(AbstractUIManagerComponent.ITEM_REV_ID)).getValue().toString();
	}

	protected String getCurDisplayType(InterfaceCATIAComponent comp) {
		AbstractUIManagerComponent component = (AbstractUIManagerComponent) comp;
		Object[] typeLOV =
				((ItemTypeListProperty) component
						.getComponentProperty(AbstractUIManagerComponent.ITEM_TYPE))
						.getPropertiesList();
		int typeIndex = (int) component.getComponentProperty(AbstractUIManagerComponent.ITEM_TYPE).getValue();

		return ((TCTypeInfo) typeLOV[typeIndex]).get_display_name();
	}
}
