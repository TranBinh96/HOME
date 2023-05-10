package com.tcic.utils;

import com.tcic.actions.AssemblyInfo;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import java.util.ArrayList;
import java.util.List;

public class XMLHandler extends DefaultHandler {
    private static final String UID = "UID";
    private static final String ID = "ID";
    private static final String PART_MAKE_BUY = "PartMakeBuy";
    private static final String ASSEMBLIES_OCC_MODIFIED = "AssembliesOccModified";
    private static final String ASSEMBLIES_UPDATE_PART_MAKE_BUY = "AssembliesUpdatePartMakeBuy";
    private String content;
    private AssemblyInfo assemblyInfo;
    private List<AssemblyInfo> assembliesOccModified = new ArrayList<>();
    private List<AssemblyInfo> assembliesUpdatePartMakeBuy = new ArrayList<>();;
    private List<AssemblyInfo> tempList;

    public List<AssemblyInfo> getAssembliesUpdatePartMakeBuy() {
        return assembliesUpdatePartMakeBuy;
    }

    public List<AssemblyInfo> getAssembliesOccModified() {
        return assembliesOccModified;
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        switch (qName) {
            case ASSEMBLIES_OCC_MODIFIED:
                if (assembliesOccModified == null)
                    assembliesOccModified = new ArrayList<>();
                break;

            case AssemblyInfo.NAME:
                assemblyInfo = new AssemblyInfo();
                if (tempList == null)
                    tempList = new ArrayList<>();
                break;

            case ASSEMBLIES_UPDATE_PART_MAKE_BUY:
                if (assembliesUpdatePartMakeBuy == null)
                    assembliesUpdatePartMakeBuy = new ArrayList<>();
                break;
        }
    }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        switch (qName) {
            case AssemblyInfo.NAME:
                tempList.add(assemblyInfo);
                break;
            case UID:
                assemblyInfo.setUid(content);
                break;
            case ID:
                assemblyInfo.setId(content);
                break;
            case PART_MAKE_BUY:
                assemblyInfo.setPartMakeBuy(content);
                break;
            case ASSEMBLIES_OCC_MODIFIED:
                assembliesOccModified.addAll(tempList);
                tempList.clear();
                break;
            case ASSEMBLIES_UPDATE_PART_MAKE_BUY:
                assembliesUpdatePartMakeBuy.addAll(tempList);
                tempList.clear();
                break;
        }
    }

    @Override
    public void characters(char[] ch, int start, int length) throws SAXException {
        content = String.copyValueOf(ch, start, length).trim();
    }
}
