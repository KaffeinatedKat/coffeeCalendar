#include "ical.h"

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

int ical_download(const char *url, const char *filename) {
    CURL *curl_handle;
    FILE *ical_file;

    // Curl shit
    // TODO: error handling
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    // Open the file to write
    ical_file = fopen(filename, "wb");
    if (ical_file == NULL) {
        // TODO: better errors
        perror("Error");
        return -1;
    }

    // Write the ical file
    // TODO: error handling
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, ical_file);
    curl_easy_perform(curl_handle);

    // Close the ical file
    fclose(ical_file);

    return 0;
}
