package com.tcic.utils;

import com.ebsolutions.soacore.sessionmanager.SOASession;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.query.SavedQueryService;
import com.teamcenter.services.strong.query._2010_04.SavedQuery;
import com.teamcenter.soa.client.Connection;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.strong.ImanQuery;

import java.util.*;

public class Query {
    private static String qryName = "Item...";

    public static String getQryName() {
        return qryName;
    }

    private static ModelObject[] quObject(LinkedHashMap<String, String> inputQuery, String queryName)
            throws ServiceException {
        ModelObject[] rs = null;

        SOASession soaInstance = SOASession.get_instance();
        Connection soaConnection = soaInstance.getConnection();
        SavedQueryService savedQuery = SavedQueryService.getService(soaConnection);
        SavedQuery.FindSavedQueriesCriteriaInput[] qry = new SavedQuery.FindSavedQueriesCriteriaInput[1];
        SavedQuery.FindSavedQueriesCriteriaInput qurey = new SavedQuery.FindSavedQueriesCriteriaInput();
        String[] name = {queryName};
        String[] desc = {"*"};

        qurey.queryNames = name;
        qurey.queryDescs = desc;
        qurey.queryType = 0;

        qry[0] = qurey;

        SavedQuery.FindSavedQueriesResponse responce1 = savedQuery.findSavedQueries(qry);
        ImanQuery[] result = responce1.savedQueries;

        com.teamcenter.services.strong.query._2007_06.SavedQuery.SavedQueryInput qc = new com.teamcenter.services.strong.query._2007_06.SavedQuery.SavedQueryInput();
        com.teamcenter.services.strong.query._2007_06.SavedQuery.SavedQueryInput[] qcArr = new com.teamcenter.services.strong.query._2007_06.SavedQuery.SavedQueryInput[1];

        qc.query = result[0];
        qc.entries = new String[inputQuery.size()];
        qc.values = new String[inputQuery.size()];
        int i = 0;

        for (Map.Entry<String, String> entry : inputQuery.entrySet()) {
            qc.entries[i] = entry.getKey();
            qc.values[i] = entry.getValue();
            i++;
        }
        qcArr[0] = qc;

        com.teamcenter.services.strong.query._2007_06.SavedQuery.ExecuteSavedQueriesResponse response
                = savedQuery.executeSavedQueries(qcArr);

        com.teamcenter.services.strong.query._2007_06.SavedQuery.SavedQueryResults[] results
                = response.arrayOfResults;

        if (results[0].numOfObjects != 0) {
            rs = results[0].objects;
            System.out.println("[INFORMATION]" + results[0].numOfObjects + " OBJECT FOUND");
        } else {
            rs = new ModelObject[0];
            System.out.println("[WARNING] NO OBJECT FOUND");
        }

        return rs;
    }

    public static Criteria savedQuery(String savedQueryName) {
        qryName = savedQueryName;
        Criteria.criteriaList.clear();

        return new Criteria();
    }

    public static ModelObject[] query() {
        ModelObject[] qryRs = null;
        LinkedHashMap<String, String> inputQuery = new LinkedHashMap<>();
        for (Criteria criteria : Criteria.criteriaList) {
            inputQuery.put(criteria.getCriteria(), criteria.getValue());
        }
        try {
            qryRs = quObject(inputQuery, qryName);
        } catch (Exception e1) {
            System.out.println(e1.getMessage().toString());
        }

        return qryRs;
    }


    public static class Criteria {
        private String criteria;
        private String value;

        public String getCriteria() {
            return criteria;
        }

        public String getValue() {
            return value;
        }

        static LinkedList<Criteria> criteriaList = new LinkedList<>();

        public Criteria() {
        }

        public Criteria(String criteria, String value) {
            this.criteria = criteria;
            this.value = value;
        }

        public Criteria addCriteria(String criteria, String value) {
            criteriaList.add(new Criteria(criteria, value));
            return this;
        }

        public ModelObject[] execute() {
            return query();
        }
    }

    public static ModelObject[] execute(Map<String, String> criteriaMap, String savedQuery) {
        Criteria query = Query.savedQuery(savedQuery);
        for (Map.Entry<String, String> entry : criteriaMap.entrySet()) {
            query.addCriteria(entry.getKey(), entry.getValue());
        }
        return query.execute();
    }

    public static boolean isExisting(String curID, String curType) {
        ModelObject[] rs = Query.savedQuery(qryName)
                .addCriteria("Item ID", curID)
                .addCriteria("Type", curType)
                .execute();
        return !(rs == null || rs.length == 0);
    }

    public static int latestRev(String id, String type) {
        HashMap<String, String> criteriaMap = new HashMap<>();
        criteriaMap.put("Item ID", id);
        criteriaMap.put("Type", type);

        ModelObject[] qryRs = execute(criteriaMap, "Latest Item Revision...");
//        ModelObject[] qryRs = execute(criteriaMap, qryName);
        int latestRevision = 0;
        try {
            DataManagementService dmService = DataManagementService.getService(SOASession.get_instance().getConnection());
            dmService.getProperties(qryRs, new String[]{"item_revision_id"});
            String revID = qryRs[0].getPropertyDisplayableValue("item_revision_id");
            latestRevision = qryRs.length != 1 ? 0 : Integer.parseInt(revID);
        } catch (Exception e) {
            System.out.println(e.getMessage().toString());
        }
        return latestRevision;
    }

    public static List<String> getRevList(String id, String type) {
        type = type + " Revision";
        List<String> revList = new ArrayList<>();
        HashMap<String, String> criteriaMap = new HashMap<>();
        criteriaMap.put("Item ID", id);
        criteriaMap.put("Type", type);

        ModelObject[] qryRs = execute(criteriaMap, "Item Revision...");
        try {
            DataManagementService dmService = DataManagementService.getService(SOASession.get_instance().getConnection());
            dmService.getProperties(qryRs, new String[]{"item_revision_id"});
            for (ModelObject model : qryRs) {
                revList.add(model.getPropertyDisplayableValue("item_revision_id"));
            }
        } catch (Exception e) {
            System.out.println(e.getMessage().toString());
        }
        return revList;
    }
}
