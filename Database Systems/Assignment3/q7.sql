USE `videosharing`;

SELECT views.video,title,AVG(rating)
FROM views,
(SELECT DISTINCT video,title
FROM videos,views
WHERE video=video_id AND rating=5 AND user=uploaded_by) AS rating_5_by_uploader
WHERE views.video=rating_5_by_uploader.video
GROUP BY video
ORDER BY AVG(rating) ASC
LIMIT 0,5;