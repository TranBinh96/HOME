package com.tcic.actions;

import java.awt.event.ActionEvent;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

import com.ebsolutions.soacore.sessionmanager.SOASession;
import com.ebsolutions.uiapplication.InterfaceCATIAComponent;
import com.ebsolutions.uiapplication.actions.AbstractAction;
import com.ebsolutions.uimanager.AbstractUIManagerApplication;
import com.ebsolutions.uimanager.AbstractUIManagerComponent;
import com.ebsolutions.uimanager.AbstractUIManagerSession;
import com.ebsolutions.uimanager.ItemTypeListProperty;
import com.ebsolutions.uimanager.Property;
import com.ebsolutions.uimanager.TCTypeInfo;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.ebsolutions.uimanager.modules.TableEditor;
import com.ebsolutions.uimanager.modules.UIDescriptor;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.query.SavedQueryService;
import com.teamcenter.services.strong.query._2007_06.SavedQuery.ExecuteSavedQueriesResponse;
import com.teamcenter.services.strong.query._2007_06.SavedQuery.SavedQueryInput;
import com.teamcenter.services.strong.query._2007_06.SavedQuery.SavedQueryResults;
import com.teamcenter.services.strong.query._2010_04.SavedQuery.FindSavedQueriesCriteriaInput;
import com.teamcenter.services.strong.query._2010_04.SavedQuery.FindSavedQueriesResponse;
import com.teamcenter.soa.client.Connection;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.ImanQuery;
import com.teamcenter.soa.common.ObjectPropertyPolicy;
import com.teamcenter.soa.common.PolicyProperty;
import com.teamcenter.soa.exceptions.NotLoadedException;

public abstract class AbstractCustomAction extends AbstractAction implements ComponentAttribute {
    /**
     * @author bangpq - VinFast Custom
     */

    protected AbstractUIManagerApplication app = null;
    protected SaveModule module = null;
    protected AbstractUIManagerSession session = null;

    @Override
    public void actionPerformed(ActionEvent e) {
        app = (AbstractUIManagerApplication) getApplication();
        try {
            module = (SaveModule) app.getCurrentModule();
        } catch (ClassCastException cce) {
            module = (SaveModule) app.getModule(SaveModule.DEFAULT_ID);
        }

        if (module == null) {
            return;
        }

//		Get selected cad
        session = (AbstractUIManagerSession) app.getUIApplicationSession().getCatiaSession();
        session.save();
    }

    protected String typeFromID(AbstractUIManagerComponent component) {

        Property itemIDProp = (Property) component.getComponentProperty(ITEM_ID);
        String itemIDVal = itemIDProp.getValue().toString();

        if (!component.getComponentType().toString().equals(CAT_PART)) {
            if (itemIDVal.matches(VALID_MP_DESIGN_ID_FORMAT_1) || itemIDVal.matches(VALID_MP_DESIGN_ID_FORMAT_2))
                return MP_DESIGN;
            else
                return CAT_PRODUCT;
        }

        if (itemIDVal.matches(VALID_JF_ID_FORMAT_1) || itemIDVal.matches(VALID_JF_ID_FORMAT_2))
            return JF_PART;
        else if (itemIDVal.matches(VALID_MP_DESIGN_ID_FORMAT_1) || itemIDVal.matches(VALID_MP_DESIGN_ID_FORMAT_2))
            return MP_DESIGN;
        else
            return null;
    }

    protected String typeFromID(String itemIDVal) {

        if (itemIDVal.matches(VALID_JF_ID_FORMAT_1) || itemIDVal.matches(VALID_JF_ID_FORMAT_2))
            return JF_PART;
        else if (itemIDVal.matches(VALID_MP_DESIGN_ID_FORMAT_1) || itemIDVal.matches(VALID_MP_DESIGN_ID_FORMAT_2))
            return MP_DESIGN;
        else
            return null;
    }

