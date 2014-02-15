USE `videosharing`;

SELECT name,title,uploaded_at
FROM videos,users
WHERE uploaded_by=username AND video_id NOT IN
(SELECT DISTINCT video 
FROM videos,views
WHERE video_id=video AND viewed_at>uploaded_at + INTERVAL 14 DAY);