package com.example.smartsaw;

public enum EmbeddedCode {
    SOFF("SIERRA_OFF"),
    SON("SIERRA_ON"),
    S("SEND_START_SAW"),
    T("SEND_STOP_SAW"),
    I("SEND_LEFT_MOVEMENT"),
    D("SEND_RIGHT_MOVEMENT"),
    ME_ON("MOTION_ENGINE_ON"),
    ME_OFF("MOTION_ENGINE_OFF");

    private final String value;
    EmbeddedCode(String value) {
        this.value=value;
    }

    public String getValue() {
        return value;
    }
}
