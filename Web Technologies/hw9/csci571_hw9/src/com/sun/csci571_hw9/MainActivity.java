package com.sun.csci571_hw9;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;

public class MainActivity extends Activity {

	private static final String[] movie_types = { "All Types", "Feature Film",
			"TV Series", "Video Game" };
	private ArrayAdapter<String> adapter;

	private String getType(String type) {
		String value = "";
		if (type == "All Types")
			value = "all_types";
		if (type == "Feature Film")
			value = "feature";
		if (type == "TV Series")
			value = "tv_series";
		if (type == "Video Game")
			value = "game";
		return value;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		final EditText et = (EditText) findViewById(R.id.editText1);

		final Spinner sp = (Spinner) findViewById(R.id.spinner1);
		adapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_spinner_item, movie_types);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		sp.setAdapter(adapter);

		sp.setVisibility(View.VISIBLE);

		Button b = (Button) findViewById(R.id.button1);

		b.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub

				Intent intent = new Intent(MainActivity.this, MovieShow.class);
				intent.putExtra("movie_title", et.getText().toString());
				intent.putExtra("movie_type", getType(sp.getSelectedItem()
						.toString()));

				startActivity(intent);
			}
		});
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

}
