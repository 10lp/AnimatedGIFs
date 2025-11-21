%.new:	%.gif
	convert $? -coalesce -resize '192x192!' -layers Optimize $@

%.new2:	%.gif
	gifsicle -U --colors 256 --resize 192x192 $? > $@
