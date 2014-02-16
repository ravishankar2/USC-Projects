<?php
ini_set('default_charset', 'utf-8');
header('Content-Type:text/xml; charset=utf-8');
//$title = "batman";
//$title_type = "game";
$title = $_GET['title'];
$title_type = $_GET['media_type'];
//$title = utf8_encode($title);
if($title_type == "all_types")
	$url = "http://www.imdb.com/search/title?title=" . $title . "&title_type=feature,tv_series,game";
else
	$url = "http://www.imdb.com/search/title?title=" . $title . "&title_type=" . $title_type;
$url = str_replace(' ', '%20', $url);
$imdb_content = file_get_contents($url);

//Get movie parts
$regex = "/<tr\sclass=\"[even|odd]+\sdetailed\">(.*?)<\/tr>/is";
preg_match_all($regex, $imdb_content, $content_match);

$title_array = array();
$image_array = array();
$year_array = array();
$dir_array = array();
$rating_array = array();
$link_array = array();

$n = count($content_match[1]) > 5 ? 5 : count($content_match[1]);
//$i = 2;
for($i = 0; $i < $n; $i++)
{
	//Get the title
	$regex_title = "/<a\shref=\"\/title\/[\S]*\/\">(.*)<\/a>[\s]*<span/is";
	preg_match_all($regex_title, $content_match[1][$i], $title_match);
	$title_array[$i] = $title_match[1][0];

	//Get the image
	$regex_image = "/<img\ssrc=\"(.*?)\"\sheight/is";
	preg_match_all($regex_image, $content_match[1][$i], $image_match);
	$image_array[$i] = $image_match[1][0];

	//Get the year
	$regex_year = "/<span\sclass=\"year_type\">\((.*?)\)<\/span>/is";
	preg_match_all($regex_year, $content_match[1][$i], $year_match);
	$year_array[$i] = $year_match[1][0];
	$regex_pure_number = "/[\d]*/is";
	preg_match_all($regex_pure_number, $year_array[$i], $number_match);
	//var_dump($number_match);
	$year_array[$i] = $number_match[0][0] == '' ? "N/A" : $number_match[0][0];

	//Get the director
	$regex_dir = "/Dir:\s<a\shref=\"[\S]*\">(.*?)<\/a>(,?)(\s*?)(<a\shref=\"[\S]*\">(.*?)<\/a>(,?)(\s*?))*?With:/is";
	preg_match_all($regex_dir, $content_match[1][$i], $dir_match);
	//var_dump($dir_match);
	if($dir_match[1] != null)
	{
		$dir_array[$i] = $dir_match[1][0];
		if($dir_match[5][0] != null && $dir_match[5][0] != '')
			$dir_array[$i] .= ", " . $dir_match[5][0];
	}
	else
		$dir_array[$i] = "N/A";

	//Get the rating
	$regex_rating = "/<span\sclass=\"rating-rating\"><span\sclass=\"value\">(.*?)<\/span>/is";
	preg_match_all($regex_rating, $content_match[1][$i], $rating_match);
	if($rating_match[1] != null)
		if($rating_match[1][0] != '-')
			$rating_array[$i] = $rating_match[1][0];
		else
			$rating_array[$i] = "N/A";
	else
		$rating_array[$i] = "N/A";

	//Get the link
	$regex_link = "/<\/span>[\s]*<a\shref=\"(.*?)\">/is";
	preg_match_all($regex_link, $content_match[1][$i], $link_match);
	$link_array[$i] = $link_match[1][0];
}
	
$xml_content = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><rsp stat=\"ok\"><results total=\"" . $n . "\">";
	
for($i = 0; $i < $n; $i++)
{
	$xml_content .= "<result cover=\"";
	$xml_content .= $image_array[$i];
	$xml_content .= "\" title=\"";
	$xml_content .= $title_array[$i];
	$xml_content .= "\" year=\"";
	$xml_content .= $year_array[$i];
	$xml_content .= "\" director=\"";
	$xml_content .= $dir_array[$i];
	$xml_content .= "\" rating=\"";
	$xml_content .= $rating_array[$i];
	$xml_content .= "\" details=\"http://www.imdb.com";
	$xml_content .= $link_array[$i];
	$xml_content .= "\"/>";
}
	
$xml_content .= "</results></rsp>";
echo $xml_content;
?>