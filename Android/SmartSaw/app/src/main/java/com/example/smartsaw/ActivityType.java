package com.example.smartsaw;

public enum ActivityType {
    MAIN_ACTIVITY("MAIN_ACTIVITY"),
    OPTIONS_ACTIVITY("OPTIONS_ACTIVITY"),
    CONFIGURATION_ACTIVITY("CONFIGURATION_ACTIVITY"),
    PRECISION_ACTIVITY("PRECISION_ACTIVITY"),
    MOVEMENT_ACTIVITY("MOVEMENT_ACTIVITY"),
    ;

    private final String activityValue;
    ActivityType(String activityValue) {
        this.activityValue=activityValue;
    }

    public String getValue() {
        return activityValue;
    }
}
