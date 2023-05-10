package com.tcic.tableeditor;

import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uiapplication.ui.AbstractApplication;
import com.ebsolutions.uimanager.AbstractUIManagerApplication;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.ebsolutions.uimanager.common.UIPreferencesManager;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.ebsolutions.uimanager.modules.TableEditor;
import com.tcic.actions.AssemblyInfo;
import com.tcic.actions.ComponentAttribute;
import com.tcic.handlers.VFRevisionHandler;
import com.tcic.utils.RollBackHandler;
import com.tcic.utils.XMLReader;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.util.Collection;
import java.util.HashMap;

public class VFTableEditor extends TableEditor implements ComponentAttribute {

    @Override
    protected void performOKAction(ActionEvent e) {
        VFRevisionHandler revisionHandler = new VFRevisionHandler((AbstractUIManagerApplication) getModule().getApplication());
        boolean isValidRev = revisionHandler.actionPerformed();

        if (!isValidRev) return; //Invalid revision

        if (RollBackHandler.isRollbackAll() || RollBackHandler.isRollbackApplyButtonAction()) {
            System.out.println("VF_SYSTEM: Rollback => true");
            super.performOKAction(e);
        } else {
            String saveMode = this.checkSaveMode();
            SaveModule module = (SaveModule) getModule();
            InterfaceCATIAComponent[] selComponents = module.getSelectedComponents();
            module.saveSelection();
            try {
                if (isStudyMode(saveMode)) {
                    super.performOKAction(e);
                }
                if (isDesignMode(saveMode, selComponents)) {
                    super.performOKAction(e);
                }
            } catch (Exception e1) {
                e1.printStackTrace();
            }

        }
    }

    private boolean isDesignMode(String saveMode, InterfaceCATIAComponent[] selComponents) {
        return saveMode.equalsIgnoreCase(DESIGN_MODE) ? designModeHandler(selComponents) : false;
    }

    private boolean designModeHandler(InterfaceCATIAComponent[] selComponents) {
//        XMLReader.readXML();
        HashMap<String, AssemblyInfo> partMakeBuyList = XMLReader.getPartMakeBuyList();
        Collection<String> idList = partMakeBuyList.keySet();

        for (InterfaceCATIAComponent comp : selComponents) {
            AbstractUIManagerComponent component = (AbstractUIManagerComponent) comp;
            String itemID = component.getProperty(AbstractUIManagerComponent.ITEM_ID).toString();

            if (idList.contains(itemID)) {
                String makeBuyVal = partMakeBuyList.get(itemID).getPartMakeBuy();
                component.setComponentProperty(PART_MAKE_BUY, makeBuyVal);
                System.out.println(itemID + "=> " + makeBuyVal + " applied");
            }
        }
        return true;
    }

    private boolean isStudyMode(String saveMode) {
        return saveMode.equalsIgnoreCase(STUDY_MODE) ? studyModeHandler() : false;
    }

    private boolean studyModeHandler() {
        return true;
    }


    private boolean checkRollbackOption() {
        try {
            String[] tcicRollbackOptions = UIPreferencesManager.getPreferenceValues("CATIA_vf_tcic_rollback_option");
            return tcicRollbackOptions[0] != null && Boolean.parseBoolean(tcicRollbackOptions[0]);
        } catch (Exception e) {
            System.out.println("VF-ERROR: Fail to get pref: vf_tcic_rollback_option => Set Rollback to true");
            return true;
        }
    }

    private String checkSaveMode() {
        String saveMode = XMLReader.getSaveMode();
        if (!(saveMode.equalsIgnoreCase(STUDY_MODE) || saveMode.equalsIgnoreCase(DESIGN_MODE))) {
            System.out.println("VF-ERROR: Incorrect Save Mode => " + saveMode);
            saveMode = handleNullStudyMode();
        }
        System.out.println("/n VF SYSTEM: Save Mode => " + saveMode);

        return saveMode;
    }

    private String handleNullStudyMode() {
        AbstractApplication app = getModule().getApplication();
        String[] options = new String[]{"Study Session", "Design Session"};
        int selection = JOptionPane.showOptionDialog(app, "Please select Save Mode:", "Save Mode Selector",
                JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE, null, options, options[0]);
        if (selection == 0)
            return options[0];
        else
            return options[1];
    }
}