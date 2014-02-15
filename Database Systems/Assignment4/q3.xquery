xquery version "1.0";
declare function local:rate($appid)
{
let $rating := doc("appinfo.xml")//AppStat[AppID=$appid ]
return
	data($rating//Statistics//Rating)
};
declare function local:average($appid, $rate)
{
let $rating := $rate 
return <nodes><appid>{data($appid)}</appid><avg_rating>{avg($rating)}</avg_rating></nodes>
};
declare function local:info()
{
<vertex>
{
for $x in doc("appinfo.xml")//AppStat//AppID
let $rating := local:average($x, local:rate($x))
return $rating
}
</vertex>
};
declare function local:maximum($info)
{
let $a := $info//nodes
return max(data($a//avg_rating))
};
<query3>
{
let $max := local:maximum(local:info())
for $x in (local:info())//nodes, $y in doc("appinfo.xml")//Apps, $z in doc("appinfo.xml")//Developer
where $x//avg_rating=$max and $x//appid=$y//ID and $z//ID=$y//Developer
return <result><App_ID>{data($x//appid)}</App_ID><App_Name>{data($y//Name)}</App_Name><Developer>{data($z//Name)}</Developer><Average_Rating>{$max}</Average_Rating></result>
}
</query3>
}
</query3>