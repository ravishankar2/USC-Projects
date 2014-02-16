package com.sun.csci571_hw9;

import java.io.*;
import java.net.*;
import java.util.*;

import org.json.*;

import com.facebook.android.DialogError;
import com.facebook.android.Facebook;
import com.facebook.android.FacebookError;
import com.facebook.android.Facebook.DialogListener;

import android.app.*;
import android.content.Context;
import android.content.Intent;
import android.graphics.*;
import android.os.AsyncTask;
import android.os.Bundle;
import android.text.Html;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.SimpleAdapter.ViewBinder;
import android.widget.TextView;
import android.view.Window;
import android.view.WindowManager;

public class MovieShow extends ListActivity {
	String movie_title = null;
	String movie_type = null;
	List<Map<String, Object>> list_view;
	String[] cover_array = null;
	String[] title_array = null;
	String[] year_array = null;
	String[] director_array = null;
	String[] rating_array = null;
	String[] details_array = null;
	Bitmap[] image_array = null;
	boolean set_listener = false;

	private String readStream(InputStream in) {
		BufferedReader reader = null;
		String line = "";
		try {
			// Log.d("Debug", "in the readStream");
			reader = new BufferedReader(new InputStreamReader(in));
			// while ((line = reader.readLine()) != null) {
			// System.out.println(line);
			// }
			// line = changeEscape(reader.readLine());
			line = Html.fromHtml(reader.readLine()).toString();
			// line = URLDecoder.decode(line, "UTF-8");
			// line = URLDecoder.decode(line, "ISO-8859-1");
			// Log.d("Debug", "line" + line);
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (reader != null) {
				try {
					reader.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		return line;
	}

	ProgressDialog pd;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		pd = new ProgressDialog(this);
		pd.setProgressStyle(ProgressDialog.STYLE_SPINNER);
		pd.setMessage("Loading...");
		// pd.setIndeterminate(true);
		// pd.setCancelable(true);
		pd.show();
		prepareMovies prepare = new prepareMovies();
		prepare.execute(this);
	}

	protected class prepareMovies extends AsyncTask<Context, Integer, String> {
		@Override
		protected String doInBackground(Context... params) {
			movie_title = getIntent().getExtras().getString("movie_title")
					.trim();
			movie_type = getIntent().getExtras().getString("movie_type");
			String movie_temp = "";
			for (int i = 0; i < movie_title.length(); i++) {
				if (movie_title.charAt(i) == ' ') {
					movie_temp += "%20";
				} else {
					movie_temp += movie_title.charAt(i);
				}
			}
			movie_title = movie_temp;
			// Log.d("Debug", "title:" + movie_title);
			// Log.d("Debug", "type:" + movie_type);
			String json_string = "";
			JSONObject json_content = null;
			JSONObject json_node = null;
			JSONArray json_array = null;
			try {
				// Log.d("Debug", "in the open url");
				URL url = new URL(
						"http://cs-server.usc.edu:16272/examples/servlet/HelloWorldExample?title="
								+ movie_title + "&type=" + movie_type);
				HttpURLConnection con = (HttpURLConnection) url
						.openConnection();
				json_string = readStream(con.getInputStream());
				// Log.d("Debug", "JSON_STRING" + json_string);
			} catch (Exception e) {
				// e.printStackTrace();
				return "error";
			}
			try {
				json_content = new JSONObject(json_string);
				json_node = json_content.getJSONObject("results");
				json_array = json_node.getJSONArray("result");
			} catch (JSONException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				return "error";
			}
			// Log.d("Debug", "JSON_CONTENT:" + json_content);
			if (json_array.length() > 0) {
				set_listener = true;
				int array_length = json_array.length() - 1;
				cover_array = new String[array_length];
				title_array = new String[array_length];
				year_array = new String[array_length];
				director_array = new String[array_length];
				rating_array = new String[array_length];
				details_array = new String[array_length];
				image_array = new Bitmap[array_length];
				for (int i = 0; i < array_length; i++) {
					try {
						JSONObject json_temp = (JSONObject) json_array.get(i);
						cover_array[i] = (String) json_temp.get("cover");
						title_array[i] = (String) json_temp.get("title");
						year_array[i] = json_temp.getString("year");
						director_array[i] = (String) json_temp.get("director");
						rating_array[i] = (String) json_temp.get("rating");
						details_array[i] = (String) json_temp.get("details");
						URL url;
						try {
							url = new URL(cover_array[i]);
							HttpURLConnection conn = (HttpURLConnection) url
									.openConnection();
							InputStream is = conn.getInputStream();
							image_array[i] = BitmapFactory.decodeStream(is);
						} catch (MalformedURLException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
							return "error";
						} catch (IOException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
							return "error";
						}

					} catch (JSONException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
						return "error";
					}
				}

				list_view = new ArrayList<Map<String, Object>>();
				Map<String, Object> map_temp = null;
				for (int i = 0; i < array_length; i++) {
					map_temp = new HashMap<String, Object>();
					map_temp.put("title", title_array[i] + " (" + year_array[i]
							+ ")");
					map_temp.put("rating", "Rating: " + rating_array[i]);
					map_temp.put("img", image_array[i]);
					list_view.add(map_temp);
				}
				return "yes";
			} else {
				return "no";
			}
		}

		@Override
		protected void onPostExecute(String result) {
			// super.onPostExecute(result);
			pd.dismiss();
			if (result == "yes")
				bindListView();
			else
				bindNullView(result);
		}
	}

	protected void bindListView() {
		SimpleAdapter adapter = new SimpleAdapter(this, list_view,
				R.layout.list_show, new String[] { "title", "rating", "img" },
				new int[] { R.id.title, R.id.rating, R.id.img });

		adapter.setViewBinder(new ViewBinder() {
			@Override
			public boolean setViewValue(View view, Object data,
					String textRepresentation) {
				if (view instanceof ImageView && data instanceof Bitmap) {
					ImageView iv = (ImageView) view;
					iv.setImageBitmap((Bitmap) data);
					return true;
				} else
					return false;
			}
		});

		setListAdapter(adapter);
	}

	protected void bindNullView(String str) {
		list_view = new ArrayList<Map<String, Object>>();
		Map<String, Object> map_temp = new HashMap<String, Object>();
		if (str == "no")
			map_temp.put("error", "Cannot find movie '" + movie_title
					+ "' of type '" + movie_type + "'!");
		else
			map_temp.put("error", "Some errors happen, please try again:)");
		list_view.add(map_temp);

		SimpleAdapter adapter = new SimpleAdapter(this, list_view,
				R.layout.no_movie, new String[] { "error" },
				new int[] { R.id.nomovietext });

		setListAdapter(adapter);
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		// Log.v("Debug", (String) list_view.get(position).get("title"));
		// String item = (String) getListAdapter().getItem(position);
		if (set_listener)
			showDetail(id);
	}

	public void showDetail(long lid) {
		final int id = (int) lid;

		final Dialog dialog = new Dialog(MovieShow.this);
		dialog.setContentView(R.layout.detail_show);
		dialog.setTitle("Details");

		TextView name = (TextView) dialog.findViewById(R.id.name);
		name.setText("Name: " + title_array[id]);
		TextView year = (TextView) dialog.findViewById(R.id.year);
		year.setText("Year: " + year_array[id]);
		TextView director = (TextView) dialog.findViewById(R.id.director);
		director.setText("Director: " + director_array[id]);
		TextView rating = (TextView) dialog.findViewById(R.id.rating);
		rating.setText("Rating: " + rating_array[id] + "/10");
		ImageView image = (ImageView) dialog.findViewById(R.id.image);
		image.setImageBitmap(image_array[id]);
		Button button = (Button) dialog.findViewById(R.id.button1);
		button.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				// authenticatedFacebook.authorize(MovieShow.this, PERMISSIONS,
				// new MyListener());
				dialog.dismiss();

				Bundle params = new Bundle();
				params.putString("link", details_array[id]);
				params.putString("picture", cover_array[id]);
				params.putString("name", title_array[id]);
				params.putString("caption",
						"I'm interested in this movie/series/game");
				params.putString("description", title_array[id]
						+ " released in " + year_array[id]
						+ " has a rating of " + rating_array[id]);
				JSONObject properties = new JSONObject();
				try {
					JSONObject json_temp = new JSONObject();
					json_temp.put("text", "here");
					json_temp.put("href", details_array[id] + "reviews");
					properties.put("Look at user reviews", json_temp);
				} catch (JSONException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
				params.putString("properties", properties.toString());
				authenticatedFacebook.dialog(MovieShow.this, "stream.publish",
						params, new DialogListener() {

							@Override
							public void onFacebookError(FacebookError e) {
								// TODO Auto-generated method stub

							}

							@Override
							public void onError(DialogError e) {
								// TODO Auto-generated method stub

							}

							@Override
							public void onComplete(Bundle values) {
								// TODO Auto-generated method stub

							}

							@Override
							public void onCancel() {
								// TODO Auto-generated method stub

							}
						});
			}
		});

		Window dialogWindow = dialog.getWindow();
		WindowManager.LayoutParams lp = dialogWindow.getAttributes();
		lp.width = 400;
		lp.height = 450;
		dialog.show();
	}

	public static final String APP_ID = "558540930827124";

	// private static final String[] PERMISSIONS = new String[] {
	// "publish_stream", "read_stream", "offline_access" };

	Facebook authenticatedFacebook = new Facebook(APP_ID);
}
