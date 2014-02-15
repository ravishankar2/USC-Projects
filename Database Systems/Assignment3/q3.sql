USE `videosharing`;

SELECT name
FROM users
WHERE username IN
(SELECT DISTINCT advertiser
FROM advertisements
WHERE views_wanted<
(SELECT COUNT(ad_shown)
FROM views
WHERE ad_shown=ad_id
GROUP BY ad_shown));