package com.example.smartsaw;

import android.content.Context;
import android.content.res.TypedArray;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.widget.EditText;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

public class NumberField extends ConstraintLayout
{

  //#region Attributes

  private TextView labelField;
  private EditText numberField;

  //#endregion

  //#region Constructors

  public NumberField(Context context)
  {
    super(context);
    init(context, null);
  }

  public NumberField(Context context, AttributeSet attrs)
  {
    super(context, attrs);
    init(context, attrs);
  }

  public NumberField(Context context, AttributeSet attrs, int defStyleAttr)
  {
    super(context, attrs, defStyleAttr);
    init(context, attrs);
  }

  //#endregion

  //#region Public Methods

  public void setLabelText(String text)
  {
    labelField.setText(text);
  }

  @SuppressWarnings("unused")
  public void setNumberFieldText(String text)
  {
    numberField.setText(text);
  }

  public Integer getValue()
  {
    String text = getNumberFieldText();
    if (TextUtils.isEmpty(text))
    {
      return null;
    }
    try
    {
      return Integer.parseInt(text);
    } catch (NumberFormatException e)
    {
      return null;
    }
  }

  public void setError(String errorMessage)
  {
    numberField.setError(errorMessage);
  }

  //#endregion

  //#region Private Methods

  private void init(Context context, AttributeSet attrs)
  {
    LayoutInflater.from(context).inflate(R.layout.number_field, this, true);

    labelField = findViewById(R.id.title_number_field);
    numberField = findViewById(R.id.value_number_field);

    if (attrs != null)
    {
      try (TypedArray typedArray = context.obtainStyledAttributes(attrs, R.styleable.NumberField))
      {
        String labelText = typedArray.getString(R.styleable.NumberField_labelText);
        setLabelText(labelText);
        typedArray.recycle();
      } catch (Exception e)
      {
        Log.e("NumberField", "Error initializing NumberField", e);
      }
    }
  }

  private String getNumberFieldText()
  {
    return numberField.getText().toString();
  }

  //#endregion

}
