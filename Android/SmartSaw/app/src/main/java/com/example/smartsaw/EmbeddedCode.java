package com.example.smartsaw;

public enum EmbeddedCode {
    SOFF("SIERRA_OFF"),
    SON("SIERRA_ON"),

    SUS("THRESHOLD_OVER_LIMIT"),
    S("SEND_START_SAW"),
    T("SEND_STOP_SAW"),
    I("SEND_LEFT_MOVEMENT"),
    D("SEND_RIGHT_MOVEMENT"),
    ME_ON("MOTION_ENGINE_ON"),
    ME_OFF("MOTION_ENGINE_OFF"),
    PNOK("PRECISION_NUMBER_OK"),
    OK("BLUETOOTH_CONNECTED");

    private final String value;

    EmbeddedCode(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }
}
