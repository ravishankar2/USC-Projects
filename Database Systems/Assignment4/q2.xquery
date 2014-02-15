xquery version "1.0";
declare function local:ios()
{
<vertex>
{
for $x in doc("appinfo.xml")//AppStat
where $x//Statistics//Platform='iOS'
return 
	let $ios := $x//Statistics[Platform='iOS']
	return	<ios><app_id>{data($x//AppID)}</app_id><download>{data($ios//Downloads)}</download>{$x//Last_Checked}</ios>
}
</vertex>
};
declare function local:info()
{
<vertex>
{
for $x in local:ios()//ios, $y in doc("appinfo.xml")//Apps, $z in doc("appinfo.xml")//Developer
where $x//download>55000 and $x//app_id=$y//ID and $y//Developer=$z//ID
return <result><company>{data($z//Company)}</company><website>{data($z//Website)}</website><phone>{data($z//Phone)}</phone>{$x//Last_Checked}</result>
}
</vertex>
};
declare function local:com($xml)
{
<vertex>
{
for $x in distinct-values($xml//result//company)
return <company>{$x}</company>
}
</vertex>
};
declare function local:construct()
{
<vertex>
{
let $info := local:info()
for $c in local:com($info)//company
let $s := $info//result[company=$c]
let $m := max(
for $t in $s//Last_Checked
return xs:date($t)
)
return
<result><company_name>{data($c)}</company_name><website>{distinct-values($s//website)}</website><recent_last_check>{$m}</recent_last_check>
{
	for $phone in distinct-values($s//phone)
	return <phones>{data($phone)}</phones>
}
</result>
}
</vertex>
};
<query2>
{
for $x in local:construct()//result
order by $x//recent_last_check ascending
return $x
}
</query2>