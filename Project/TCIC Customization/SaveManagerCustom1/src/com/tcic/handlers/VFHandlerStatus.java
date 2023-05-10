package com.tcic.handlers;

public interface VFHandlerStatus {
    enum STATUS {
        PASS,
        STUDY_FAIL,
        DESIGN_FAIL,
        NOT_REQUIRE,
        NULL_POINTER
    }

    enum STUDY_MODE_ERROR {
        ERROR_1,
        ERROR_2,
        ERROR_3
    }

    enum DESIGN_MODE_ERROR {
        ERROR_1,
        ERROR_2,
        ERROR_3
    }
}
