set(req adc)
idf_component_register(
	SRCS "ad8232.c"
	INCLUDE_DIRS "."
	REQUIRES ${req}
)
