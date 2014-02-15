USE `videosharing`;

SELECT ad_target_clicked.ad_id,name,email,price/views_wanted*1000 AS CPM,
targeted_clicks_sum/targeted_views_sum AS targeted_click_rate,none_targeted_clicks_sum/none_targeted_views_sum AS none_targeted_click_rate
FROM advertisements,users,
(SELECT ad_id,SUM(ad_clicked) AS targeted_clicks_sum
FROM
(SELECT video,ad_id,ad_clicked,videos.category AS video_category,ad_target_categories.category AS ad_category
FROM views,videos,ad_target_categories
WHERE video=video_id AND ad_shown=ad_id)
AS video_ad_category
WHERE video_category=ad_category
GROUP BY ad_id)
AS ad_target_clicked,
(SELECT ad_id,COUNT(ad_id) AS targeted_views_sum
FROM
(SELECT video,ad_id,ad_clicked,videos.category AS video_category,ad_target_categories.category AS ad_category
FROM views,videos,ad_target_categories
WHERE video=video_id AND ad_shown=ad_id)
AS video_ad_category
WHERE video_category=ad_category
GROUP BY ad_id)
AS ad_target_view,
(SELECT ad_shown,SUM(ad_clicked) AS none_targeted_clicks_sum
FROM videos,views
WHERE video_id=video AND
category NOT IN
(SELECT category
FROM ad_target_categories
WHERE ad_id=ad_shown)
GROUP BY ad_shown)
AS ad_none_target_clicked,
(SELECT ad_shown,COUNT(ad_shown) AS none_targeted_views_sum
FROM videos,views
WHERE video_id=video AND
category NOT IN
(SELECT category
FROM ad_target_categories
WHERE ad_id=ad_shown)
GROUP BY ad_shown)
AS ad_none_target_view
WHERE ad_target_clicked.ad_id=ad_target_view.ad_id AND ad_none_target_clicked.ad_shown=ad_none_target_view.ad_shown
AND ad_target_clicked.ad_id=ad_none_target_clicked.ad_shown AND advertisements.ad_id=ad_target_clicked.ad_id
AND username=advertiser
AND targeted_clicks_sum/targeted_views_sum<none_targeted_clicks_sum/none_targeted_views_sum
ORDER BY CPM DESC;