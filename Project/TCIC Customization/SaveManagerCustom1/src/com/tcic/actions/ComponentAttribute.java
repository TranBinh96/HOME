package com.tcic.actions;

import com.ebsolutions.uimanager.AbstractUIManagerComponent;

public interface ComponentAttribute {
    String STUDY_PART_REAL_NAME = "VF4_Study_Part";
    String STUDY_MODE = "STUDY";
    String DESIGN_MODE = "DESIGN";
    String DONOR_VEHICLE = "TC Item vf4_donor_vehicle";
    String IMDS_OBJECT = "TC Item sci0IsIMDSObject";
    String PART_CATEGORY = "TC Item vf4_part_category";
    String PART_MAKE_BUY = "TC Item vf4_item_make_buy";
    String ITEM_ID = AbstractUIManagerComponent.ITEM_ID;
    String ITEM_TYPE = AbstractUIManagerComponent.ITEM_TYPE;
    String ITEM_NAME = AbstractUIManagerComponent.ITEM_NAME;
    String ITEM_REV_ID = AbstractUIManagerComponent.ITEM_REV_ID;
    String DATASET_UID = AbstractUIManagerComponent.DATASET_UID;
    String CO_STATUS = AbstractUIManagerComponent.CO_STATUS;
    String JF_PART = "VF4_JF_Part";
    String MP_DESIGN = "VF4_MP_Design";
    String VF_DESIGN = "VF4_Design";
    String CAT_PART = "catpart";
    String CAT_PRODUCT = "catproduct";
    String JF_PART_DN = "JF Part";
    String MP_DESIGN_DN = "VF MP Design";
    String VF_DESIGN_DN = "VF Design";
    String VALID_JF_ID_FORMAT_1 = "(^[A-Z]{3}[0-9]{8}[A-Z]{2}(_JF){1}$)";
    String VALID_JF_ID_FORMAT_2 = "(^[A-Z]{3}[0-9]{8}(_JF){1}$)";
    String VALID_MP_DESIGN_ID_FORMAT_1 = "(^[A-Z]{3}[0-9]{8}[A-Z]{2}(_MP){1}$)";
    String VALID_MP_DESIGN_ID_FORMAT_2 = "(^[A-Z]{3}[0-9]{8}(_MP){1}$)";
    String DASH_SPACE = "- ";

    enum STATUS {
        PASS,
        STUDY_FAIL,
        DESIGN_FAIL,
        NOT_REQUIRE,
        NULL_POINTER
    }


}
