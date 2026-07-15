# Content sources

S_CONTENT :=			\
	content.c		\
	content_factory.c	\
	fetch.c			\
	hlcache.c		\
	llcache.c		\
	mimesniff.c		\
	textsearch.c		\
	urldb.c			\
	fs_backing_store.c


# Content fetcher sources
include content/fetchers/Makefile

S_FETCHERS := $(addprefix content/,$(S_FETCHERS))

# Content handler sources
include content/handlers/Makefile

S_CONTENT := $(addprefix content/,$(S_CONTENT))
