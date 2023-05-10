package com.tcic.handlers;

import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uimanager.AbstractUIManagerApplication;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.ebsolutions.uimanager.ItemTypeListProperty;
import com.ebsolutions.uimanager.TCTypeInfo;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.tcic.actions.ComponentAttribute;
import com.tcic.utils.XMLReader;

import javax.swing.*;
import java.util.ArrayList;
import java.util.List;

import static com.tcic.actions.ComponentAttribute.STUDY_MODE;
import static com.tcic.actions.ComponentAttribute.STUDY_PART_REAL_NAME;

public class VFStudyModeVFHandler implements VFHandlerStatus {
    public AbstractUIManagerApplication app;
    private static final String ACTION_APPLY = "tableEditor_okAction";
    private static final String ACTION_SAVE = "application_saveAction";
    private static final String ACTION_CO = "check_out";
    private final String vfSaveMode;
    private static VFStudyModeVFHandler instance;
    private List<STUDY_MODE_ERROR> errorList = new ArrayList<>();

    public void setErrorList(List<STUDY_MODE_ERROR> errorList) {
        this.errorList = errorList;
    }

    public static VFStudyModeVFHandler getInstance() {
        return instance;
    }

    public static VFStudyModeVFHandler getInstance(AbstractUIManagerApplication app) {
        return instance == null ? new VFStudyModeVFHandler(app) : instance;
    }

    public VFStudyModeVFHandler(AbstractUIManagerApplication app) {
        this.app = app;
        vfSaveMode = XMLReader.getSaveMode();
    }

    public STATUS performActioned(String action) {
        return vfSaveMode.equalsIgnoreCase(STUDY_MODE) ? studyModeHandle(action) : STATUS.NOT_REQUIRE;
    }

    private STATUS studyModeHandle(String action) {
        switch (action) {
            case ACTION_APPLY:
                return actionApplyHandle();
            case ACTION_SAVE:
                return actionSaveHandle();
            case ACTION_CO:
                return actionCOHandle();
            default:
                return STATUS.PASS;
        }
    }

    private STATUS actionCOHandle() {
        return this.coHandler();
    }

    private STATUS coHandler() {
        List<String> notAllowedList = new ArrayList<>();

        SaveModule module = app.getSaveModule();
        InterfaceCATIAComponent[] selComponents = module.getSelectedComponents();

        for (InterfaceCATIAComponent comp : selComponents) {
            if (!isStudyPart(comp)) {
                String id = comp.getProperty(AbstractUIManagerComponent.ITEM_ID).toString();
                notAllowedList.add(id);
            }
        }
        if (!notAllowedList.isEmpty()) {
            errorList.add(STUDY_MODE_ERROR.ERROR_1);
            System.out.println("VF_SYSTEM: Item checking stops at => " + notAllowedList);
        }

        return notAllowedList.isEmpty() ? STATUS.PASS : STATUS.STUDY_FAIL;
    }

    private boolean isStudyPart(InterfaceCATIAComponent catiaComp) {
        AbstractUIManagerComponent comp = (AbstractUIManagerComponent) catiaComp;
        Object[] typeLOV =
                ((ItemTypeListProperty) comp
                        .getComponentProperty(AbstractUIManagerComponent.ITEM_TYPE))
                        .getPropertiesList();
        int typeIndex = (int) comp.getComponentProperty(AbstractUIManagerComponent.ITEM_TYPE).getValue();

        return ((TCTypeInfo) typeLOV[typeIndex]).get_real_name().equalsIgnoreCase(STUDY_PART_REAL_NAME);
    }

    private void failureHandler(ComponentAttribute.STATUS[] failCode) {
        for (ComponentAttribute.STATUS failure : failCode) {
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

    private STATUS actionSaveHandle() {
        return STATUS.PASS;
    }

    private STATUS actionApplyHandle() {
        return STATUS.PASS;
    }
}
