Database Systems
================

#### CSCI 585 – Database Systems – Fall 2012
###### Professor Dennis McLeod
###### Assignment 3
###### Due: November 8, 2012 @ 02:00 PM PST

In this assignment you will realize a practical database implementation based on a given relational schema within the MySQL database management system, write and run queries on it, and access it programmatically using Java Database Connectivity (JDBC). The tasks that you will accomplish in this assignment area as follows:

* create a database using DDL statements in the mysql command line tool
* populate the database row-by-row from provided raw data using JDBC
* write SQL queries to the database to be executed from the mysql command line tool
* query and manipulate the database programmatically using JDBC
* completely remove all traces of your database from the DBMS

#### Preliminaries: Installing and Using MySQL Community Server
The current Generally Available release of MySQL is version 5.5.28. Download link is given below. Detailed installation instructions can be found in the reference manual, which also provides a tutorial, extensive documentation of client and administrative programs, data types, SQL statement syntax, etc.

* Download: http://www.mysql.com/downloads/mysql/5.5.html
* Reference Manual: http://dev.mysql.com/doc/refman/5.5/en/

We will be using the mysql command line client to access the database; you can read more about it at http://dev.mysql.com/doc/refman/5.5/en/mysql.html. Other database management tools like MySQL Workbench, GUI Tools, phpMyAdmin, etc. are not required for this assignment; use of these tools will neither be accepted nor supported by the TAs and graders – use at your own risk.

#### Preliminaries: Java
For this assignment, we will use the Java Standard Edition (SE) 6 Update 35 JDK. Note that we will not be able to accommodate any compatibility issues if you use a different version. That includes Java SE 7.

* Download & instructions: http://www.oracle.com/technetwork/java/javase/downloads/index.html
* Documentation: http://download.oracle.com/javase/6/docs/
* API Specification: http://download.oracle.com/javase/6/docs/api/

Resources for those who are new to Java:

* The Java Tutorials: http://download.oracle.com/javase/tutorial/
* Thinking in Java (free e-book): http://www.mindview.net/Books/TIJ/

In case you don’t have access to a machine with requested version of the JDK, the USC Student Compute Facility (SCF) has Java SE 6 Update 35 installed on its servers; you can use this to verify that your programs will compile correctly in Java 6. Documentation on how to access SCF resources is available at http://www.usc.edu/its/unix/servers/scf.html. To setup your SCF server environment to use Java SE 6 Update 35, run the following command from the terminal. Note this will only work for the current login session. To make this permanent, you need to append this command to your ~/.login file:

	source /usr/usc/jdk/1.6.0_23/setup.csh

You are free to use any Integrated Development Environment (IDE) such as Eclipse, NetBeans, etc. if you wish. However, you must be able to compile and execute your code from the Windows command line (or Linux/Mac shell) environment, outside of the IDE.

#### Preliminaries: JDBC
To connect to your MySQL database from within your Java program, you will need to use JDBC. The current Generally Available release of the official MySQL JDBC driver is MySQL Connector/J 5.1.22. It can be obtained from http://www.mysql.com/downloads/connector/j/

* JDBC Documentation: http://download.oracle.com/javase/6/docs/technotes/guides/jdbc/
* API Specification: http://download.oracle.com/javase/6/docs/api/java/sql/package-summary.html
* Tutorial: http://download.oracle.com/javase/tutorial/jdbc/basics/index.html
* MySQL Connector Documentation: http://dev.mysql.com/doc/refman/5.5/en/connector-j.html

You may NOT use any other third-party libraries apart from MySQL Connector/J.

#### Connecting to the Database
To connect to your MySQL database from within your Java programs, you will need to supply it with the necessary connection parameters. For this assignment, these parameters must be stored in a separate configuration file to be passed as an argument to your Java program during execution. Your program should read the configuration file and use the parameters contained therein. Do not hardcode any of the connection parameters in your Java source code.

A sample dbparams.txt file has been provided for your reference. The five lines of this plain text file correspond to the host, port, database name, username and password for the database. Default values for host and port values have been used; you may need to adjust them accordingly if you had changed your settings when installing MySQL. Obviously, you will need to change the username and password according to your individual system setup in order to connect to your MySQL instance.

For grading, we will of course use a different dbparams.txt configuration file based on the setup of the machine used for grading. Make sure you follow the configuration file format. We will not be able to grade your assignment if your program fails to parse our database configuration file.

#### Application Domain Description
You will implement the database backend for an online video sharing website using the open source MySQL Community Server pseudo-relational database management system.

