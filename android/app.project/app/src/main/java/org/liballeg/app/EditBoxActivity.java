package org.liballeg.app;

import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.util.Log;
import android.text.InputType;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodManager;

public class EditBoxActivity extends Activity
{
	private TextView edit_title;
	private EditText edit_box;
	private Button ok_button, cancel_button;
	private Context context;

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.edit_box);

		context = this.getApplicationContext();

		Bundle extras = getIntent().getExtras();
		String title_string = extras.getString("EXTRA_TITLE");
		String initial_string = extras.getString("EXTRA_INITIAL");
		String flags = extras.getString("EXTRA_FLAGS");
		int text_flags = InputType.TYPE_CLASS_TEXT;
		edit_title = (TextView) findViewById(R.id.editTitle);
		edit_title.setText(title_string);
		edit_box = (EditText) findViewById(R.id.editInp);
		if(flags.equals("CapWords"))
		{
			text_flags |= InputType.TYPE_TEXT_FLAG_CAP_WORDS;
		}
		else if(flags.equals("CapSentences"))
		{
			text_flags |= InputType.TYPE_TEXT_FLAG_CAP_SENTENCES;
		}
		edit_box.setInputType(text_flags);
		edit_box.setText(initial_string);
		edit_box.setSelection(initial_string.length());
		edit_box.setOnEditorActionListener(new TextView.OnEditorActionListener() {
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event)
			{
				Intent intent = new Intent();
		        if (actionId == EditorInfo.IME_ACTION_DONE)
				{
		            InputMethodManager imm = (InputMethodManager)v.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
		            imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
					intent.putExtra("EXTRA_TEXT", edit_box.getText().toString());
					setResult(RESULT_OK, intent);
					finish();
		            return true;
		        }
		        return false;
		    }
		});
		ok_button = (Button) findViewById(R.id.okBtn);
		ok_button.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				Intent intent = new Intent();
				intent.putExtra("EXTRA_TEXT", edit_box.getText().toString());
				setResult(RESULT_OK, intent);
				finish();
			}
		});
		cancel_button = (Button) findViewById(R.id.cancelBtn);
		cancel_button.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				setResult(RESULT_CANCELED);
				finish();
			}
		});
	}
}
