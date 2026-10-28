#include "file_io.h"

// Module Variables -----------------------------------------------------------


// ----------------------------------------------------------------------------
err_t read_json(cJSON **json, const char *filename) {

    // Read the JSON file
    char *json_string = read_file(filename);  // MEM JSON file contents
    if (!json_string) {
        return ERR("Error reading file: %s", filename);
    }

    // Parse the JSON string
    *json = cJSON_Parse(json_string);
    if (!json) {
        return ERR("Failed to parse JSON");
    }

    // Free the JSON string
    free(json_string);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
err_t write_json(const cJSON *json, const char *filepath) {

    char *json_string = cJSON_Print(json);  // MEM Printed JSON string

    if (json_string == NULL) {
        return ERR("Failed generate JSON string for %s", filepath);
    }

    FILE *fp = fopen(filepath, "w");
    if (fp == NULL) {
        free(json_string);
        return ERR("Failed to open file %s", filepath);
    }

    if (fputs(json_string, fp) == EOF) {
        free(json_string);
        fclose(fp);
        return ERR("Failed to write to file %s", filepath);
    }

    fclose(fp);
    free(json_string);

    return NO_ERROR;
}

// ----------------------------------------------------------------------------
char *read_file(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        return NULL;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }

    long size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }

    rewind(fp);

    char *buffer = malloc((size_t)size + 1); // MEM File contents
    if (buffer == NULL) {
        fclose(fp);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)size, fp);
    fclose(fp);

    if (bytes_read != (size_t)size) {
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';
    return buffer;
}
