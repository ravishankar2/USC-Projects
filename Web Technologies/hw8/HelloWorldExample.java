import java.io.IOException;
import java.io.PrintWriter;
import java.util.ResourceBundle;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import java.net.*;
import java.io.*;

/**
 * The simplest possible servlet.
 *
 * @author Yu Sun
 */

public class HelloWorldExample extends HttpServlet {

    private static final long serialVersionUID = 1L;

    public void doGet(HttpServletRequest request,
                      HttpServletResponse response)
        throws IOException, ServletException
    {
        ResourceBundle rb = ResourceBundle.getBundle("LocalStrings",
            request.getLocale());
        request.setCharacterEncoding("UTF-8");
        response.setContentType("text/html; charset=UTF-8");
        PrintWriter out = response.getWriter();

        String media_title = request.getParameter("title");
        String media_type = request.getParameter("type");
        //String media_title = "batman";
        //String media_type = "feature";
        URLEncoder.encode(media_title, "UTF-8");
        String media_temp = "";
        for (int i = 0; i < media_title.length(); i++) {
            if (media_title.charAt(i) == ' ') {
                media_temp += "%20";
            } else {
                media_temp += media_title.charAt(i);
            }
        }
        media_title = media_temp;
        try
        {
            URL url = new URL("http://cs-server.usc.edu:16271/get_movies2.php?title=" + media_title + "&media_type=" + media_type);
            URLConnection urlConnection = url.openConnection();
            urlConnection.setAllowUserInteraction(false);
            InputStream urlStream = url.openStream();

            // read content
            BufferedReader in = new BufferedReader(new InputStreamReader(urlStream));

            String xml_content;
            // while ((inputLine = in.readLine()) != null)
            // out.println("<p>" + inputLine + "</p>");
            xml_content = in.readLine();
            in.close();

            // Parse the XML content
            // Get cover, title, year, director, details
            String _cover[] = xml_content.split("cover=\"");
            int node_sum = _cover.length;
            String[] cover = new String[node_sum - 1];
            String _title[] = xml_content.split("title=\"");
            String[] title = new String[node_sum - 1];
            String _year[] = xml_content.split("year=\"");
            String[] year = new String[node_sum - 1];
            String _director[] = xml_content.split("director=\"");
            String[] director = new String[node_sum - 1];
            String _rating[] = xml_content.split("rating=\"");
            String[] rating = new String[node_sum - 1];
            String _details[] = xml_content.split("details=\"");
            String[] details = new String[node_sum - 1];
            for (int i = 1; i < node_sum; i++) {
                String temp[] = _cover[i].split("\" title=");
                cover[i - 1] = temp[0];
                temp = _title[i].split("\" year=");
                title[i - 1] = temp[0];
                temp = _year[i].split("\" director=");
                year[i - 1] = temp[0];
                temp = _director[i].split("\" rating=");
                director[i - 1] = temp[0];
                temp = _rating[i].split("\" details=");
                rating[i - 1] = temp[0];
                temp = _details[i].split("\"/>");
                details[i - 1] = temp[0];
            }
            // Build JSON
            String json_content = "{\"results\":{ \"result\":[ ";
            for (int i = 0; i < node_sum - 1; i++) {
                json_content += "{\"cover\":\"" + cover[i] + "\", ";
                json_content += "\"title\":\"" + title[i] + "\", ";
                json_content += "\"year\":\"" + year[i] + "\", ";
                json_content += "\"director\":\"" + director[i] + "\", ";
                json_content += "\"rating\":\"" + rating[i] + "\", ";
                json_content += "\"details\":\"" + details[i] + "\"}, ";
            }
            json_content += "]} }";
            out.println(json_content);
        }
        catch (Exception e)
        {
            out.print("error!");
        }
    }
}



