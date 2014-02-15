xquery version "1.0";
declare function local:apps($platform)
{
for $x in doc("appinfo.xml")//AppStat//Statistics
where $x//Platform=$platform 
return <download>{data($x//Downloads)}</download> 
};
declare function local:platform()
{
<vertex>
{
for $x in distinct-values(doc("appinfo.xml")//AppStat//Statistics//Platform)
let $app := local:apps($x)
return <nodes platform="{data($x)}">{$app}</nodes>
}
</vertex>
};
declare function local:maximum($p)
{
<vertex>
{
for $i in $p//nodes
let $d := $p//nodes[@platform=$i/@platform]
return <nodes><platform>{data($i/@platform)}</platform><maximum>{max($d//download)}</maximum></nodes>
}
</vertex>
};
declare function local:stat()
{
<vertex>
{
for $x in doc("appinfo.xml")//AppStat
return
	for $y in $x//Statistics
	return <app><aid>{data($x//AppID)}</aid><downloads>{data($y//Downloads)}</downloads><platform>{data($y//Platform)}</platform></app>
}
</vertex>
};
<query5>
{
for $x in local:maximum(local:platform())//nodes, $y in local:stat()//app, $a in doc("appinfo.xml")//Apps, $d in doc("appinfo.xml")//Developer
where $x//maximum=$y//downloads and $x//platform=$y//platform and $a//ID=$y//aid and $a//Developer=$d//ID
return <result><app>{data($a//Name)}</app><developer>{data($d//Name)}</developer><company>{data($d//Company)}</company><platform>{data($y//platform)}</platform><download>{data($y//downloads)}</download></result>
}
</query5>