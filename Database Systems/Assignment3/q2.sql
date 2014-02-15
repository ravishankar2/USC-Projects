USE `videosharing`;

SELECT category,sum(price)
FROM advertisements,ad_target_categories
WHERE advertisements.ad_id=ad_target_categories.ad_id
GROUP BY category;