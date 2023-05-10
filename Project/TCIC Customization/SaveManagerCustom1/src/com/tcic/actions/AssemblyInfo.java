package com.tcic.actions;

public class AssemblyInfo {
	private String uid;
	private String id;
	private String partMakeBuy;

    public String getPartMakeBuy() {
        return partMakeBuy;
    }

    public void setPartMakeBuy(String partMakeBuy) {
        this.partMakeBuy = partMakeBuy;
    }

    public static final String NAME = "AssemblyInfo";

	public String getUid() {
		return uid;
	}

	public void setUid(String uID) {
		uid = uID;
	}

	public String getId() {
		return id;
	}

	public void setId(String iD) {
		id = iD;
	}


	@Override
	public String toString() {
		return "Assembly, UID= " + uid + ", ID= " + id;
	}
}