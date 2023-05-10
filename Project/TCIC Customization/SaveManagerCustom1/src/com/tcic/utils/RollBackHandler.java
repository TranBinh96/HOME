package com.tcic.utils;

import com.ebsolutions.uimanager.common.UIPreferencesManager;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class RollBackHandler {
    private static boolean ROLLBACK_ALL = true;
    private static boolean ROLLBACK_APPLY_BUTTON_ACTION = true;
    private static boolean ROLLBACK_CO_ACTION = true;
    private static boolean ROLLBACK_SAVE_BUTTON_ACTION = true;
    private static boolean ROLLBACK_ALL_POST_ACTION= true;
    private static boolean ROLLBACK_CREATE_NEW_POST_ACTION = true;
    private static boolean ROLLBACK_APPLY_BUTTON_POST_ACTION = true;
    private static boolean ROLLBACK_REVISION_HANDLER = true;

    private static final String ROLLBACK_PREFERENCE = "CATIA_vf_tcic_rollback_option";

    public RollBackHandler(){}
    public static void loadRollbackOption() throws IllegalAccessException {
        String[] tcicRollbackOption = getRollbackOption();
        if (tcicRollbackOption.length != 0){
            List<String> optionList = new ArrayList<>(Arrays.asList(tcicRollbackOption));
            Field[] fields = RollBackHandler.class.getFields();
            for (Field field : fields){
                field.setBoolean(RollBackHandler.class,optionList.contains(field.getName()));
            }
        }
    }

    private static String[] getRollbackOption() {
        try {
            return UIPreferencesManager.getPreferenceValues(ROLLBACK_PREFERENCE);
        } catch (Exception e) {
            System.out.println("VF-ERROR: Fail to get pref: " + ROLLBACK_PREFERENCE + " => Set Rollback All");
            return new String[]{"ROLLBACK_ALL"};
        }
    }
    public static boolean isRollbackAll() {
        return ROLLBACK_ALL;
    }

    public static boolean isRollbackApplyButtonAction() {
        return ROLLBACK_APPLY_BUTTON_ACTION;
    }

    public static boolean isRollbackSaveButtonAction() {
        return ROLLBACK_SAVE_BUTTON_ACTION;
    }

    public static boolean isRollbackCoAction() {
        return ROLLBACK_CO_ACTION;
    }

    public static boolean isRollbackAllPostAction() {
        return ROLLBACK_ALL_POST_ACTION;
    }

    public static boolean isRollbackCreateNewPostAction() {
        return ROLLBACK_CREATE_NEW_POST_ACTION;
    }

    public static boolean isRollbackApplyButtonPostAction() {
        return ROLLBACK_APPLY_BUTTON_POST_ACTION;
    }

    public static boolean isRollbackRevisionHandler() {
        return ROLLBACK_REVISION_HANDLER;
    }
}
