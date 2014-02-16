<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN"
"http://www.w3.org/TR/REC-html40/strict.dtd">
<html>
<body>
	<div align="center">
		<?php
		header('Content-Type:text/html; charset=utf-8');
		//$title = "batman";
		//$title_type = "game";
		$title = $_POST['title'];
		$title_type = $_POST['media_type'];
		//$title = utf8_encode($title);
		if($title_type == "all_types")
			$url = "http://www.imdb.com/search/title?title=" . $title . "&title_type=feature,tv_series,game";
		else
			$url = "http://www.imdb.com/search/title?title=" . $title . "&title_type=" . $title_type;
		$url = str_replace(' ', '%20', $url);
		$imdb_content = file_get_contents($url);
		
		echo "<p style='font-size:21px;'><b>Search Result</b></p>";
		echo "<p><b>\"" . $title . "\" of type \"" . $title_type . "\":</b></p>";
		
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
		$i = 2;
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
 		
 		if(count($content_match[1]) >= 1)
 			$html_content = "<table border='1'><tr align='center' valign='center'><td>Image</td><td>Title</td><td>Year</td><td>Director</td><td>Rating(10)</td><td>Link to Movie</td></tr>";
 		else
 			$html_content = "<p style='font-size:23px;'><b>No movies found!</b></p>";
 		
 		for($i = 0; $i < $n; $i++)
 		{
 			$html_content .= "<tr align='center' valign='center'><td><img src='";
 			$html_content .= $image_array[$i];
 			$html_content .= "' height='74' width='54'></td><td>";
 			$html_content .= $title_array[$i];
 			$html_content .= "</td><td>";
 			$html_content .= $year_array[$i];
 			$html_content .= "</td><td>";
 			$html_content .= $dir_array[$i];
 			$html_content .= "</td><td>";
 			$html_content .= $rating_array[$i];
 			$html_content .= "</td><td><a href='http://www.imdb.com";
 			$html_content .= $link_array[$i];
 			$html_content .= "'>details</a></td></tr>";
 		}
 		
 		$html_content .= "</table>";
 		echo $html_content;
		?>
	</div>
	<div align="center">
		<a href="movie_search.html">Return</a>
	</div>
</body>
</html>