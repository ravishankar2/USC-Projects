import java.io.*;
import java.sql.*;
import java.text.*;

public class Populate {

	/**
	 * @param args
	 */
	// File URL parameters
	static String dbparams, users, videos, advertisements,
			ad_target_categories, views, partners;
	// Connect DB parameters
	static String localhost, port, database, user, password;
	// DB parameters
	static Connection conn;
	static Statement statement;

	static void get_file_params(String[] args) {
		dbparams = args[0];
		users = args[1];
		videos = args[2];
		advertisements = args[3];
		ad_target_categories = args[4];
		views = args[5];
		partners = args[6];
	}

	static boolean get_db_params(String url) throws IOException {
		BufferedReader br;
		try {
			br = new BufferedReader(new FileReader(url));
			localhost = br.readLine();
			port = br.readLine();
			database = br.readLine();
			user = br.readLine();
			password = br.readLine();
			return true;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			System.out.println("Cannot load file " + url + "!");
			System.out.println("Please press Enter to quite...");
			int exit = System.in.read();
			return false;
			// e.printStackTrace();
		}
	}

	static boolean connect_db() throws IOException {
		String driver = "com.mysql.jdbc.Driver";
		String url = "jdbc:mysql://" + localhost + ":" + port + "/" + database;
		try {
			Class.forName(driver);
			conn = DriverManager.getConnection(url, user, password);
			if (!conn.isClosed()) {
				System.out.println("Database connecting succeeded!");
				statement = conn.createStatement();
				return true;
			} else {
				System.out
						.println("Cannot connect to database! Please check the connect information.");
				System.out.println("Please press Enter to quite...");
				int exit = System.in.read();
				return false;
			}
		} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			//e.printStackTrace();
			System.out.println("Cannot load the MySQL Driver.");
			System.out.println("Please press Enter to quite...");
			int exit = System.in.read();
			return false;
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			// e.printStackTrace();
			System.out
					.println("Cannot connect to database! Please check the connect information.");
			System.out.println("Please press Enter to quite...");
			int exit = System.in.read();
			return false;
		}
	}

	static void clear_db() {
		String sql;
		try {
			sql = "DELETE FROM partners";
			statement.executeUpdate(sql);
			sql = "DELETE FROM views";
			statement.executeUpdate(sql);
			sql = "DELETE FROM ad_target_categories";
			statement.executeUpdate(sql);
			sql = "DELETE FROM advertisements";
			statement.executeUpdate(sql);
			sql = "DELETE FROM videos";
			statement.executeUpdate(sql);
			sql = "DELETE FROM users";
			statement.executeUpdate(sql);
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	// Format string in order to make every element separated by #&
	static String format_string(String str) {
		String format_str = "";
		int count_quotes = 0;
		for (int i = 0; i < str.length(); i++) {
			if (str.charAt(i) == '"')
				count_quotes++;
			else if (str.charAt(i) == ',')
				if (count_quotes % 2 == 0)
					format_str += "#&";
				else
					format_str += str.charAt(i);
			else
				format_str += str.charAt(i);
		}
		return format_str;
	}

	// Check whether the element should be set to NULL
	static boolean check_valid(String[] str, int index) {
		if (str.length >= index)
			if (!str[index - 1].equals(""))
				return true;
		return false;
	}

	static void import_db() throws IOException {
		BufferedReader br;
		String str, sql;
		PreparedStatement pstmt;
		SimpleDateFormat time_format = new SimpleDateFormat(
				"yyyy-MM-dd hh:mm:ss");
		System.out.println("Database initializing...");
		try {
			// Insert Users Data
			br = new BufferedReader(new FileReader(users));
			sql = "INSERT INTO users (username," + "password," + "name,"
					+ "email," + "date_registered)" + " VALUES(?,?,?,?,?)";
			pstmt = conn.prepareStatement(sql);
			while ((str = br.readLine()) != null) {
				str = format_string(str);
				String str_column[] = str.split("#&");
				pstmt.setString(1, str_column[0]);
				pstmt.setString(2, str_column[1]);
				if (check_valid(str_column, 3))
					pstmt.setString(3, str_column[2]);
				else
					pstmt.setNull(3, Types.VARCHAR);
				if (check_valid(str_column, 4))
					pstmt.setString(4, str_column[3]);
				else
					pstmt.setNull(4, Types.VARCHAR);
				if (check_valid(str_column, 5))
					pstmt.setDate(5, java.sql.Date.valueOf(str_column[4]));
				else
					pstmt.setNull(5, Types.DATE);
				pstmt.executeUpdate();
			}
			// Insert Videos Data
			br = new BufferedReader(new FileReader(videos));
			sql = "INSERT INTO videos (video_id," + "title," + "uploaded_by,"
					+ "uploaded_at," + "category)" + " VALUES(?,?,?,?,?)";
			pstmt = conn.prepareStatement(sql);
			while ((str = br.readLine()) != null) {
				str = format_string(str);
				String str_column[] = str.split("#&");
				pstmt.setString(1, str_column[0]);
				if (check_valid(str_column, 2))
					pstmt.setString(2, str_column[1]);
				else
					pstmt.setNull(2, Types.VARCHAR);
				if (check_valid(str_column, 3))
					pstmt.setString(3, str_column[2]);
				else
					pstmt.setNull(3, Types.VARCHAR);
				if (check_valid(str_column, 4)) {
					java.util.Date date = time_format.parse(str_column[3]);
					java.sql.Timestamp time_stamp = new java.sql.Timestamp(
							date.getTime());
					pstmt.setTimestamp(4, time_stamp);
				} else
					pstmt.setNull(4, Types.TIMESTAMP);
				if (check_valid(str_column, 5))
					pstmt.setString(5, str_column[4]);
				else
					pstmt.setNull(5, Types.VARCHAR);
				pstmt.executeUpdate();
			}
			// Insert Advertisements Data
			br = new BufferedReader(new FileReader(advertisements));
			sql = "INSERT INTO advertisements (ad_id," + "advertiser,"
					+ "price," + "views_wanted)" + " VALUES(?,?,?,?)";
			pstmt = conn.prepareStatement(sql);
			while ((str = br.readLine()) != null) {
				str = format_string(str);
				String str_column[] = str.split("#&");
				pstmt.setString(1, str_column[0]);
				if (check_valid(str_column, 2))
					pstmt.setString(2, str_column[1]);
				else
					pstmt.setNull(2, Types.VARCHAR);
				if (check_valid(str_column, 3)) {
					java.math.BigDecimal decimal = new java.math.BigDecimal(
							str_column[2]);
					pstmt.setBigDecimal(3, decimal);
				} else
					pstmt.setNull(3, Types.DECIMAL);
				if (check_valid(str_column, 4)) {
					int integer = Integer.valueOf(str_column[3]).intValue();
					pstmt.setInt(4, integer);
				} else
					pstmt.setNull(4, Types.INTEGER);
				pstmt.executeUpdate();
			}
			// Insert Ad_Target_Categories Data
			br = new BufferedReader(new FileReader(ad_target_categories));
			sql = "INSERT INTO ad_target_categories (ad_id," + "category)"
					+ " VALUES(?,?)";
			pstmt = conn.prepareStatement(sql);
			while ((str = br.readLine()) != null) {
				str = format_string(str);
				String str_column[] = str.split("#&");
				pstmt.setString(1, str_column[0]);
				pstmt.setString(2, str_column[1]);
				pstmt.executeUpdate();
			}
			// Insert Views Data
			br = new BufferedReader(new FileReader(views));
			sql = "INSERT INTO views (user," + "video," + "viewed_at,"
					+ "rating," + "ad_shown," + "ad_clicked)"
					+ " VALUES(?,?,?,?,?,?)";
			pstmt = conn.prepareStatement(sql);
			while ((str = br.readLine()) != null) {
				str = format_string(str);
				String str_column[] = str.split("#&");
				pstmt.setString(1, str_column[0]);
				pstmt.setString(2, str_column[1]);
				java.util.Date date = time_format.parse(str_column[2]);
				java.sql.Timestamp time_stamp = new java.sql.Timestamp(
						date.getTime());
				pstmt.setTimestamp(3, time_stamp);
				int integer = 0;
				if (check_valid(str_column, 4)) {
					integer = Integer.valueOf(str_column[3]).intValue();
					pstmt.setInt(4, integer);
				} else
					pstmt.setNull(4, Types.INTEGER);
				if (check_valid(str_column, 5))
					pstmt.setString(5, str_column[4]);
				else
					pstmt.setNull(5, Types.VARCHAR);
				if (check_valid(str_column, 6)) {
					integer = Integer.valueOf(str_column[5]).intValue();
					pstmt.setInt(6, integer);
				} else
					pstmt.setNull(6, Types.INTEGER);
				pstmt.executeUpdate();
			}
			// Insert Partners Data
			br = new BufferedReader(new FileReader(partners));
			sql = "INSERT INTO partners (username," + "revenue_share)"
					+ " VALUES(?,?)";
			pstmt = conn.prepareStatement(sql);
			while ((str = br.readLine()) != null) {
				str = format_string(str);
				String str_column[] = str.split("#&");
				pstmt.setString(1, str_column[0]);
				if (check_valid(str_column, 2)) {
					java.math.BigDecimal decimal = new java.math.BigDecimal(
							str_column[1]);
					pstmt.setBigDecimal(2, decimal);
				} else
					pstmt.setNull(2, Types.VARCHAR);
				pstmt.executeUpdate();
			}
			System.out.println("Data initializing succeed!");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			System.out.println("Cannot load .csv file!");
			// e.printStackTrace();
		} catch (ParseException e) {
			// TODO Auto-generated catch block
			// e.printStackTrace();
			System.out.println("Error in Parse.");
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			System.out.println("Error in SQL.");
			// e.printStackTrace();
		} finally {
			System.out.println("Please press Enter to quite...");
			int exit = System.in.read();
		}
	}

	public static void main(String[] args) throws IOException {
		// TODO Auto-generated method stub
		if (args.length < 7) {
			System.out.println("Please input 7 parameters in order, try again.");
			System.out.println("Please press Enter to quite...");
			int exit = System.in.read();
			return;
		}
		get_file_params(args);
		if (get_db_params(dbparams)) {
			if (connect_db()) {
				clear_db();
				import_db();
			}
		}
	}

}
