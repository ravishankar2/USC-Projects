<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:fn="http://www.w3.org/2005/xpath-functions">
<xsl:template match="/">
	<html>
		<body>
		<h2>Developer information</h2>
			<table border="1" bgcolor="green" style="font-family:Sans-Serif; font-size:14px; border-collapse:collapse;">
				<tr>
					<th>ID</th>
					<th>Name</th>
					<th>Email</th>
					<th>Company</th>
					<th>Website</th>
					<th>Phone</th>
				</tr>
				<xsl:for-each select="AppInfo/Developer">
				<tr>
					<td><xsl:value-of select="ID"/></td>
					<td><xsl:value-of select="Name"/></td>
					<td><xsl:value-of select="Email"/></td>
					<td><xsl:value-of select="Company"/></td>
					<td><xsl:value-of select="Website"/></td>
					<td><xsl:value-of select="Phone"/></td>
				</tr>
				</xsl:for-each>
			</table>
		<h2>Apps information</h2>
			<table border="2" bgcolor="yellow" style="font-family:Arial; font-size:12px; border-collapse:collapse;">
				<tr>
					<th>ID</th>
					<th>Name</th>
					<th>Developer</th>
					<th>Price</th>
					<th>Tablet Compatible</th>
					<th>Category</th>
					<th width="50">Platform</th>
				</tr>
				<xsl:for-each select="AppInfo/Apps">
				<tr>
					<td align="center"><xsl:value-of select="ID"/></td>
					<td align="center"><xsl:value-of select="Name"/></td>
					<td align="center"><xsl:value-of select="Developer"/></td>
					<td align="center"><xsl:value-of select="Price"/></td>
					<td align="center">
					<xsl:if test="Tablet_Compatible='true'">
						Yes
					</xsl:if>
					</td>
					<td align="center"><xsl:value-of select="Category"/></td>
					<td>
					<table border="2" cellspacing="0" frame="void" bgcolor="yellow" style="font-family:Arial; font-size:12px; border-collapse:collapse;">
						<xsl:for-each select="Platform/Platform_Node">
							<tr>
								<td width="60">
									<xsl:value-of select="."/>
								</td>
							</tr>
						</xsl:for-each>
					</table>
					</td>
				</tr>
				</xsl:for-each>
			</table>
			<h2>Appstats</h2>
			<table border="1" bgcolor="grey" style="font-family:Times; font-size:10px; border-collapse:collapse;">
				<tr style="font-weight: bold">
					<td rowspan="2" align="center">AppID</td>
					<td colspan="3" align="center">Statistics</td><td rowspan="2">Last Checked</td>
				</tr>
				<tr style="font-weight: bold">
					<td width="60" align="center">Platform</td>
					<td width="60" align="center">Downloads</td>
					<td width="40" align="center">Rating</td>
				</tr>
				<xsl:for-each select="AppInfo/AppStat">
				<tr>
					<td align="center"><xsl:value-of select="AppID"/></td>
					<td colspan="3">
					<table border="1" cellspacing="0" frame="void" bgcolor="grey" style="font-family:Times; font-size:10px; border-collapse:collapse;">
						<xsl:for-each select="Statistics">
							<tr>
								<td width="60">
									<xsl:value-of select="Platform"/>
								</td>
								<td align="center" width="60">
									<xsl:value-of select="Downloads"/>
								</td>
								<td align="center" width="40">
									<xsl:value-of select="Rating"/>
								</td>
							</tr>
						</xsl:for-each>
					</table>
					</td>
					<td align="center"><xsl:value-of select="Last_Checked"/></td>
				</tr>
				</xsl:for-each>
			</table>
		</body>
	</html>
</xsl:template>
</xsl:stylesheet>