Obviously, the database needs to keep track of information about registered users of the website and the videos that they upload, view, and rate. Each uploaded video is put into one of a predefined set of content categories. A user can watch the same video multiple times; they may give a different rating to a video each time they view it, or they may not rate it at all.

To make money, the video sharing service depends on revenue generated from advertising. When a user views a video, they are often (but not always) shown an advertisement before the video begins. Any user can create an advertisement and pay a fixed total price to have it shown a specified number of times (number of views wanted). The advertiser can target their advertising at users with relevant interests by specifying one or more content categories for their advertisement; advertisements are more likely to be shown together with videos in those selected categories. For accounting and statistical purposes, the database tracks when advertisements are shown and whether or not users clicked on the advertisement.

Some prolific users who have uploaded many high quality videos may be invited into a revenue- sharing partnership in which they get paid a fraction of the earnings from advertisements that are shown on their uploaded videos.

#### Part 1: Creating the database
The schema for our database is given by Fig. 1. Primary keys have been underlined. Foreign keys have been omitted, but you are expected to identify and implement them accordingly.

##### Users
<table>
    <tr>
        <td><b>username</b></td><td>password</td><td>name</td><td>email</td><td>date_registered</td>
    </tr>
</table>

##### Videos
<table>
    <tr>
        <td><b>video_id</b></td><td>title</td><td>uploaded_by</td><td>uploaded_at</td><td>category</td>
    </tr>
</table>

##### Advertisements
<table>
    <tr>
        <td><b>ad_id</b></td><td>advertiser</td><td>price</td><td>views_wanted</td>
    </tr>
</table>

##### Ad_Target_Categories
<table>
    <tr>
        <td><b>ad_id</b></td><td><b>category</b></td>
    </tr>
</table>

##### Views
<table>
    <tr>
        <td><b>user</b></td><td><b>video</b></td><td><b>viewed_at</b></td><td>rating</td><td>ad_shown</td><td>ad_clicked</td>
    </tr>
</table>

##### Partners
<table>
    <tr>
        <td><b>username</b></td><td><b>revenue_share</b></td>
    </tr>
</table>

Fig. 1: Schema for VideoSharing

You are expected to prepare and submit a createdb.sql file containing SQL statements that will create a database VideoSharing containing the above tables. Primary keys and foreign keys must be implemented. Choose the most appropriate data type for each field.

#### Part 2: Populating the database
You are given samples of the following files, which contain data for the respective tables.

* users.csv
* videos.csv
* advertisements.csv
* ad_target_categories.csv
* views.csv
* partners.csv

These are standard comma-separated value files, which you may open and examine using any standard text editor. Observe that each row represents one record, and fields are comma-separated.

Note: The data files provided to you are samples demonstrating file format and attribute domains only. When grading, we will test your programs using a different set of unseen data. As such, you may wish to create additional data for your own testing purposes.

You are required to implement a Java program Populate.java that will accept filenames of the input data files as command line parameters, open and parse each file, and populate the data contained within them into your database by executing individual INSERT statements for each row using JDBC. Note: when executing the same statement repeatedly, you should use the JDBC PreparedStatement construct for greater efficiency as compared to the more generic Statement class.

Your program should be compiled and executed from the Windows command line or Linux/Mac shell. It should be compiled like this:

	> javac -classpath .;mysql-connector-java-5.1.22-bin.jar Populate.java

and executed like this (as one single long command):

	> java -classpath .;mysql-connector-java-5.1.22-bin.jar Populate <dbparams filename> <users filename> <videos filename> <advertisements filename> <ad_target_categories filename> <views filename> <partners filename>

Do not hardcode the locations and/or filenames of the configuration file and input data files into your program. We will use different filenames at different locations for grading.

Sample command line batch files / shell scripts for both compilation and execution are provided.

Note that every time you run this program, it should start by first removing the previous data in your tables; otherwise the tables will have redundant or incorrect data.

#### Part 3: SQL queries on the database
Write the following queries in SQL and run them on your MySQL database via the mysql command line tool. Prepare and submit them as separate files q1.sql to q8.sql. If two or more SQL statements are needed for a single question, they should be written after each other in one file.

Do not create or use views. Storing intermediate query results is not an appropriate use of views.

* Q1. List the advertisement ID, and advertiser name and email for each advertisement with a CPM (cost per mille, the price paid per one thousand views wanted) of less than 7 cents.

* Q2. Find the total price of all advertisements targeted at each category. (An advertisement targeted at multiple categories is counted in every category that it is targeted at.)

