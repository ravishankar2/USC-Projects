Database Systems
================

#### CSCI 585 – Database Systems – Fall 2012
###### Professor Dennis McLeod
###### Assignment 4
###### Due: November 29, 2012 @ 02:00 PM PST

In this assignment you will explore the design and implementation of XML Schemas, XML Stylesheets and the use of the XML query language XQuery to query XML data.

#### Preliminaries: W3C Specifications
The technical reports specifying the standards/recommendations used in this assignment are as follows:

* XML: http://www.w3.org/TR/xml/
* XML Stylesheet: http://www.w3.org/TR/xml-stylesheet/
* XML Schema: http://www.w3.org/TR/xmlschema-1/
* XQuery: http://www.w3.org/TR/xquery/

While working on this assignment, you are free to use any XML editor and/or tools of your choice, as long as you verify that your submission will properly execute on our grading platform specified in the following section. (Use of such tools is entirely optional.) Some of the commonly used XML editors are:

1. Altova XMLSpy 2013 (for Windows):
	* Download: http://www.altova.com/download.html
	* Documentation: http://www.altova.com/solutions_center.html
2. EditiX XML Editor 2012 (for Windows, Mac & Linux)
	* Download: http://www.editix.com/download.html
	* Documentation: http://www.editix.com/doc/manual10/index.html

The tutorial website w3schools.com has tutorials on XML, XML Schema, XPath, XSLT and XQuery that you may find helpful in doing this assignment – but beware of implementation-specific variations in syntax and behavior. Ultimately, you must ensure that your solution works on our grading platform.

#### Preliminaries: Grading Platform
This assignment will be graded using a standard platform of tools and programs accessible via USC’s Student Computing Facility. It is your responsibility to ensure that your submission works under the procedures described in this section.

Parts 1 and 2 of the assignment will be graded by validating your XML file against its XML Schema using the xmllint tool on aludra.usc.edu. Fig. 1 demonstrates usage of this tool and illustrates the result of a successful validation:

	aludra.usc.edu(1): xmllint --noout --schema appinfo.xsd appinfo.xml appinfo.xml validates
	aludra.usc.edu(2):
Figure 1: xmllint tool on aludra.usc.edu

Part 3 will be graded using Firefox (version 13 and above) to display your XML file with the formatting specified in your XML stylesheet. If you do not have at least version 13 of Firefox installed on your own computer, you can test your submission at USC’s Public Computing Centers, which have Firefox 13 available as standard software.

For Part 4, we will process your XQuery queries on aludra using Qexo, an open source XQuery implementation. Figure 2 demonstrates how to install and run Qexo on aludra and use it to execute the query in q1.xquery.

	aludra.usc.edu(3): source /usr/usc/jdk/default/setup.csh
	aludra.usc.edu(4): wget http://ftp.gnu.org/pub/gnu/kawa/kawa-1.9.1.jar
	aludra.usc.edu(5): java -jar kawa-1.9.1.jar --xquery q1.xquery
Figure 2: Downloading and using Qexo on aludra.usc.edu

This prints the XQuery result to screen; you can also redirect the output to an XML file.

#### Part 1: Design XML Schema
A market research firm would like to keep track of information about top mobile apps and its developers to use in their studies. You would like to represent this data in XML. Given the information shown below about the data, create a valid XML Schema appinfo.xsd.

You should use appropriate data types, including defining your own complex types as applicable. Restrictions should be set on the format or range of possible values where necessary.

Each Developer has:

* ID (1 occurrence, required. Format: two letters, two numbers followed by a letter)
* Name (1 occurrence, required. Format: first name [space] last name)
* Email (1 occurrence, required. A valid email)
* Company (1 occurrence, required)
* Website (1 occurrence, required)
* Phone (1 occurrence, required. Format: “(xxx) xxx-yyyy” where x and y are numbers and xxx does not start with a “0”)

Each App has:

* ID (1 occurrence, required. Format: four numbers followed by an upper case letter. Does not start with a “0”)
* Name (1 occurrence, required)
* Developer (1 occurrence, required. Type: Developer/ID)
* Price (1 occurrence, required. Format: number in the range [0.00, 6.99], both inclusive.
* Tablet Compatible (1 occurrence, optional. Format: true/false)
* Category (1 occurrence, required. The only permitted values are: {Travel, Productivity, Game, Music, Education, Lifestyle}
* Platform (1 occurrence, required. The only permitted values are: {Android, iOS, Blackberry})

Each AppStats has:

* AppID (1 occurrence, required. Type: App/ID)
* Statistics (1 or more occurrences, required) an occurrence of this element should have in order:
	- Platform (1 occurrence, required. Type: App/Platform
	- Downloads (1 occurrence, required. Format: positive, non-zero integer)
	- Rating (1 occurrence, required. Format: number in the range [0.0, 5.0], both inclusive)
* Last Checked (1 occurrence, required (1 occurrence, required, format: YYYY-MM-DD)

#### Part 2: Create XML Data Files
Create a valid XML data file appinfo.xml based on the XML Schema in part 1 for the data given in the following tables:

##### Developer
<table>
    <tr>
        <td><b>ID</b></td>
        <td><b>Name</b></td>
        <td><b>Email</b></td>
        <td><b>Company</b></td>
        <td><b>Website</b></td>
        <td><b>Phone</b></td>
    </tr>
    <tr>
        <td>EJ23P</td>
        <td>Mary Barton</td>
        <td>mbarton@4wireapps.net</td>
        <td>4WireApps</td>
        <td>http://4wireapps.net</td>
        <td>(639)839-4568</td>
    </tr>
    <tr>
        <td>YF46K</td>
        <td>David Tucker</td>
        <td>tucker.d@appliodev.com</td>
        <td>Applio Dev</td>
        <td>http://appliodev.com</td>
        <td>(778)807-8224</td>
    </tr>
    <tr>
        <td>HX91P</td>
        <td>Yuli Deleon</td>
        <td>deleon@infosoft.com</td>
        <td>InfoSoft</td>
        <td>http://infosoft.com</td>
        <td>(337)838-2223</td>
    </tr>
    <tr>
        <td>VJ62A</td>
        <td>Matt Reynolds</td>
        <td>mr@4wireapps.net</td>
        <td>4WireApps</td>
        <td>http://4wireapps.net</td>
        <td>(639)839-1486</td>
    </tr>
    <tr>
    	<td>ES60N</td>
		<td>Ryder Lee</td>
		<td>lee@msoft.com</td>
		<td>M-Soft Inc.</td>
		<td>http://msoft.com</td>
		<td>(572) 271-6905</td>
    </tr>
	<tr>
		<td>JW41H</td>
		<td>Garrison Nolan</td>
		<td>gnolan@ wsharklabs.com</td>
		<td>WhiteShark Labs</td>
		<td>http://wsharklabs.com</td>
		<td>(859)105-5595</td>
	</tr>
	<tr>
		<td>VY76D</td>
		<td>Inez Farley</td>
		<td>ifarley@wsharklabs.com</td>
		<td>WhiteShark Labs</td>
		<td>http://wsharklabs.com</td>
		<td>(859)105-4502</td>
	</tr>
</table>

##### Apps
<table>
    <tr>
        <td><b>ID</b></td>
        <td><b>Name</b></td>
        <td><b>Developer</b></td>
        <td><b>Price</b></td>
        <td><b>Tablet Compatible</b></td>
        <td><b>Category</b></td>
        <td><b>Platform</b></td>
    </tr>
    <tr>
		<td>9311E</td>
		<td>TravelBug</td>
		<td>EJ23P</td>
		<td>0.99</td>
		<td>Yes</td>
		<td>Travel</td>
		<td>Android iOS BlackBerry</td>
	</tr>
	<tr>
		<td>1363M</td>
		<td>FlightOgram</td>
		<td>VY76D</td>
		<td>1.99</td>
		<td>Yes</td>
		<td>Travel</td>
		<td>iOS BlackBerry</td>
	</tr>
	<tr>
		<td>3455J</td>
		<td>InstaNews</td>
		<td>VJ62A</td>
		<td>0.00</td>
		<td>Yes</td>
		<td>Productivity</td>
		<td>iOS</td>
	</tr>
	<tr>
		<td>4610N</td>
		<td>Peak Interest</td>
		<td>VJ62A</td>
		<td>3.99</td>
		<td></td>
		<td>Travel</td>
		<td>Android</td>
	</tr>
	<tr>
		<td>5271X</td>
		<td>Power Cal</td>
		<td>VY76D</td>
		<td>0.00</td>
		<td>Yes</td>
		<td>Productivity</td>
		<td>Android</td>
	</tr>
	<tr>
		<td>5345V</td>
		<td>SoundWhale</td>
		<td>JW41H</td>
		<td>0.00</td>
		<td></td>
		<td>Music</td>
		<td>Android iOS</td>
	</tr>
	<tr>
		<td>6006X</td>
		<td>Mobile Guitar</td>
		<td>EJ23P</td>
		<td>0.99</td>
		<td>Yes</td>
		<td>Music</td>
		<td>Android OS BlackBerry</td>
	</tr>
	<tr>
		<td>6126C</td>
		<td>PictoGuide</td>
		<td>JW41H</td>
		<td>0.99</td>
		<td></td>
		<td>Travel</td>
		<td>iOS BlackBerry</td>
	</tr>
	<tr>
		<td>6172B</td>
		<td>Angry Camels</td>
		<td>VJ62A</td>
		<td>0.99</td>
		<td>Yes</td>
		<td>Game</td>
		<td>Android iOS BlackBerry</td>
	</tr>
	<tr>
		<td>6263C</td>
		<td>Mobiodic Table</td>
		<td>ES60N</td>
		<td>1.99</td>
		<td></td>
		<td>Education</td>
		<td>Android iOS BlackBerry</td>
	</tr>
	<tr>
		<td>7394D</td>
		<td>GooCity</td>
		<td>HX91P</td>
		<td>0.00</td>
		<td>Yes</td>
		<td>Game</td>
		<td>Android iOS BlackBerry</td>
	</tr>
	<tr>
		<td>8987N</td>
		<td>OpenDiner</td>
		<td>EJ23P</td>
		<td>1.99</td>
		<td>Yes</td>
		<td>Lifestyle</td>
		<td>BlackBerry</td>
	</tr>
	<tr>
		<td>9554R</td>
		<td>Gulp Radio</td>
		<td>HX91P</td>
		<td>3.99</td>
		<td></td>
		<td>Music</td>
		<td>Android iOS</td>
	</tr>
</table>

##### AppStats
<table>
	<tr>
        <td><b>AppID</b></td>
        <td>
        	<tr><td><b>tr</b></td></tr>
        	<tr>
        		<td><b>Platform</b></td>
        		<td><b>Downloads</b></td>
        		<td><b>Rating</b></td>
        	</tr>
        </td>
        <td><b>Last Checked</b></td>
    </tr>
	<tr>
		<td>9311E</td>
		<td>
			<tr>
				<td>Android</td>
				<td>90000</td>
				<td>5</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>15000</td>
				<td>3</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Blackberry</td>
				<td>42000</td>
				<td>4.5</td>
			</tr>
		</td>
		<td>2012-05-30</td>
	</tr>
	<tr>
		<td>1363M</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>35000</td>
				<td>3.5</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Blackberry</td>
				<td>89000</td>
				<td>4.5</td>
			</tr>
		</td>
		<td>2012-08-15</td>
	</tr>
	<tr>
		<td>3455J</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>83000</td>
				<td>3.5</td>
			</tr>
		</td>
		<td>2012-06-18</td>
	</tr>
	<tr>
		<td>4610N</td>
		<td>
			<tr>
				<td>Android</td>
				<td>46000</td>
				<td>2.5</td>
			</tr>
		</td>
		<td>2012-08-08</td>
	</tr>
	<tr>
		<td>5271X</td>
		<td>
			<tr>
				<td>Android</td>
				<td>31000</td>
				<td>4</td>
			</tr>
		</td>
		<td>2012-10-30</td>
	</tr>
	<tr>
		<td>5345V</td>
		<td>
			<tr>
				<td>Android</td>
				<td>7000</td>
				<td>2.5</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>95000</td>
				<td>5</td>
			</tr>
		</td>
		<td>2012-08-20</td>
	</tr>
	<tr>
		<td>6006X</td>
		<td>
			<tr>
				<td>Android</td>
				<td>66000</td>
				<td>4</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Blackberry</td>
				<td>45000</td>
				<td>5</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>98000</td>
				<td>3.5</td>
			</tr>
		</td>
		<td>2012-07-31</td>
	</tr>
	<tr>
		<td>6126C</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>94000</td>
				<td>2.5</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Android</td>
				<td>34000</td>
				<td>5</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Blackberry</td>
				<td>15000</td>
				<td>2.5</td>
			</tr>
		</td>
		<td>2012-07-30</td>
	</tr>
	<tr>
		<td>6172B</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>8000</td>
				<td>3</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Blackberry</td>
				<td>3000</td>
				<td>2</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Android</td>
				<td>12000</td>
				<td>4</td>
			</tr>
		</td>
		<td>2012-06-30</td>
	</tr>
	<tr>
		<td>6263C</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>46000</td>
				<td>3.5</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Android</td>
				<td>95000</td>
				<td>3.5</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Blackberry</td>
				<td>1000</td>
				<td>4</td>
			</tr>
		</td>
		<td>2012-09-11</td>
	</tr>
	<tr>
		<td>7394D</td>
		<td>
			<tr>
				<td>Blackberry</td>
				<td>17000</td>
				<td>4</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>46000</td>
				<td>3</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>Android</td>
				<td>55000</td>
				<td>5</td>
			</tr>
		</td>
		<td>2012-06-17</td>
	</tr>
	<tr>
		<td>8987N</td>
		<td>
			<tr>
				<td>Blackberry</td>
				<td>47000</td>
				<td>3</td>
			</tr>
		</td>
		<td>2012-07-04</td>
	</tr>
	<tr>
		<td>9554R</td>
		<td>
			<tr>
				<td>Android</td>
				<td>5000</td>
				<td>2.5</td>
			</tr>
		</td>
		<td>
			<tr>
				<td>iOS</td>
				<td>27000</td>
				<td>2</td>
			</tr>
		</td>
		<td>2012-08-12</td>
	</tr>
</table>
