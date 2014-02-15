USE `videosharing`;

SELECT ad_id,name,email,(price/views_wanted*1000) AS CPM
FROM users,advertisements
WHERE username=advertiser AND (price/views_wanted*1000)<0.07;