    protected void uneditableProps(AbstractUIManagerComponent component, String[] uneditableProps) {

        Thread appThread = new Thread() {
            public void run() {
                try {
                    SwingUtilities.invokeAndWait(new Runnable() {
                        @Override
                        public void run() {
                            Property[] compProps = component.getComponentProperties(uneditableProps);

                            for (Property property : compProps) {
                                if (property == null)
                                    continue;

                                if (property.isEditable() && isFilled(component, property)) {
                                    property.setEditable(false);
                                }
                            }
                        }
                    });
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        };
        appThread.start();

    }

    protected void setType(AbstractUIManagerComponent component) {

        /* Get type list and select wanted type */
        ItemTypeListProperty itemTypeProp = (ItemTypeListProperty) component.getComponentProperty(ITEM_TYPE);
        TCTypeInfo[] typeLOV = (TCTypeInfo[]) itemTypeProp.getPropertiesList();

        for (int index = 0; index < typeLOV.length; index++) {

            if (typeLOV[index].get_real_name().equals(typeFromID(component))) {

                component.getComponentProperty(ITEM_TYPE).setEditable(true);
                component.setComponentProperty(ITEM_TYPE, index);

                break;

            }
        }

        session.checkWithResetProps(component);
    }

    protected boolean isFilled(AbstractUIManagerComponent component, Property properties) {

        String propVal = properties.getValue().toString();

        if (propVal == null || propVal.equals(""))
            return false;
        else
            return true;
    }

    protected boolean matchType(AbstractUIManagerComponent component, String type) {

        Object[] typeLOV = ((ItemTypeListProperty) component.getComponentProperty(ITEM_TYPE)).getPropertiesList();

        int typeIndex = (int) ((AbstractUIManagerComponent) component).getComponentProperty(ITEM_TYPE).getValue();
        String currType = ((TCTypeInfo) typeLOV[typeIndex]).get_real_name();
        String currType1 = ((TCTypeInfo) typeLOV[typeIndex]).get_display_name();

        if (currType.equals(type))
            return true;
        else
            return false;
    }

    protected void warning(AbstractUIManagerApplication app, List<InterfaceCATIAComponent> invalidComps) {

        String list = DASH_SPACE;

        for (int i = 0; i < invalidComps.size(); i++) {

            AbstractUIManagerComponent component = (AbstractUIManagerComponent) invalidComps.get(i);
            String idVal = component.getComponentProperty(ITEM_ID).getValue().toString();

            if (i == invalidComps.size() - 1)
                list = list + idVal + "\n";
            else
                list = list + idVal + "\n" + DASH_SPACE;

        }

        JOptionPane.showMessageDialog(app, "Invalid ID to Create:\n" + list, "Warning", JOptionPane.WARNING_MESSAGE,
                null);

    }

    protected void warning(AbstractUIManagerApplication app, List<InterfaceCATIAComponent> validComps,
                           List<InterfaceCATIAComponent> invalidComps) {

        String invalidCompsList = DASH_SPACE;
        String validCompsList = DASH_SPACE;

        for (int i = 0; i < invalidComps.size(); i++) {

            AbstractUIManagerComponent component = (AbstractUIManagerComponent) invalidComps.get(i);
            String idVal = component.getComponentProperty(ITEM_ID).getValue().toString();

            if (i == invalidComps.size() - 1)
                invalidCompsList = invalidCompsList + idVal + "\n";
            else
                invalidCompsList = invalidCompsList + idVal + "\n" + DASH_SPACE;

        }

        for (int i = 0; i < validComps.size(); i++) {

            AbstractUIManagerComponent component = (AbstractUIManagerComponent) validComps.get(i);
            String idVal = component.getComponentProperty(ITEM_ID).getValue().toString();

            if (i == validComps.size() - 1)
                validCompsList = validCompsList + idVal + "\n";
            else
                validCompsList = validCompsList + idVal + "\n" + DASH_SPACE;
        }

        JOptionPane.showMessageDialog(app,

                "Invalid ID to Create:\n" + invalidCompsList + "\n" + "Successfully applied configuration for:\n"
                        + validCompsList + "\n",

                "Process Information", JOptionPane.INFORMATION_MESSAGE, null);
    }

    protected void getInputID() {
        TableEditor tableEditor = (TableEditor) ((SaveModule) (getApplication().getCurrentModule())).getTableEditor();
        UIDescriptor uiDes = tableEditor.getDescriptor();

//        Object idText = tableEditor.getKeyList().get("item_id");

    }

    protected ModelObject[] quObject(LinkedHashMap<String, String> inputQuery, String queryName)
            throws ServiceException {

        ModelObject[] rs = null;

        SOASession soaInstance = SOASession.get_instance();
        Connection soaConnection = soaInstance.getConnection();
        SavedQueryService savedQuery = SavedQueryService.getService(soaConnection);
        FindSavedQueriesCriteriaInput qry[] = new FindSavedQueriesCriteriaInput[1];
        FindSavedQueriesCriteriaInput qurey = new FindSavedQueriesCriteriaInput();
        String name[] = {queryName};
        String desc[] = {""};

        ObjectPropertyPolicy policy = new ObjectPropertyPolicy();
        policy.addType("VF4_Design", new String[]{"item_id", "vf4_donor_vehicle"});
        policy.addType("Design", new String[]{"item_id", "vf4_donor_vehicle"});
        policy.addType("Item", new String[]{"item_id", "vf4_donor_vehicle"});

        policy.setModifier(PolicyProperty.WITH_PROPERTIES, true);
        policy.setModifier(PolicyProperty.AS_ATTRIBUTE, true);
        policy.setModifier(PolicyProperty.INCLUDE_MODIFIABLE, true);

        qurey.queryNames = name;
        qurey.queryDescs = desc;
        qurey.queryType = 0;

        qry[0] = qurey;

//		SavedQueryRestBindingStub.getService(soaConnection);
        FindSavedQueriesResponse responce1 = savedQuery.findSavedQueries(qry);
        ImanQuery[] result = responce1.savedQueries;

        SavedQueryInput qc = new SavedQueryInput();
        SavedQueryInput qc_v[] = new SavedQueryInput[1];

        qc.query = result[0];
        qc.entries = new String[inputQuery.size()];
        qc.values = new String[inputQuery.size()];
        int i = 0;

        for (Map.Entry<String, String> entry : inputQuery.entrySet()) {
            qc.entries[i] = entry.getKey();
            qc.values[i] = entry.getValue();
            i++;
        }
        qc_v[0] = qc;

        ExecuteSavedQueriesResponse responce = savedQuery.executeSavedQueries(qc_v);

        SavedQueryResults[] results = responce.arrayOfResults;

        if (results[0].numOfObjects != 0) {
//			objects = results[0];
            rs = results[0].objects;
            System.out.println("[INFORMATION] results[0].numOfObjects OBJECT FOUND");

        } else {
            System.out.println("[WARNING] NO OBJECT FOUND");
        }
        return rs;
    }

    protected void initRev(AbstractUIManagerComponent component) {
        component.getComponentProperty(ITEM_REV_ID).setEditable(true);
        component.setProperty(ITEM_REV_ID, "01");
        component.getComponentProperty(ITEM_REV_ID).setEditable(false);
    }

    protected void removeRev(AbstractUIManagerComponent component) {
        component.getComponentProperty(ITEM_REV_ID).setEditable(true);
        component.setProperty(ITEM_REV_ID, "");
        component.getComponentProperty(ITEM_REV_ID).setEditable(false);
    }

    protected void initDonor(AbstractUIManagerComponent component, String parentDonor) {
        component.getComponentProperty(DONOR_VEHICLE).setEditable(true);
        component.setProperty(DONOR_VEHICLE, parentDonor);
        component.getComponentProperty(ITEM_REV_ID).setEditable(false);
    }

    protected ModelObject[] savedQuery(String inputID, String type) {
        LinkedHashMap<String, String> inputQuery = new LinkedHashMap<String, String>();

        inputQuery.put("Item ID", inputID);
        inputQuery.put("Type", type);
        String qryName = "Item...";
        ModelObject[] qryRs = null;

        try {
            qryRs = quObject(inputQuery, qryName);
        } catch (ServiceException e1) {
            System.out.println(e1.getMessage().toString());
        }

        return qryRs;
    }

    protected boolean isInputExist(String inputID, String type) {
        boolean rs = false;
        ModelObject[] qryRs = null;

        qryRs = savedQuery(inputID, type);

        if (qryRs != null)
            rs = true;

        return rs;
    }

    protected String getParentDonor(String inputID) {
        String parentDonor = "";
        String[] inputSplitted = inputID.split("_");
        String parentID = inputSplitted[0];
        SOASession soaInstance = SOASession.get_instance();
        Connection soaConnection = soaInstance.getConnection();

        ModelObject[] qryRs = savedQuery(parentID, "VF Design");

        if (qryRs == null)
            return parentDonor;

        ModelObject parent = qryRs[0];
        try {
            DataManagementService dmService = DataManagementService.getService(soaConnection);
            ServiceData sd = dmService.getProperties(qryRs, new String[]{"vf4_donor_vehicle"});
            parentDonor = parent.getPropertyDisplayableValue("vf4_donor_vehicle");
        } catch (NotLoadedException e) {
            System.out.println(e.getMessage().toString());
        }

        return parentDonor;
    }
}
