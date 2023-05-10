package com.tcic.entrypoints;

import com.ebsolutions.savemanager.SaveManagerApplication;
import com.ebsolutions.savemanager.extension.IEntryPointForButtonAction;
import com.ebsolutions.seedpanel.SeedApplication;
import com.ebsolutions.statedetails.StateDetailsApplication;
import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.ebsolutions.uimanager.ItemTypeListProperty;
import com.ebsolutions.uimanager.Property;
import com.ebsolutions.uimanager.TCTypeInfo;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.ebsolutions.uimanager.modules.UIDescriptor;
import com.tcic.utils.Query;
import com.tcic.utils.RollBackHandler;

import java.util.Hashtable;
import java.util.List;

public class SaveManagerPostAction implements IEntryPointForButtonAction {
    private static final String APPLY_ACTION = "table_editor_okAction";

    @Override
    public boolean userEntryPreButtonAction(String s, SaveManagerApplication saveManagerApplication) {
        return true;
    }

    @Override
    public boolean userEntryPostButtonAction(String s, SaveManagerApplication saveManagerApplication) {
        System.out.println("Post action of button" + s);

        if (RollBackHandler.isRollbackAll() || RollBackHandler.isRollbackAllPostAction())
            return true;

        if (s.equalsIgnoreCase("create_new") || s.equalsIgnoreCase(APPLY_ACTION)
                || s.equalsIgnoreCase("new_revision")) {
            SaveModule module = saveManagerApplication.getSaveModule();
            module.saveSelection();

            InterfaceCATIAComponent[] comps = saveManagerApplication.getSaveModule().getSelectedComponents();
            if (!RollBackHandler.isRollbackCreateNewPostAction() && s.equalsIgnoreCase("create_new")) {
                postCreateNewAction(comps);
            }
            if (!RollBackHandler.isRollbackApplyButtonPostAction() && s.equalsIgnoreCase(APPLY_ACTION)) {
//                postApplyAction(comps);
            }
            module.updateSelection();
        }
        return true;
    }

    @Override
    public boolean userEntryPreButtonAction(String s, StateDetailsApplication stateDetailsApplication) {
        return true;
    }

    @Override
    public boolean userEntryPostButtonAction(String s, StateDetailsApplication stateDetailsApplication) {
        return true;
    }

    @Override
    public boolean userEntryPreButtonAction(String s, SeedApplication seedApplication) {
        return true;
    }

    @Override
    public boolean userEntryPostButtonAction(String s, SeedApplication seedApplication) {
        return true;
    }

    private void postCreateNewAction(InterfaceCATIAComponent[] comps) {
        if (comps != null) {
            for (InterfaceCATIAComponent comp : comps) {
                setProperty(comp, AbstractUIManagerComponent.ITEM_REV_ID, "01");
            }
        }
    }

    private void setProperty(InterfaceCATIAComponent comp, String i_property_key, Object value) {
        AbstractUIManagerComponent component = (AbstractUIManagerComponent) comp;
        component.getComponentProperty(AbstractUIManagerComponent.ITEM_REV_ID).setEditable(true);
        component.setProperty(i_property_key, value);
        component.getComponentProperty(AbstractUIManagerComponent.ITEM_REV_ID).setEditable(false);
    }

    private String getCurDisplayType(InterfaceCATIAComponent comp) {
        AbstractUIManagerComponent component = (AbstractUIManagerComponent) comp;
        Object[] typeLOV =
                ((ItemTypeListProperty) component
                        .getComponentProperty(AbstractUIManagerComponent.ITEM_TYPE))
                        .getPropertiesList();
        int typeIndex = (int) component.getComponentProperty(AbstractUIManagerComponent.ITEM_TYPE).getValue();

        return ((TCTypeInfo) typeLOV[typeIndex]).get_display_name();
    }

    private void postApplyAction(InterfaceCATIAComponent[] comps) {
        if (comps != null) {
            for (InterfaceCATIAComponent comp : comps) {
                AbstractUIManagerComponent component = (AbstractUIManagerComponent) comp;
                int saveMode = component.getSaveAsMode();
                if (saveMode == AbstractUIManagerComponent.NEW_REVISION || saveMode == AbstractUIManagerComponent.SAVE_AS) {
                    int latestRev =
                            Query.latestRev(component
                                            .getComponentProperty(AbstractUIManagerComponent.ITEM_ID)
                                            .getValue().toString(),
                                    getCurDisplayType(comp));

                    if (saveMode == AbstractUIManagerComponent.NEW_REVISION) {
                        assignNextRevision(comp, latestRev);
                    }
                    if (saveMode == AbstractUIManagerComponent.SAVE_AS) {
                        assignLatestRevision(comp, latestRev);
                    }
                }
            }
        }
    }

    private void assignNextRevision(InterfaceCATIAComponent comp, int latestRev) {
        String rev2Assign = (latestRev != 0 && latestRev + 1 < 10 ? "0" : "") + (latestRev + 1);
        setProperty(comp, AbstractUIManagerComponent.ITEM_REV_ID, rev2Assign);
    }

    private void assignLatestRevision(InterfaceCATIAComponent comp, int latestRev) {
        String rev2Assign;
        if (latestRev == 0)
            rev2Assign = "01";
        else
            rev2Assign = latestRev < 10 ? "0" : "" + latestRev;
        setProperty(comp, AbstractUIManagerComponent.ITEM_REV_ID, rev2Assign);
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
                String curID = getCurID(comp, uiDescriptor);
                if (!revList.contains(curID) || !curID.equals(possibleRev)) {
                    return false;
                }
            }
        }
        return true;
    }

    private boolean preApplyAction(InterfaceCATIAComponent[] comps, UIDescriptor uiDescriptor) {
        return checkValidRev(comps, uiDescriptor);
    }

    private String getCurID(InterfaceCATIAComponent comp, UIDescriptor uiDescriptor) {
        Hashtable properties = uiDescriptor.getProperties(comp.getComponentType());
        return ((Property) properties.get(AbstractUIManagerComponent.ITEM_ID)).getValue().toString();
    }

}
