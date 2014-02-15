USE `videosharing`;

SELECT name,MAX(viewed_sum),title
FROM users 
LEFT JOIN 
(SELECT username,title,COUNT(video) AS viewed_sum
FROM users,videos,views
WHERE username=uploaded_by AND video_id=video
GROUP BY video) AS user_video_view
ON users.username=user_video_view.username
GROUP BY users.username
ORDER BY date_registered DESC;