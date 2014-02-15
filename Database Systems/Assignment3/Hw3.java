import java.io.*;
import java.sql.*;
import java.util.Formatter;
import java.util.Locale;

public class Hw3 {

	/**
	 * @param args
	 */
	// File URL parameters
	static String dbparams, query_no;
	static String[] arguments = null;
	// Connect DB parameters
	static String localhost, port, database, user, password;
	// DB parameters
	static Connection conn;
	static Statement statement;

	static void get_file_params(String[] args) {
		dbparams = args[0];
		query_no = args[1];
		arguments = new String[args.length - 2];
		for (int i = 0; i < arguments.length; i++) {
			arguments[i] = new String(args[i + 2]);
		}
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

	static void connect_db() {
		String driver = "com.mysql.jdbc.Driver";
		String url = "jdbc:mysql://" + localhost + ":" + port + "/" + database;
		try {
			Class.forName(driver);
			conn = DriverManager.getConnection(url, user, password);
			if (!conn.isClosed())
				System.out.println("Database connection succeeded!");
			statement = conn.createStatement();
		} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	static void excute_sql_q1() throws SQLException, IOException {
		String reg_date = arguments[0] + "-" + arguments[1] + "-"
				+ arguments[2];
		String sql = "select * from users where date_registered = '" + reg_date
				+ "'";
		ResultSet rs = statement.executeQuery(sql);
		String username, name, email;
		System.out.println("q1 Result:");
		if (rs.next()) {
			rs.previous();
			StringBuilder str_title = new StringBuilder();
			Formatter formatter_title = new Formatter(str_title, Locale.US);
			formatter_title.format("%1$-30s\t%2$-30s\t%3$-30s", "Username",
					"Name", "Email");
			System.out.println(str_title);
			while (rs.next()) {
				username = rs.getString("username");
				name = rs.getString("name");
				email = rs.getString("email");
				StringBuilder strb = new StringBuilder();
				Formatter formatter = new Formatter(strb, Locale.US);
				formatter.format("%1$-30s\t%2$-30s\t%3$-30s\t", username, name,
						email);
				System.out.println(strb);
			}
		} else {
			System.out.println("Cannot find a user who registered on "
					+ reg_date + ".");
		}
		System.out.println("Please press Enter to quite...");
		int exit = System.in.read();
		rs.close();
		conn.close();
	}

	static void excute_sql_q2() throws SQLException, IOException {
		String username = arguments[0], revenue_share = arguments[1];
		java.math.BigDecimal revenue_share_decimal = new java.math.BigDecimal(
				revenue_share);
		String sql;
		ResultSet rs;
		sql = "select * from users where username = '" + username + "'";
		rs = statement.executeQuery(sql);
		if (rs.next()) {
			sql = "select * from partners where username = '" + username + "'";
			rs = statement.executeQuery(sql);
			if (rs.next()) {
				String sql_update = "UPDATE partners SET revenue_share = ? WHERE username =?";
				PreparedStatement pstmt = conn.prepareStatement(sql_update);
				pstmt.setBigDecimal(1, revenue_share_decimal);
				pstmt.setString(2, username);
				pstmt.executeUpdate();
				System.out.println("q2 Update succeed!");
				System.out.println("Please press Enter to quite...");
				int exit = System.in.read();
			} else {
				String sql_insert = "INSERT INTO partners (username,revenue_share) VALUES (?,?)";
				PreparedStatement pstmt = conn.prepareStatement(sql_insert);
				pstmt.setString(1, username);
				pstmt.setBigDecimal(2, revenue_share_decimal);
				pstmt.executeUpdate();
				System.out.println("q2 Insert succeed!");
				System.out.println("Please press Enter to quite...");
				int exit = System.in.read();
			}
		} else {
			System.out.println("q2 Foreign key constraint fails, cannot find "
					+ username + " in users!");
			System.out.println("Please press Enter to quite...");
			int exit = System.in.read();
		}
		conn.close();
	}

	static void excute_sql_q3() throws SQLException, IOException {
		java.math.BigDecimal revenue_share_decimal = new java.math.BigDecimal(
				arguments[0]);
		String sql_delete = "DELETE FROM partners WHERE revenue_share>=?";
		PreparedStatement pstmt = conn.prepareStatement(sql_delete);
		pstmt.setBigDecimal(1, revenue_share_decimal);
		int row = pstmt.executeUpdate();
		if (row == 0) {
			System.out
					.println("q3 Failed! There is no partner whose revenue is share more than "
							+ Double.parseDouble(arguments[0]) * 100 + "%.");
			System.out.println("Please press Enter to quite...");
			int exit = System.in.read();
		} else
			System.out.println("q3 Remove succeed!");
		System.out.println("Please press Enter to quite...");
		int exit = System.in.read();
	}

	public static void main(String[] args) throws SQLException, IOException {
		// TODO Auto-generated method stub
		get_file_params(args);
		if (get_db_params(dbparams)) {
			connect_db();
			if (query_no.equals("q1"))
				excute_sql_q1();
			if (query_no.equals("q2"))
				excute_sql_q2();
			if (query_no.equals("q3"))
				excute_sql_q3();
		}
	}

}
