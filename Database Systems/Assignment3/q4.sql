USE `videosharing`;

SELECT name,SUM(profit)*revenue_share
FROM videos,users,partners,
(SELECT video,viewed_at,price/views_wanted AS profit
FROM views,advertisements
WHERE ad_shown=ad_id) AS video_profit
WHERE video_id=video AND uploaded_by=users.username AND partners.username=uploaded_by
AND	viewed_at BETWEEN '2012-06-01 00:00:00' AND '2012-07-01 00:00:00'
GROUP BY partners.username
ORDER BY SUM(profit)*revenue_share DESC
LIMIT 0,3;