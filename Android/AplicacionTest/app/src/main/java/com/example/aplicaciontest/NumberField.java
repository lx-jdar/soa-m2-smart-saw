package com.example.aplicaciontest;

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.widget.EditText;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

public class NumberField extends ConstraintLayout {

    private TextView labelField;

    private EditText numberField;

    public NumberField(Context context) {
        super(context);
        init(context, null);
    }

    public NumberField(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context, attrs);
    }

    public NumberField(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context, attrs);
    }

    private void init(Context context, AttributeSet attrs) {
        LayoutInflater.from(context).inflate(R.layout.number_field, this, true);

        labelField = findViewById(R.id.title_number_field);
        numberField = findViewById(R.id.value_number_field);

        if (attrs != null) {
            try (TypedArray typedArray = context.obtainStyledAttributes(attrs, R.styleable.NumberField)) {
                String labelText = typedArray.getString(R.styleable.NumberField_labelText);
                setLabelText(labelText);
                typedArray.recycle();
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void setLabelText(String text) {
        labelField.setText(text);
    }

    public String getNumberFieldText() {
        return numberField.getText().toString();
    }

    public void setNumberFieldText(String text) {
        numberField.setText(text);
    }

}
