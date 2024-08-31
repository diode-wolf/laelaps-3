idf_component_register(SRCS "main.c"
                    "gps.c"
                    "init.c"
                    "servo.c"
                    "control.c"
                    # REQUIRES "main.c"
                    INCLUDE_DIRS ".")
