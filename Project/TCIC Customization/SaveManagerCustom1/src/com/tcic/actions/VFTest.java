package com.tcic.actions;

import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uiapplication.actions.AbstractAction;
import com.ebsolutions.uimanager.AbstractUIManagerApplication;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.ebsolutions.uimanager.Property;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.ebsolutions.uimanager.modules.TableEditor;
import com.ebsolutions.uimanager.modules.UIDescriptor;
import com.tcic.utils.Query;
import com.teamcenter.soa.client.model.ModelObject;

import java.awt.event.ActionEvent;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.List;

public class VFTest extends AbstractAction {
	private UIDescriptor uiDescriptor;
	private static final String PART_MAKE_BUY = "TC Item vf4_item_make_buy";
	protected AbstractUIManagerApplication app = null;
	protected SaveModule module = null;
	//	protected UIApplicationSession session = null;
	List<InterfaceCATIAComponent> selComp;

	@Override
	public void actionPerformed(ActionEvent e) {
		app = (AbstractUIManagerApplication) getApplication();
//		try {
//			module = (SaveModule) app.getCurrentModule();
//		} catch (ClassCastException cce) {
//			module = (SaveModule) app.getModule(SaveModule.DEFAULT_ID);
//		}
//		if (module == null) {
//			return;
//		}
		try {
			module = app.getSaveModule();
			TableEditor editor = (TableEditor) module.getTableEditor();
			uiDescriptor = editor.getDescriptor();
			module.saveSelection();
			List selComponents = module.getSavedSelection();
//			String id = ((InterfaceCATIAComponent)selComponents.get(0)).getProperty(AbstractUIManagerComponent.ITEM_ID).toString();

			//Get map of displaying property value
			Hashtable properties = uiDescriptor.getProperties(((InterfaceCATIAComponent)selComponents.get(0)).getComponentType());
			String uiID = ((Property) properties.get(AbstractUIManagerComponent.ITEM_ID)).getValue().toString();
//			String uiType = ((Property) properties.get(AbstractUIManagerComponent.ITEM_TYPE)).getValue().toString();
			AbstractUIManagerComponent component = (AbstractUIManagerComponent) selComponents.get(0);
//			boolean notExist = !isExisting(uiID, "Study Part");
//			if (notExist) {
					initFirstRev(component);
//			}
			String rev = component.getComponentProperty(AbstractUIManagerComponent.ITEM_REV_ID).getValue().toString();

			module.updateSelection();
		} catch (Exception e1) {
			e1.printStackTrace();
			System.out.println(e1.getMessage());
		}
	}

	private void initFirstRev(AbstractUIManagerComponent component) {
		component.getComponentProperty(AbstractUIManagerComponent.ITEM_REV_ID).setEditable(true);
		component.setProperty(AbstractUIManagerComponent.ITEM_REV_ID, "01");
		component.getComponentProperty(AbstractUIManagerComponent.ITEM_REV_ID).setEditable(false);
	}

	private boolean isExisting(String uiID, String uiType) {
		ModelObject[] rs = Query.savedQuery("Item...")
				.addCriteria("Item ID", uiID)
				.addCriteria("Type", uiType)
				.execute();
		return !(rs == null || rs.length == 0);
	}
}
