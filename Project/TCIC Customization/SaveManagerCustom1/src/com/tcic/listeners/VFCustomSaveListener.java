package com.tcic.listeners;

import com.ebsolutions.savemanager.SaveManagerApplication;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.ebsolutions.uimanager.Property;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.ebsolutions.uimanager.modules.TableEditor;
import com.tcic.actions.AssemblyInfo;
import com.tcic.handlers.VFHandlerStatus;
import com.tcic.handlers.VFDesignModeVFHandler;
import com.tcic.handlers.VFStudyModeVFHandler;
import com.tcic.utils.XMLReader;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Set;

import static com.tcic.actions.ComponentAttribute.*;

public class VFCustomSaveListener implements ActionListener, VFHandlerStatus {
    private static final String ACTION_SAVE = "application_saveAction";
    private final SaveManagerApplication app;
    private final ActionListener saveListener;
    private final ActionListener coListener;
    private static final String CHECKED_IN = "0";
    private static final String CHECKED_OUT = "1";
    private static final String CHECKED_OUT_NOT_ALLOWED = "2";
    private final String saveMode;
    private VFDesignModeVFHandler designModeHandler;
    private VFStudyModeVFHandler studyModeHandler;
    private static VFCustomSaveListener instance;

    public static VFCustomSaveListener getInstance(SaveManagerApplication app, ActionListener saveListener, ActionListener coListener) {
        return instance == null ? new VFCustomSaveListener(app, saveListener, coListener) : instance;
    }

    public VFCustomSaveListener(SaveManagerApplication app, ActionListener saveListener, ActionListener coListener) {
        saveMode = XMLReader.getSaveMode();
        this.app = app;
        this.saveListener = saveListener;
        this.coListener = coListener;
        designModeHandler = VFDesignModeVFHandler.getInstance(app);
        studyModeHandler = VFStudyModeVFHandler.getInstance(app);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        switch (validator(e)) {
            case STUDY_FAIL:
                failureHandler(new STATUS[]{STATUS.STUDY_FAIL});
                break;
            case DESIGN_FAIL:
                failureHandler(new STATUS[]{STATUS.DESIGN_FAIL});
                break;
            case NULL_POINTER:
                System.out.println("VF-ERROR: NULL_POINTER in HANDLE SAVE MODE");
                break;
            default:
                saveListener.actionPerformed(e);
        }
    }

    private void failureHandler(STATUS[] statuses) {

    }

