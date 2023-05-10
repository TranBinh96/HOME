package com.tcic.actions;

import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uimanager.AbstractUIManagerApplication;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.ebsolutions.uimanager.actions.CheckOutAction;
import com.ebsolutions.uimanager.common.UIPreferencesManager;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.tcic.utils.RollBackHandler;
import com.tcic.utils.XMLReader;
import com.teamcenter.soa.client.model.ModelObject;

import javax.swing.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

public class VFCheckOutAction extends CheckOutAction implements ComponentAttribute{
    private String saveMode;
    private AbstractUIManagerApplication app = null;

    @Override
    protected synchronized void doCheck(ModelObject[] comps, Vector<ModelObject> i_refresh_comps) {
        app = (AbstractUIManagerApplication) getApplication();
        //Check roll back option
        if (RollBackHandler.isRollbackAll() || RollBackHandler.isRollbackCoAction()) {
            System.out.println("VF_SYSTEM: Rollback => true");
            super.doCheck(comps, i_refresh_comps);
        } else {
            switch (validator()) {
                case STUDY_FAIL:
                    failureHandler(new STATUS[]{STATUS.STUDY_FAIL});
                    break;
                case DESIGN_FAIL:
                    break;
                case NULL_POINTER:
                    System.out.println("VF-ERROR: NULL_POINTER in HANDLE SAVE MODE");
                    break;
                default:
                    super.doCheck(comps, i_refresh_comps);
            }
        }
    }

    private STATUS validator() {
        saveMode = XMLReader.getSaveMode();
        STATUS ret = null;
        switch (isStudyMode()) {
            case PASS:
                ret = STATUS.PASS;
                break;
            case STUDY_FAIL:
                ret = STATUS.STUDY_FAIL;
                break;
            default:
                break;
        }

        switch (isDesignMode()) {
            case PASS:
                ret = STATUS.PASS;
                break;
            case DESIGN_FAIL:
                ret = STATUS.DESIGN_FAIL;
                break;
            default:
                break;
        }
        return ret == null ? STATUS.NULL_POINTER : ret;
    }

    private STATUS isStudyMode() {
        return saveMode.equalsIgnoreCase(STUDY_MODE) ? studyModeHandler() : STATUS.NOT_REQUIRE;
    }

    private STATUS isDesignMode() {
        return saveMode.equalsIgnoreCase(DESIGN_MODE) ? STATUS.PASS : STATUS.NOT_REQUIRE;
    }

    private STATUS studyModeHandler() {
        SaveModule module = app.getSaveModule();
        InterfaceCATIAComponent[] selComponents = module.getSelectedComponents();

        List<String> notAllowedList = new ArrayList<>();

        for (InterfaceCATIAComponent catiaComp : selComponents) {
            if (!isStudyPart(catiaComp)) {
                String id = catiaComp.getProperty(AbstractUIManagerComponent.ITEM_ID).toString();
                notAllowedList.add(id);
            }
        }
        if (!notAllowedList.isEmpty()) System.out.println("VF_SYSTEM: Item checking stops at => " + notAllowedList);

        return notAllowedList.isEmpty() ? STATUS.PASS : STATUS.STUDY_FAIL;
    }

    private boolean isStudyPart(InterfaceCATIAComponent catiaComp) {
        AbstractUIManagerComponent comp = (AbstractUIManagerComponent) catiaComp;
        String itemType = comp.getItemType();
        return itemType.equalsIgnoreCase(STUDY_PART_REAL_NAME);
    }

    private void failureHandler(STATUS[] failCode) {
        for (STATUS failure : failCode) {
            switch (failure) {
                case STUDY_FAIL:
                    JOptionPane.showMessageDialog(app, "You are currently in Study Save Mode. "
                                    + "Design Revision cannot be checked out. \n"
                                    + "Please save Design Revision as Study Part or Switch to Design Save Mode",
                            "Check-out Design Revision Error", JOptionPane.ERROR_MESSAGE, null);
                    break;
                case DESIGN_FAIL:
                    break;
                default:
                    break;
            }
        }
    }
}
