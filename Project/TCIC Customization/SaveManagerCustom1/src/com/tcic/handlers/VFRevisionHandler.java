package com.tcic.handlers;

import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uimanager.*;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.ebsolutions.uimanager.modules.TableEditor;
import com.ebsolutions.uimanager.modules.UIDescriptor;
import com.tcic.utils.Query;

import javax.swing.*;
import java.util.Hashtable;
import java.util.List;

public class VFRevisionHandler {
    public AbstractUIManagerApplication app;

    public VFRevisionHandler(AbstractUIManagerApplication app) {
        this.app = app;
    }

    public boolean actionPerformed() {
        SaveModule module = app.getSaveModule();
        module.saveSelection();
        TableEditor editor = (TableEditor) module.getTableEditor();
        UIDescriptor uiDescriptor = editor.getDescriptor();
        InterfaceCATIAComponent[] comps = app.getSaveModule().getSelectedComponents();

        boolean rs = preApplyAction(comps, uiDescriptor);

        module.updateSelection();
        if (!rs)
        {
            JOptionPane.showMessageDialog(app,
                    "Invalid Revision. Please assign correct revision", "Error", JOptionPane.ERROR_MESSAGE);
        }
        return rs;
    }

    private boolean validateRev(InterfaceCATIAComponent[] comps, UIDescriptor uiDescriptor) {
        if (comps != null) {
            for (InterfaceCATIAComponent comp : comps) {
                AbstractUIManagerComponent component = (AbstractUIManagerComponent) comp;
                List<String> revList =
                        Query.getRevList(component
                                .getComponentProperty(AbstractUIManagerComponent.ITEM_ID)
                                .getValue().toString(), getCurDisplayType(comp));
                String curRev = getCurRev(comp, uiDescriptor);
                if (!isValid(revList, curRev))
                    return false;
            }
        }
        return true;
    }

    private boolean isValid(List<String> revList, String curRev) {
        if (curRev.matches("^\\d{2}$"))
            return false;

        int latestRev = 0;
        for (String rev : revList) {
            if (rev.matches("^\\d{2}$") && Integer.parseInt(rev) > latestRev)
                latestRev = Integer.parseInt(rev);
        }
        String possibleRev = (latestRev + 1 < 10 ? "0" : "") + (latestRev + 1);

        if (!revList.contains(curRev) && !curRev.equals(possibleRev)) {
            return false;
        } else {
            return true;
        }
    }

    private boolean preApplyAction(InterfaceCATIAComponent[] comps, UIDescriptor uiDescriptor) {
        return validateRev(comps, uiDescriptor);
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
