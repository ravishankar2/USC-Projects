xquery version "1.0";
declare function local:apps($company)
{
<company name="{$company}">
{
let $developers := doc("appinfo.xml")//Developer[Company=$company]
return
		for $i in $developers 
		return 
			<developer id="{data($i//ID)}">
			{
				let $apps := doc("appinfo.xml")//Apps[Developer=data($i//ID)]
				return
					 for $a in $apps 
					 return
						<app id="{data($a//ID)}">{data($a//Category)}</app>
			}
			</developer>
}
</company>
}; 
declare function local:company()
{
<vertex>
{
for $x in distinct-values(doc("appinfo.xml")//Developer//Company)
let $apps := local:apps($x)
where $apps//developer//app='Travel'
return <company>{data($x)}</company>
}
</vertex>
};
declare function local:count()
{
<vertex>
{
for $x in distinct-values(doc("appinfo.xml")//Developer//Company)
let $developer := doc("appinfo.xml")//Developer[Company=$x]
return <nodes><names>{data($x)}</names><counts>{count($developer)}</counts></nodes>
}
</vertex>
};
<query4>
{
for $x in local:company()//company, $y in local:count()//nodes
let $website := distinct-values(doc("appinfo.xml")//Developer[Company=$x]//Website)
where $x=$y//names
return <result><company_name>{data($x)}</company_name><website>{$website}</website><developer_num>{data($y//counts)}</developer_num></result>
}
</query4>
}
</query4>