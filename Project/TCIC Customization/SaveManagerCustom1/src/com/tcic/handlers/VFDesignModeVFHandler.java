package com.tcic.handlers;

import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uimanager.AbstractUIManagerApplication;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.tcic.actions.AssemblyInfo;
import com.tcic.utils.XMLReader;

import java.util.Collection;
import java.util.HashMap;
import java.util.List;

import static com.ebsolutions.uimanager.AbstractUIManagerComponent.ITEM_ID;
import static com.tcic.actions.ComponentAttribute.DESIGN_MODE;
import static com.tcic.actions.ComponentAttribute.PART_MAKE_BUY;

public class VFDesignModeVFHandler implements VFHandlerStatus {
    public AbstractUIManagerApplication app;
    private static VFDesignModeVFHandler instance;
    private List<DESIGN_MODE_ERROR> errorList;

    public void setErrorList(List<DESIGN_MODE_ERROR> errorList) {
        this.errorList = errorList;
    }

    public static VFDesignModeVFHandler getInstance(AbstractUIManagerApplication app) {
        return instance == null? new VFDesignModeVFHandler(app) : instance;
    }

    public VFDesignModeVFHandler(AbstractUIManagerApplication app) {
        this.app = app;
    }

    public boolean actionPerformed(String saveMode, InterfaceCATIAComponent[] selComponents) {
        return saveMode.equalsIgnoreCase(DESIGN_MODE) ? designModeHandler(selComponents) : false;
    }

    private boolean designModeHandler(InterfaceCATIAComponent[] selComponents) {
        XMLReader.readXML();
        HashMap<String, AssemblyInfo> partMakeBuyList = XMLReader.getPartMakeBuyList();
        Collection<String> idList = partMakeBuyList.keySet();

        for (InterfaceCATIAComponent comp : selComponents) {
            AbstractUIManagerComponent component = (AbstractUIManagerComponent) comp;
            String itemID = component.getProperty(ITEM_ID).toString();

            if (idList.contains(itemID)) {
                String makeBuyVal = partMakeBuyList.get(itemID).getPartMakeBuy();
                component.setComponentProperty(PART_MAKE_BUY, makeBuyVal);
                System.out.println(itemID + "=> " + makeBuyVal + " applied");
            }
        }
        return true;
    }
}
