xquery version "1.0";
<query1>
{
for $x in doc("appinfo.xml")//AppInfo//Apps, $y in doc("appinfo.xml")//AppInfo//Developer
where $x//Tablet_Compatible=true() and $x//Developer=$y//ID
order by $x//Name descending
return <result><id>{data($x//ID)}</id><name>{data($x//Name)}</name><developer_name>{data($y//Name)}</developer_name><price>{data($x//Price)}</price></result>
}
</query1>