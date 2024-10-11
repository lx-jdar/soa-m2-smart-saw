package com.example.smartsaw;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.TextView;

public class ButtonWood extends FrameLayout {

    //#region Attributes

    private TextView text;
    private ImageButton image;

    //#endregion

    //#region Constructor

    public ButtonWood(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    //#endregion

    //#region Public Methods

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        image.setEnabled(enabled);
        text.setEnabled(enabled);
    }

    public void setButtonText(String text) {
        this.text.setText(text);
    }

    public void setButtonOnClickListener(OnClickListener listener) {
        image.setOnClickListener(listener);
    }

    //#endregion

    //#region Private Methods

    private void init(Context context) {
        LayoutInflater.from(context).inflate(R.layout.button_wood, this, true);
        text = findViewById(R.id.txt_btn_wood);
        image = findViewById(R.id.img_btn_wood);
    }

    //#endregion

}