* Q3. List the names of all advertisers who have had at least one of their advertisements shown more times than they had requested. Each advertiser should not be listed more than once.

* Q4. Find the three partners who earned the most in the month of June 2012. List their names and June 2012 earnings. Partners are paid based on their revenue share and on a per ad shown basis.

* Q5. Find all videos that have received no views since the first two weeks after they were uploaded. For each such video, give the name of the upload user who uploaded it, the video title, and the date and time it was uploaded.

* Q6. List the names of all users in order of date registered with the newest member first. For those users who have uploaded at least one video, also give the number of views on their most-viewed video.

* Q7. Find the five videos with the lowest average rating despite having been rated by their own uploader with the highest rating of 5. List their video ID, title and average rating.

* Q8. Extra credit (Optional)

	Find all poorly targeted advertisements.
	
	A poorly targeted advertisement is defined as one that has a lower click-through rate (click- to-view percentage) when shown alongside videos that are in categories that it is targeted at, compared to videos that are not in one of the targeted categories.
	
	For each such advertisement give the advertisement ID, the name and email address of the advertiser, CPM paid on the advertisement, and the overall click-through rates when targeted and non-targeted. Sort the results with the highest CPM first.

#### Part 4: Querying the database using JDBC
You are required to implement a Java program Hw3.java that provides the capability to run queries on the system from the Windows command line or Linux/Mac shell environment. It should be compiled like this:

	$ javac -classpath .;mysql-connector-java-5.1.22-bin.jar Hw3.java

and executed like this (as one single long command):

	$ java -classpath .;mysql-connector-java-5.1.22-bin.jar Hw3 <dbparams filename> <query_number> <query_arguments>

Depending on the query number, your program should take the arguments provided and process the corresponding query, then terminate. (Query details are given below.)

Sample command line batch files / shell scripts are also provided:

* p4q1.bat thru p4q3.bat (Windows)
* p4q1.sh thru p4q3.sh (Linux/Mac)

You should test your program to ensure that they can be executed using the provided *.bat or *.sh files, depending on your OS. Do not modify the contents of any of the *.bat or *.sh files; in particular, the classpath must be used as-is in order to ensure compliance with the grading environment. You are also encouraged to come up with your own test cases using different query arguments. Remember that you can “reset” your database by running dropdb.sql (see Part 5), and then createdb.sql and Populate.java again.

* Q1. Given a year, month and day, provide the username, name and email of users who had registered to the website on that date. For instance, given:

		$ java -classpath .;mysql-connector-java-5.1.22-bin.jar Hw3 dbparams.txt q1 2002 10 18
		
Your program should find users who registered on 18 October 2002 and print the results to the screen in tabular form.
* Q2. Given a username and decimal value between 0 and 1, add that user as a partner with the specified revenue share amount. If the user is already a partner, change their revenue share to the specified amount. For instance, given:

		$ java -classpath .;mysql-connector-java-5.1.22-bin.jar Hw3 dbparams.txt q2 Hedley8 0.18
		
Your program should add Hedley8 as a partner with a 18% revenue share if the user was not already a partner, or change the revenue share to 18% if they were already an existing partner. Your program should provide confirmation on whether the operation succeeded or not, and reasons for failure where applicable.
* Q3. Given a decimal value, remove all partners with a revenue share equal to or higher than that:

		$ java -classpath .;mysql-connector-java-5.1.22-bin.jar Hw3 dbparams.txt q3 0.31

Your program should remove all partners who receive 31% or more of the advertising revenue from their videos, and provide confirmation on whether the operation succeeded, or provide reasons for failure where applicable.

#### Important note on JDBC (Part 2 and Part 4)
You are expected to incorporate sensible logic into your programs. For example, if your program is asked to run a query involving an inexistent video, you should return an informative error message. Likewise, you should print an informative message if a query returns no results.

Your programs are also expected to handle database errors gracefully, and return informative error messages before exiting rather than terminating abruptly. For instance, the connection to the database may fail, or the database user (as given in the configuration file) may not have the necessary permissions to perform the required operations.

You may assume, however, that your program will always be run according to the given command format – i.e., query arguments will be given in the specified order, and there will never be missing arguments. Dates given as argument for Q1 are also guaranteed to be valid dates.

#### Part 5: Cleaning up
Prepare and submit a dropdb.sql file that drops your entire database, including all types and tables that are created by createdb.sql.

No marks are allocated for this section; however, failure to clean up fully will incur a penalty.
