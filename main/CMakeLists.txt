idf_component_register(SRCS "tcp_client.c" "wifi_sta.c" "main.c"
                    "gps.c"
                    "init.c"
                    "servo.c"
                    "control.c"
                    "wifi_sta.c"
                    # REQUIRES "main.c"
                    INCLUDE_DIRS ".")
