package com.tcic.utils;

import com.tcic.actions.AssemblyInfo;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

public class XMLReader {
    private static final String TEMP_FILE = System.getenv("WORK_DIR") + "\\vf_temp\\tcic_interface.xml";
    private static List<AssemblyInfo> assembliesOccModified;
    private static List<AssemblyInfo> assembliesUpdatePartMakeBuy;
    public static void readXML() {
        try {
            File inputFile = new File(TEMP_FILE);
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            XMLHandler xmlHandler = new XMLHandler();
            // read XML
            saxParser.parse(inputFile, xmlHandler);
            // print object
            assembliesOccModified = xmlHandler.getAssembliesOccModified();
            assembliesUpdatePartMakeBuy = xmlHandler.getAssembliesUpdatePartMakeBuy();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static List<AssemblyInfo> getModifiedAssembliesList() {
        return assembliesOccModified;
    }

    public static List<String> getModifiedAssembliesUID() {
        List<String> modifiedAssembliesUID = new ArrayList<>();

        for (AssemblyInfo assemblyInfo : assembliesOccModified) {
            if (assemblyInfo != null)
                modifiedAssembliesUID.add(assemblyInfo.getUid());
        }

        return modifiedAssembliesUID;
    }

    public static List<String> getModifiedAssembliesID() {
        List<String> modifiedAssembliesID = new ArrayList<>();

        for (AssemblyInfo assemblyInfo : assembliesOccModified) {
            if (assemblyInfo != null)
                modifiedAssembliesID.add(assemblyInfo.getId());
        }

        return modifiedAssembliesID;
    }

    public static HashMap<String, AssemblyInfo> getPartMakeBuyList() {
        HashMap<String, AssemblyInfo> partMakeBuyList = new HashMap<>();

        for (AssemblyInfo assemblyInfo : assembliesUpdatePartMakeBuy) {
            if (assemblyInfo != null && assemblyInfo.getPartMakeBuy() != null)
                partMakeBuyList.put(assemblyInfo.getId(), assemblyInfo);
            else if (assemblyInfo != null && assemblyInfo.getPartMakeBuy() == null)
                System.out.println(assemblyInfo.getId() + " missing Part Make/Buy");
        }

        return partMakeBuyList;
    }

    public static String getSaveMode() {
        String saveMode = "";

        try (FileReader fr = new FileReader(TEMP_FILE);
             BufferedReader br = new BufferedReader(fr);) {
            String line;

            while ((line = br.readLine()) != null) {
                if (line.contains("TCICSaveMode")) {
                    saveMode = line.substring(line.indexOf(">") + 1, line.lastIndexOf("<"));
                    break;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return saveMode;
    }
}