    private STATUS validator(ActionEvent e) {
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

        switch (isDesignMode(e)) {
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

    private HashMap<String, String> getRequiredProperties(AbstractUIManagerComponent comp, String[] properties) {
        Property[] compProp = comp.getComponentProperties(properties);

        HashMap<String, String> propertiesMap = new HashMap<>();
        for (int i = 0; i < properties.length; i++) {
            propertiesMap.put(properties[i], compProp[i].getValue().toString());
        }

        return propertiesMap;
    }

    private boolean coRequired(HashMap<String, String> properties, List<String> combinedList) {
        return combinedList.contains(properties.get(ITEM_ID)) || combinedList.contains(properties.get(DATASET_UID));
    }

    private boolean checkCOStatus(String coStatus, String id) {
        switch (coStatus) {
            case CHECKED_IN:
                return true;
            case CHECKED_OUT:
                System.out.println("VF_INFO: " + id + " already checked out");
                return false;
            case CHECKED_OUT_NOT_ALLOWED:
                System.out.println("VF_INFO: " + id + " is not allowed to check out");
                return false;
            default:
                System.out.println("VF_ERROR: " + id + " has unknown check-out status. > " + coStatus + " <");
                return false;
        }
    }

    private STATUS isStudyMode() {
        return saveMode.equalsIgnoreCase(STUDY_MODE) ? VFStudyModeVFHandler.getInstance(app).performActioned("application_saveAction") : STATUS.NOT_REQUIRE;
    }

    private STATUS isDesignMode(ActionEvent e) {
        return saveMode.equalsIgnoreCase(DESIGN_MODE) ? designModeHandler(e) : STATUS.NOT_REQUIRE;
    }

    private STATUS designModeHandler(ActionEvent e) {
        XMLReader.readXML();
        //Get Modified Assemblies from XML
        List<AssemblyInfo> listAssem = XMLReader.getModifiedAssembliesList();
        HashMap<AbstractUIManagerComponent, String> checkOutNeeded = new HashMap<>();
        if (listAssem != null && !listAssem.isEmpty()) {
            checkOutNeeded = handleModifiedList();
        }
        return checkOutNeeded.size() == 0 ? STATUS.PASS : handleCheckoutList(checkOutNeeded, e);
    }

    private STATUS handleCheckoutList(HashMap<AbstractUIManagerComponent, String> checkOutNeeded, ActionEvent e) {
        StringBuilder errorMess = new StringBuilder();
        Object[] ids = checkOutNeeded.values().toArray();

        for (int i = 0; i < ids.length; i++) {
            errorMess.append(ids[i]);
            if (i == ids.length - 1) break;
            if (i % 4 == 0) errorMess.append("/n");
            else errorMess.append(",");
        }

        int reply = JOptionPane.showConfirmDialog(app, "For E-BOM Quality Assurance. You session cannot finish without following assemblies:\n" + errorMess + "\nDo you want to checkout them?", "Assembly Check-out require", JOptionPane.YES_OPTION);

        if (reply == JOptionPane.YES_OPTION) {
            autoCheckout(checkOutNeeded, e);
            return STATUS.PASS;
        } else {
            return STATUS.DESIGN_FAIL;
        }
    }

    private void autoCheckout(HashMap<AbstractUIManagerComponent, String> checkOutNeeded, ActionEvent e) {
        Set<AbstractUIManagerComponent> compSet = checkOutNeeded.keySet();
        try {
            //Get action checkout
            SaveModule saveModule = app.getSaveModule();
            TableEditor tableEditor = (TableEditor) saveModule.getTableEditor();
            //Select wanted components
            List<AbstractUIManagerComponent> comps = new ArrayList<>(compSet);
            tableEditor.setSelectedComponents(comps);
            saveModule.saveSelection();
            saveModule.updateSelection(); //Update Selection UI
            coListener.actionPerformed(e); //Check-out action
        } catch (Exception ex) {
            ex.printStackTrace();
            System.out.println("VF_ERROR: Check-out process");
        }

    }

    private HashMap<AbstractUIManagerComponent, String> handleModifiedList() {
        HashMap<AbstractUIManagerComponent, String> checkOutNeeded = new HashMap<>();
        List<String> combinedList = this.combineList();

        List<?> compList = app.getUIApplicationSession().getCatiaSession().getComponents();

        for (Object catiaComp : compList) {
            AbstractUIManagerComponent comp = (AbstractUIManagerComponent) catiaComp;
            String[] props = new String[]{DATASET_UID, ITEM_ID, CO_STATUS};
            HashMap<String, String> properties = this.getRequiredProperties(comp, props);

            if (coRequired(properties, combinedList) && checkCOStatus(properties.get(CO_STATUS), properties.get(ITEM_ID))) {
                checkOutNeeded.put(comp, properties.get(ITEM_ID));
            }
        }
        return checkOutNeeded;
    }

    private List<String> combineList() {
        List<String> modifiedAssembliesUID = XMLReader.getModifiedAssembliesUID();
        List<String> modifiedAssembliesID = XMLReader.getModifiedAssembliesID();

        List<String> listCombined = new ArrayList<>();
        listCombined.addAll(modifiedAssembliesUID);
        listCombined.addAll(modifiedAssembliesID);

        return listCombined;
    }

    private STATUS studyModeHandler() {
        return STATUS.PASS;
    }

}
