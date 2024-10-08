package com.example.aplicaciontest;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.TextView;

public class BotonSmartSaw extends FrameLayout {

    private TextView texto;

    private ImageButton imagen;

    public BotonSmartSaw(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    private void init(Context context) {
        LayoutInflater.from(context).inflate(R.layout.button_smart_saw, this, true);
        texto = findViewById(R.id.txt_btn_smart_saw);
        imagen = findViewById(R.id.img_btn_smart_saw);
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        // Asegurarse de deshabilitar también el botón interno
        imagen.setEnabled(enabled);
        texto.setEnabled(enabled);
    }

    public void setButtonText(String text) {
        texto.setText(text);
    }

    public void setButtonOnClickListener(OnClickListener listener) {
        imagen.setOnClickListener(listener);
    }

}
