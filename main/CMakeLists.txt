idf_component_register(SRCS "main.c"
                    "gps.c"
                    "init.c"
                    # REQUIRES "main.c"
                    INCLUDE_DIRS ".")
