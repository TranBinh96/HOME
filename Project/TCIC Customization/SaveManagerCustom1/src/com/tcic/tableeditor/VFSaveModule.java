package com.tcic.tableeditor;


import com.ebsolutions.uiapplication.UIApplicationSession;
import com.ebsolutions.uimanager.modules.SaveModule;
import com.tcic.utils.RollBackHandler;
import com.tcic.utils.XMLReader;

public class VFSaveModule extends SaveModule {

	@Override
	protected VFTableEditor createDefaultTableEditor() {
		XMLReader.readXML();
		try {
			RollBackHandler.loadRollbackOption();
		} catch (IllegalAccessException e) {
			System.out.println(e.getMessage());
			throw new RuntimeException(e);
		}

		VFTableEditor editor = new VFTableEditor();

		UIApplicationSession session =  getApplication().getUIApplicationSession();
		
		editor.setSession(session);
		
		return editor;
	}
}
