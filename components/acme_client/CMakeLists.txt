idf_component_register(
    SRCS "acme_client.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_client json mbedtls
)