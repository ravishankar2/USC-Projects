if (-f my402list.h) then
	mv my402list.h my402list.h.submitted
endif
if (-f cs402.h) then
	mv cs402.h cs402.h.submitted
endif
cp ~csci570b/public/cs402/warmup1/cs402.h .
cp ~csci570b/public/cs402/warmup1/my402list.h .



